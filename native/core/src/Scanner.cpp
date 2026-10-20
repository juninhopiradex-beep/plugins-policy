#include "pp/Scanner.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <system_error>

namespace fs = std::filesystem;

namespace pp
{

namespace
{
std::string lowerExtension(const fs::path& path)
{
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext;
}
} // namespace

PluginFormat classifyFormat(const fs::path& path)
{
    const auto ext = lowerExtension(path);
    if (ext == ".vst3")      return PluginFormat::VST3;
    if (ext == ".component") return PluginFormat::AudioUnit;
    return PluginFormat::Unknown;
}

PluginInfo describePlugin(const fs::path& path)
{
    PluginInfo info;
    info.path = path;
    info.name = path.stem().string();
    info.format = classifyFormat(path);

    if (info.format == PluginFormat::Unknown)
    {
        info.state = ScanState::Unsupported;
        info.reason = "Not a VST3 or Audio Unit bundle";
    }
    else
    {
        // M1 only discovers packages. Loading and architecture checks arrive in M2.
        info.state = ScanState::Available;
    }
    return info;
}

std::vector<fs::path> defaultPluginDirectories()
{
#if defined(_WIN32)
    std::vector<fs::path> dirs;
    if (const char* common = std::getenv("COMMONPROGRAMFILES"))
        dirs.push_back(fs::path(common) / "VST3");
    dirs.push_back(fs::path("C:/Program Files/Common Files/VST3"));
    return dirs;
#elif defined(__APPLE__)
    std::vector<fs::path> dirs;
    if (const char* home = std::getenv("HOME"))
    {
        dirs.push_back(fs::path(home) / "Library/Audio/Plug-Ins/VST3");
        dirs.push_back(fs::path(home) / "Library/Audio/Plug-Ins/Components");
    }
    dirs.push_back("/Library/Audio/Plug-Ins/VST3");
    dirs.push_back("/Library/Audio/Plug-Ins/Components");
    return dirs;
#else
    return {};
#endif
}

std::vector<PluginInfo> scanDirectories(const std::vector<fs::path>& dirs)
{
    std::vector<PluginInfo> results;

    for (const auto& dir : dirs)
    {
        std::error_code ec;
        if (!fs::exists(dir, ec) || ec)
            continue; // a missing standard directory is normal, not a failure

        fs::recursive_directory_iterator it(dir, fs::directory_options::skip_permission_denied, ec);
        if (ec)
        {
            PluginInfo failed;
            failed.path = dir;
            failed.name = dir.filename().string();
            failed.state = ScanState::Failed;
            failed.reason = "Cannot read directory: " + ec.message();
            results.push_back(std::move(failed));
            continue;
        }

        const fs::recursive_directory_iterator end;
        for (; it != end; it.increment(ec))
        {
            if (ec)
            {
                ec.clear();
                continue; // skip unreadable entries and keep scanning
            }

            const auto& entry = it->path();
            const auto format = classifyFormat(entry);
            if (format == PluginFormat::Unknown)
                continue;

            results.push_back(describePlugin(entry));
            // Plugin bundles are directories; do not descend into them.
            it.disable_recursion_pending();
        }
    }

    std::sort(results.begin(), results.end(),
              [](const PluginInfo& a, const PluginInfo& b) { return a.path < b.path; });
    results.erase(std::unique(results.begin(), results.end(),
                              [](const PluginInfo& a, const PluginInfo& b) { return a.path == b.path; }),
                  results.end());
    return results;
}

const char* toString(ScanState state)
{
    switch (state)
    {
        case ScanState::Scan:        return "Scan";
        case ScanState::Available:   return "Available";
        case ScanState::Unsupported: return "Unsupported";
        case ScanState::Failed:      return "Failed";
    }
    return "Unknown";
}

const char* toString(PluginFormat format)
{
    switch (format)
    {
        case PluginFormat::VST3:      return "VST3";
        case PluginFormat::AudioUnit: return "AU";
        case PluginFormat::Unknown:   return "Unknown";
    }
    return "Unknown";
}

} // namespace pp
