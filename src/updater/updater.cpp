#include "updater.hpp"

#ifdef __SWITCH__
#include <switch.h>
#endif

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace updater
{
    namespace
    {
        const int NetworkTimeoutSeconds = 8;

        struct Url
        {
            std::string host;
            std::string port;
            std::string path;
        };

        struct HttpResponse
        {
            int statusCode = 0;
            std::vector<unsigned char> body;
        };

#ifdef _WIN32
        typedef SOCKET SocketHandle;
        const SocketHandle InvalidSocket = INVALID_SOCKET;
#else
        typedef int SocketHandle;
        const SocketHandle InvalidSocket = -1;
#endif

        struct Progress
        {
            Progress(ProgressCallback callback, void* userData)
                : callback_(callback), userData_(userData)
            {
            }

            void report(const std::string& message) const
            {
                std::fprintf(stderr, "[update] %s\n", message.c_str());
                if (callback_ != nullptr)
                {
                    callback_(message.c_str(), userData_);
                }
            }

        private:
            ProgressCallback callback_;
            void* userData_;
        };

        std::string trim(const std::string& value)
        {
            std::size_t first = 0;
            while (first < value.size() &&
                   std::isspace(static_cast<unsigned char>(value[first])) != 0)
            {
                ++first;
            }

            std::size_t last = value.size();
            while (last > first &&
                   std::isspace(static_cast<unsigned char>(value[last - 1])) != 0)
            {
                --last;
            }
            return value.substr(first, last - first);
        }

        std::string lowerCase(const std::string& value)
        {
            std::string result = value;
            for (char& character : result)
            {
                character = static_cast<char>(
                    std::tolower(static_cast<unsigned char>(character)));
            }
            return result;
        }

        bool readTextFile(const std::string& path, std::string& output)
        {
            std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
            if (!input)
            {
                return false;
            }

            std::ostringstream contents;
            contents << input.rdbuf();
            output = contents.str();
            return true;
        }

        bool fileExists(const std::string& path)
        {
            std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
            return static_cast<bool>(input);
        }

        bool extractValue(const std::string& document,
                          const std::string& key,
                          std::string& value)
        {
            const std::string jsonKey = "\"" + key + "\"";
            std::size_t keyPosition = document.find(jsonKey);
            if (keyPosition == std::string::npos)
            {
                // Also accept the simple YAML form:
                // manifest_url: http://example.invalid/file.json
                keyPosition = document.find(key);
                if (keyPosition == std::string::npos)
                {
                    return false;
                }
            }

            const std::size_t colon = document.find(':', keyPosition + key.size());
            if (colon == std::string::npos)
            {
                return false;
            }

            std::size_t position = colon + 1;
            while (position < document.size() &&
                   std::isspace(static_cast<unsigned char>(document[position])) != 0)
            {
                ++position;
            }

            if (position >= document.size())
            {
                return false;
            }

            if (document[position] == '"')
            {
                ++position;
                std::string parsed;
                while (position < document.size())
                {
                    const char character = document[position++];
                    if (character == '"')
                    {
                        value = parsed;
                        return true;
                    }

                    if (character == '\\' && position < document.size())
                    {
                        const char escaped = document[position++];
                        if (escaped == '"' || escaped == '\\' || escaped == '/')
                        {
                            parsed.push_back(escaped);
                        }
                        else if (escaped == 'n')
                        {
                            parsed.push_back('\n');
                        }
                        else if (escaped == 'r')
                        {
                            parsed.push_back('\r');
                        }
                        else if (escaped == 't')
                        {
                            parsed.push_back('\t');
                        }
                        else
                        {
                            parsed.push_back(escaped);
                        }
                    }
                    else
                    {
                        parsed.push_back(character);
                    }
                }
                return false;
            }

            const std::size_t end = document.find_first_of(",}\r\n#", position);
            value = trim(document.substr(position, end == std::string::npos
                ? std::string::npos
                : end - position));
            return !value.empty();
        }

#ifndef __SWITCH__
        std::string directoryOf(const std::string& path)
        {
            const std::size_t slash = path.find_last_of("/\\");
            if (slash == std::string::npos)
            {
                return ".";
            }
            if (slash == 0)
            {
                return "/";
            }
            return path.substr(0, slash);
        }
#endif

        std::string configPath(const Config& config)
        {
            const std::string baseName = config.configFileBaseName.empty()
                ? config.applicationName
                : config.configFileBaseName;
#ifdef __SWITCH__
            return std::string("sdmc:/switch/") + baseName + "-update.json";
#else
            return directoryOf(config.executablePath) + "/" + baseName + "-update.json";
#endif
        }

        std::string alternateConfigPath(const Config& config,
                                        const char* extension)
        {
            const std::string baseName = config.configFileBaseName.empty()
                ? config.applicationName
                : config.configFileBaseName;
#ifdef __SWITCH__
            return std::string("sdmc:/switch/") + baseName + "-update." + extension;
#else
            return directoryOf(config.executablePath) +
                "/" + baseName + "-update." + extension;
#endif
        }

        std::string configuredManifestUrl(const Config& config)
        {
            const std::vector<std::string> paths =
            {
                configPath(config),
                alternateConfigPath(config, "yaml"),
                alternateConfigPath(config, "yml"),
            };

            for (const std::string& path : paths)
            {
                std::string document;
                if (!readTextFile(path, document))
                {
                    continue;
                }

                std::string url;
                if (extractValue(document, "manifest_url", url) ||
                    extractValue(document, "url", url))
                {
                    url = trim(url);
                    if (url.compare(0, 7, "http://") == 0 ||
                        url.compare(0, 8, "https://") == 0)
                    {
                        return url;
                    }
                }
            }

            return config.defaultManifestUrl;
        }

        bool parseHttpUrl(const std::string& value, Url& url)
        {
            if (value.compare(0, 7, "http://") != 0)
            {
                std::fprintf(
                    stderr,
                    "[update] unsupported URL (only http:// is currently supported): %s\n",
                    value.c_str());
                return false;
            }

            const std::size_t authorityStart = 7;
            const std::size_t pathStart = value.find('/', authorityStart);
            const std::string authority = value.substr(
                authorityStart,
                pathStart == std::string::npos
                    ? std::string::npos
                    : pathStart - authorityStart);
            if (authority.empty())
            {
                return false;
            }

            const std::size_t portSeparator = authority.rfind(':');
            if (portSeparator != std::string::npos &&
                authority.find(']') == std::string::npos)
            {
                url.host = authority.substr(0, portSeparator);
                url.port = authority.substr(portSeparator + 1);
            }
            else
            {
                url.host = authority;
                url.port = "80";
            }

            url.path = pathStart == std::string::npos
                ? "/"
                : value.substr(pathStart);
            return !url.host.empty() && !url.port.empty();
        }

        void closeSocket(SocketHandle socket)
        {
            if (socket == InvalidSocket)
            {
                return;
            }
#ifdef _WIN32
            closesocket(socket);
#else
            close(socket);
#endif
        }

        void setSocketTimeout(SocketHandle socket)
        {
#ifdef _WIN32
            const DWORD timeout = static_cast<DWORD>(NetworkTimeoutSeconds * 1000);
            setsockopt(
                socket,
                SOL_SOCKET,
                SO_RCVTIMEO,
                reinterpret_cast<const char*>(&timeout),
                sizeof(timeout));
            setsockopt(
                socket,
                SOL_SOCKET,
                SO_SNDTIMEO,
                reinterpret_cast<const char*>(&timeout),
                sizeof(timeout));
#else
            timeval timeout{};
            timeout.tv_sec = NetworkTimeoutSeconds;
            setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#endif
        }

        class NetworkSession
        {
        public:
            NetworkSession()
                : active_(false)
            {
            }

            bool start()
            {
#ifdef _WIN32
                WSADATA data{};
                if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
                {
                    return false;
                }
#elif defined(__SWITCH__)
                if (R_FAILED(socketInitializeDefault()))
                {
                    return false;
                }
#endif
                active_ = true;
                return true;
            }

            ~NetworkSession()
            {
                if (!active_)
                {
                    return;
                }
#ifdef _WIN32
                WSACleanup();
#elif defined(__SWITCH__)
                socketExit();
#endif
            }

        private:
            bool active_;
        };

        bool sendAll(SocketHandle socket, const std::string& request)
        {
            std::size_t sent = 0;
            while (sent < request.size())
            {
                const std::size_t remaining = request.size() - sent;
                const int chunk = static_cast<int>(
                    std::min<std::size_t>(remaining, 16 * 1024));
                const int result = ::send(
                    socket,
                    request.data() + sent,
                    chunk,
                    0);
                if (result <= 0)
                {
                    return false;
                }
                sent += static_cast<std::size_t>(result);
            }
            return true;
        }

        bool findHeaderEnd(const std::vector<unsigned char>& response,
                           std::size_t& headerEnd)
        {
            for (std::size_t index = 3; index < response.size(); ++index)
            {
                if (response[index - 3] == '\r' &&
                    response[index - 2] == '\n' &&
                    response[index - 1] == '\r' &&
                    response[index] == '\n')
                {
                    headerEnd = index + 1;
                    return true;
                }
            }
            return false;
        }

        bool decodeChunkedBody(const std::vector<unsigned char>& encoded,
                               std::vector<unsigned char>& decoded)
        {
            std::size_t position = 0;
            while (position < encoded.size())
            {
                const std::vector<unsigned char>::const_iterator lineBegin =
                    encoded.begin() + position;
                const std::vector<unsigned char>::const_iterator lineEndIterator =
                    std::search(
                        lineBegin,
                        encoded.end(),
                        "\r\n",
                        "\r\n" + 2);
                if (lineEndIterator == encoded.end())
                {
                    return false;
                }

                const std::size_t lineEnd =
                    static_cast<std::size_t>(lineEndIterator - encoded.begin());
                const std::string sizeText(
                    encoded.begin() + position,
                    encoded.begin() + lineEnd);
                const unsigned long chunkSize = std::strtoul(
                    sizeText.c_str(),
                    nullptr,
                    16);
                position = lineEnd + 2;
                if (chunkSize == 0)
                {
                    return true;
                }

                if (chunkSize > encoded.size() - position ||
                    encoded.size() - position - chunkSize < 2)
                {
                    return false;
                }

                decoded.insert(
                    decoded.end(),
                    encoded.begin() + position,
                    encoded.begin() + position + chunkSize);
                position += chunkSize;
                if (encoded[position] != '\r' || encoded[position + 1] != '\n')
                {
                    return false;
                }
                position += 2;
            }
            return false;
        }

        bool httpGet(const std::string& urlString,
                     const std::string& userAgent,
                     HttpResponse& response,
                     const Progress& progress)
        {
            Url url;
            if (!parseHttpUrl(urlString, url))
            {
                return false;
            }

            progress.report("Initializing network connection...");
            NetworkSession network;
            if (!network.start())
            {
                std::fprintf(stderr, "[update] network initialization failed\n");
                return false;
            }

            progress.report("Resolving update server " + url.host + "...");
            addrinfo hints{};
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_family = AF_UNSPEC;
            addrinfo* addresses = nullptr;
            if (getaddrinfo(url.host.c_str(), url.port.c_str(), &hints, &addresses) != 0)
            {
                std::fprintf(stderr, "[update] could not resolve %s\n", url.host.c_str());
                return false;
            }

            SocketHandle connected = InvalidSocket;
            for (addrinfo* address = addresses;
                 address != nullptr;
                 address = address->ai_next)
            {
                SocketHandle candidate = static_cast<SocketHandle>(
                    socket(address->ai_family, address->ai_socktype, address->ai_protocol));
                if (candidate == InvalidSocket)
                {
                    continue;
                }

                setSocketTimeout(candidate);
                progress.report("Connecting to update server " + url.host + ":" + url.port + "...");
                if (::connect(candidate, address->ai_addr, address->ai_addrlen) == 0)
                {
                    connected = candidate;
                    break;
                }
                closeSocket(candidate);
            }
            freeaddrinfo(addresses);

            if (connected == InvalidSocket)
            {
                std::fprintf(stderr, "[update] could not connect to %s:%s\n",
                             url.host.c_str(), url.port.c_str());
                return false;
            }

            const std::string request =
                "GET " + url.path + " HTTP/1.1\r\n"
                "Host: " + url.host + "\r\n"
                "Connection: close\r\n"
                // The manifest is mutable.  Do not allow a proxy or a
                // server-side cache to make a freshly published build
                // number invisible to the updater.
                "Cache-Control: no-cache, no-store, max-age=0\r\n"
                "Pragma: no-cache\r\n"
                "User-Agent: " + userAgent + "\r\n"
                "\r\n";

            progress.report("Requesting " + url.path + "...");
            progress.report("Receiving server response...");
            bool success = sendAll(connected, request);
            std::vector<unsigned char> rawResponse;
            unsigned char buffer[16 * 1024];
            while (success)
            {
                const int count = static_cast<int>(
                    recv(connected, reinterpret_cast<char*>(buffer), sizeof(buffer), 0));
                if (count == 0)
                {
                    break;
                }
                if (count < 0)
                {
                    success = false;
                    break;
                }
                rawResponse.insert(rawResponse.end(), buffer, buffer + count);
            }
            closeSocket(connected);

            if (!success)
            {
                std::fprintf(stderr, "[update] HTTP read failed\n");
                return false;
            }

            std::size_t headerEnd = 0;
            if (!findHeaderEnd(rawResponse, headerEnd))
            {
                std::fprintf(stderr, "[update] invalid HTTP response\n");
                return false;
            }

            const std::string headers(
                rawResponse.begin(),
                rawResponse.begin() + headerEnd);
            const std::size_t firstSpace = headers.find(' ');
            if (firstSpace == std::string::npos)
            {
                return false;
            }
            response.statusCode = std::atoi(headers.c_str() + firstSpace + 1);
            if (response.statusCode < 200 || response.statusCode >= 300)
            {
                std::fprintf(stderr, "[update] server returned HTTP %d\n",
                             response.statusCode);
                return false;
            }

            std::vector<unsigned char> body(
                rawResponse.begin() + headerEnd,
                rawResponse.end());
            const std::string lowerHeaders = lowerCase(headers);
            if (lowerHeaders.find("transfer-encoding: chunked") != std::string::npos)
            {
                return decodeChunkedBody(body, response.body);
            }

            response.body.swap(body);
            return true;
        }

        class Sha256
        {
        public:
            Sha256()
                : dataLength_(0), bitLength_(0)
            {
                state_[0] = 0x6a09e667;
                state_[1] = 0xbb67ae85;
                state_[2] = 0x3c6ef372;
                state_[3] = 0xa54ff53a;
                state_[4] = 0x510e527f;
                state_[5] = 0x9b05688c;
                state_[6] = 0x1f83d9ab;
                state_[7] = 0x5be0cd19;
            }

            void update(const unsigned char* input, std::size_t length)
            {
                for (std::size_t index = 0; index < length; ++index)
                {
                    data_[dataLength_++] = input[index];
                    if (dataLength_ == 64)
                    {
                        transform();
                        bitLength_ += 512;
                        dataLength_ = 0;
                    }
                }
            }

            std::string final()
            {
                std::size_t index = dataLength_;
                data_[index++] = 0x80;
                while (index < 56)
                {
                    data_[index++] = 0;
                }

                if (dataLength_ >= 56)
                {
                    while (index < 64)
                    {
                        data_[index++] = 0;
                    }
                    transform();
                    std::fill(data_, data_ + 56, 0);
                }

                bitLength_ += static_cast<unsigned long long>(dataLength_) * 8;
                for (int offset = 0; offset < 8; ++offset)
                {
                    data_[63 - offset] = static_cast<unsigned char>(
                        bitLength_ >> (offset * 8));
                }
                transform();

                std::ostringstream output;
                output << std::hex << std::setfill('0');
                for (std::size_t stateIndex = 0; stateIndex < 8; ++stateIndex)
                {
                    output << std::setw(8) << state_[stateIndex];
                }
                return output.str();
            }

        private:
            static unsigned int rotateRight(unsigned int value, unsigned int count)
            {
                return (value >> count) | (value << (32 - count));
            }

            void transform()
            {
                static const unsigned int constants[64] =
                {
                    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
                    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
                    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
                    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
                    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
                    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
                    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
                    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
                    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
                    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
                    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
                    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
                    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
                    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
                    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
                    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
                };

                unsigned int schedule[64];
                for (int index = 0; index < 16; ++index)
                {
                    schedule[index] =
                        (static_cast<unsigned int>(data_[index * 4]) << 24) |
                        (static_cast<unsigned int>(data_[index * 4 + 1]) << 16) |
                        (static_cast<unsigned int>(data_[index * 4 + 2]) << 8) |
                        static_cast<unsigned int>(data_[index * 4 + 3]);
                }

                for (int index = 16; index < 64; ++index)
                {
                    const unsigned int smallSigma0 =
                        rotateRight(schedule[index - 15], 7) ^
                        rotateRight(schedule[index - 15], 18) ^
                        (schedule[index - 15] >> 3);
                    const unsigned int smallSigma1 =
                        rotateRight(schedule[index - 2], 17) ^
                        rotateRight(schedule[index - 2], 19) ^
                        (schedule[index - 2] >> 10);
                    schedule[index] = schedule[index - 16] + smallSigma0 +
                        schedule[index - 7] + smallSigma1;
                }

                unsigned int a = state_[0];
                unsigned int b = state_[1];
                unsigned int c = state_[2];
                unsigned int d = state_[3];
                unsigned int e = state_[4];
                unsigned int f = state_[5];
                unsigned int g = state_[6];
                unsigned int h = state_[7];

                for (int index = 0; index < 64; ++index)
                {
                    const unsigned int bigSigma1 =
                        rotateRight(e, 6) ^ rotateRight(e, 11) ^ rotateRight(e, 25);
                    const unsigned int choice = (e & f) ^ ((~e) & g);
                    const unsigned int temporary1 =
                        h + bigSigma1 + choice + constants[index] + schedule[index];
                    const unsigned int bigSigma0 =
                        rotateRight(a, 2) ^ rotateRight(a, 13) ^ rotateRight(a, 22);
                    const unsigned int majority = (a & b) ^ (a & c) ^ (b & c);
                    const unsigned int temporary2 = bigSigma0 + majority;

                    h = g;
                    g = f;
                    f = e;
                    e = d + temporary1;
                    d = c;
                    c = b;
                    b = a;
                    a = temporary1 + temporary2;
                }

                state_[0] += a;
                state_[1] += b;
                state_[2] += c;
                state_[3] += d;
                state_[4] += e;
                state_[5] += f;
                state_[6] += g;
                state_[7] += h;
            }

            unsigned char data_[64]{};
            std::size_t dataLength_;
            unsigned long long bitLength_;
            unsigned int state_[8]{};
        };

        std::string sha256(const std::vector<unsigned char>& data)
        {
            Sha256 hash;
            if (!data.empty())
            {
                hash.update(data.data(), data.size());
            }
            return hash.final();
        }

        std::vector<int> versionParts(const std::string& version)
        {
            std::vector<int> result;
            int number = 0;
            bool hasNumber = false;
            for (char character : version)
            {
                if (character >= '0' && character <= '9')
                {
                    number = number * 10 + (character - '0');
                    hasNumber = true;
                }
                else if (hasNumber)
                {
                    result.push_back(number);
                    number = 0;
                    hasNumber = false;
                }
            }
            if (hasNumber)
            {
                result.push_back(number);
            }
            while (result.size() < 3)
            {
                result.push_back(0);
            }
            return result;
        }

        int compareVersions(const std::string& left,
                            const std::string& right)
        {
            const std::vector<int> leftParts = versionParts(left);
            const std::vector<int> rightParts = versionParts(right);
            for (std::size_t index = 0; index < 3; ++index)
            {
                if (leftParts[index] != rightParts[index])
                {
                    return leftParts[index] > rightParts[index] ? 1 : -1;
                }
            }
            return 0;
        }

        bool parseBuildNumber(const std::string& text, int& buildNumber)
        {
            const std::string value = trim(text);
            if (value.empty())
            {
                return false;
            }

            char* end = nullptr;
            const long parsed = std::strtol(value.c_str(), &end, 10);
            if (end == value.c_str() ||
                *end != '\0' ||
                parsed < 0 ||
                parsed > static_cast<long>(std::numeric_limits<int>::max()))
            {
                return false;
            }

            buildNumber = static_cast<int>(parsed);
            return true;
        }

        std::string defaultTargetPath(const Config& config)
        {
#ifdef __SWITCH__
            return std::string("sdmc:/switch/") + config.applicationName + ".nro";
#else
            return config.executablePath.empty()
                ? config.applicationName
                : config.executablePath;
#endif
        }

        bool writeBinaryFile(const std::string& path,
                             const std::vector<unsigned char>& data)
        {
            std::ofstream output(
                path.c_str(),
                std::ios::out | std::ios::binary | std::ios::trunc);
            if (!output)
            {
                return false;
            }
            if (!data.empty())
            {
                output.write(
                    reinterpret_cast<const char*>(data.data()),
                    static_cast<std::streamsize>(data.size()));
            }
            return static_cast<bool>(output);
        }

        bool replaceExistingFile(const std::string& sourcePath,
                                 const std::string& targetPath)
        {
            if (std::rename(sourcePath.c_str(), targetPath.c_str()) == 0)
            {
                return true;
            }

            // Some filesystem implementations do not overwrite an existing
            // destination with rename(). Move the old file aside, install the
            // new one, and restore the old file if the second rename fails.
            if (!fileExists(targetPath))
            {
                return false;
            }

            const std::string backupPath = targetPath + ".backup";
            std::remove(backupPath.c_str());
            if (std::rename(targetPath.c_str(), backupPath.c_str()) != 0)
            {
                return false;
            }

            if (std::rename(sourcePath.c_str(), targetPath.c_str()) == 0)
            {
                std::remove(backupPath.c_str());
                return true;
            }

            // Best effort rollback so a failed update never leaves the
            // application without its executable.
            std::rename(backupPath.c_str(), targetPath.c_str());
            return false;
        }

        bool replaceTarget(const std::string& temporaryPath,
                           const std::string& targetPath)
        {
            if (replaceExistingFile(temporaryPath, targetPath))
            {
                return true;
            }

            const std::string pendingPath = targetPath + ".pending";
            std::remove(pendingPath.c_str());
            if (std::rename(temporaryPath.c_str(), pendingPath.c_str()) == 0)
            {
                std::fprintf(
                    stderr,
                    "[update] update downloaded and staged at %s\n",
                    pendingPath.c_str());
            }
            else
            {
                std::fprintf(
                    stderr,
                    "[update] could not stage downloaded update at %s\n",
                    pendingPath.c_str());
            }
            return false;
        }

        bool installPendingUpdate(const std::string& targetPath)
        {
            const std::string pendingPath = targetPath + ".pending";
            if (!fileExists(pendingPath))
            {
                return false;
            }

            if (replaceExistingFile(pendingPath, targetPath))
            {
                std::fprintf(stderr, "[update] installed previously staged update\n");
                return true;
            }
            else
            {
                std::fprintf(
                    stderr,
                    "[update] could not install staged update; keeping %s\n",
                    pendingPath.c_str());
            }
            return false;
        }
    }

    Result run(const Config& config)
    {
        const Progress progress(config.progressCallback, config.progressUserData);

#ifdef __SWITCH__
        progress.report("Mounting SD card...");
        fsdevMountSdmc();
#endif

        const std::string targetPath = config.targetPath.empty()
            ? defaultTargetPath(config)
            : config.targetPath;
        progress.report("Checking for a previously staged update...");
        // A pending update was downloaded by a previous launch.  Install it
        // before doing another network check.  The running process may still
        // contain the old build's compile-time number, so checking the
        // manifest again here would immediately download the same artifact
        // for a second time.
        if (installPendingUpdate(targetPath))
        {
            progress.report("Installed a previously staged update.");
            return Result::Updated;
        }

        progress.report("Reading update configuration...");
        const std::string manifestUrl = configuredManifestUrl(config);
        std::fprintf(stderr, "[update] checking %s\n", manifestUrl.c_str());
        progress.report("Downloading update manifest...");

        HttpResponse manifestResponse;
        if (!httpGet(
                manifestUrl,
                config.applicationName + "/" + config.currentVersion,
                manifestResponse,
                progress))
        {
            std::fprintf(stderr, "[update] check failed; starting current version\n");
            progress.report("Update check failed; starting current version.");
            return Result::Failed;
        }

        progress.report("Parsing update manifest...");
        const std::string manifest(
            manifestResponse.body.begin(),
            manifestResponse.body.end());
        std::string remoteVersion;
        std::string remoteBuildText;
        std::string downloadUrl;
        std::string expectedHash;
        std::string platform;
        if (!extractValue(manifest, "version", remoteVersion) ||
            !extractValue(manifest, "download", downloadUrl))
        {
            std::fprintf(stderr, "[update] manifest is missing version/download\n");
            progress.report("Update manifest is incomplete.");
            return Result::Failed;
        }
        const bool hasRemoteBuild =
            extractValue(manifest, "build", remoteBuildText);
        extractValue(manifest, "sha256", expectedHash);
        extractValue(manifest, "platform", platform);

        int remoteBuild = 0;
        if (hasRemoteBuild &&
            !parseBuildNumber(remoteBuildText, remoteBuild))
        {
            std::fprintf(stderr, "[update] manifest has an invalid build number\n");
            progress.report("Update manifest has an invalid build number.");
            return Result::Failed;
        }

        const std::string currentPlatform = config.platform.empty()
#ifdef __SWITCH__
            ? "switch"
#else
            ? "x86"
#endif
            : config.platform;
        if (!platform.empty() && platform != currentPlatform)
        {
            std::fprintf(stderr, "[update] manifest is for platform %s, current platform is %s\n",
                         platform.c_str(), currentPlatform.c_str());
            progress.report("Update is for a different platform.");
            return Result::NoUpdate;
        }

        const int versionComparison = compareVersions(remoteVersion, config.currentVersion);
        const bool newerVersion = versionComparison > 0;
        // Build numbers are monotonically increasing across the Switch
        // builds.  They must therefore be considered independently of the
        // marketing version: a rebuild with the same (or even accidentally
        // stale) version still needs to update when its build is newer.
        const bool newerBuild =
            hasRemoteBuild && remoteBuild > config.currentBuild;
        if (!newerVersion && !newerBuild)
        {
            std::fprintf(
                stderr,
                "[update] current version %s build %d is up to date "
                "(remote version %s build %d)\n",
                config.currentVersion.c_str(),
                config.currentBuild,
                remoteVersion.c_str(),
                remoteBuild);
            progress.report("The current version is up to date.");
            return Result::NoUpdate;
        }

        {
            std::ostringstream message;
            message << "New version " << remoteVersion
                    << " build " << remoteBuild << " is available.";
            progress.report(message.str());
        }
        std::fprintf(
            stderr,
            "[update] downloading version %s build %d\n",
            remoteVersion.c_str(),
            remoteBuild);
        progress.report("Downloading new version...");
        HttpResponse artifactResponse;
        if (!httpGet(
                downloadUrl,
                config.applicationName + "/" + config.currentVersion,
                artifactResponse,
                progress))
        {
            std::fprintf(stderr, "[update] download failed; starting current version\n");
            progress.report("Download failed; starting current version.");
            return Result::Failed;
        }

        progress.report("Verifying downloaded file...");
        if (!expectedHash.empty() &&
            lowerCase(trim(expectedHash)) != sha256(artifactResponse.body))
        {
            std::fprintf(stderr, "[update] checksum mismatch; refusing update\n");
            progress.report("Checksum verification failed; refusing update.");
            return Result::Failed;
        }

        const std::string temporaryPath = targetPath + ".download";
        progress.report("Writing downloaded file...");
        if (!writeBinaryFile(temporaryPath, artifactResponse.body))
        {
            std::fprintf(stderr, "[update] could not write %s\n",
                         temporaryPath.c_str());
            progress.report("Could not write downloaded file.");
            return Result::Failed;
        }

        progress.report("Installing update...");
        if (!replaceTarget(temporaryPath, targetPath))
        {
            progress.report("Update was downloaded but could not be installed.");
            return Result::Failed;
        }

        std::fprintf(
            stderr,
            "[update] installed version %s build %d; restart to activate it\n",
            remoteVersion.c_str(),
            remoteBuild);
        progress.report("Update installed; restart to activate it.");
        return Result::Updated;
    }
}
