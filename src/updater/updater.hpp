#pragma once

#include <string>

namespace updater
{
    typedef void (*ProgressCallback)(const char* message, void* userData);

    enum class Result
    {
        NoUpdate,
        Updated,
        Failed,
    };

    // All application-specific values are supplied by the embedding program so
    // this library can be shared by Bombermaaan, Cross//Roads, and other apps.
    struct Config
    {
        std::string applicationName;
        std::string configFileBaseName;
        std::string currentVersion;
        int currentBuild = 0;
        std::string defaultManifestUrl;
        std::string executablePath;
        std::string targetPath;
        std::string platform;
        ProgressCallback progressCallback = nullptr;
        void* progressUserData = nullptr;
    };

    Result run(const Config& config);
}
