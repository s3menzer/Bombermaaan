#pragma once

#include <string>

namespace updater
{
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
    };

    Result run(const Config& config);
}
