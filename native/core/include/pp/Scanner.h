// Plugins Policy by Piradex — plugin discovery core (no JUCE dependency).
#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace pp
{

enum class PluginFormat { VST3, AudioUnit, Unknown };

// Scan / Available / Unsupported / Failed — as specified in section 5.
enum class ScanState { Scan, Available, Unsupported, Failed };

struct PluginInfo
{
    std::filesystem::path path;
    std::string name;      // file stem, e.g. "ROOM-X"
    PluginFormat format = PluginFormat::Unknown;
    ScanState state = ScanState::Scan;
    std::string reason;    // human-readable explanation for Unsupported / Failed
};

// Maps a bundle or file path to its plugin format by extension.
PluginFormat classifyFormat(const std::filesystem::path& path);

// Returns a PluginInfo with its format and an initial state.
// Unknown formats are Unsupported; the reason explains why.
PluginInfo describePlugin(const std::filesystem::path& path);

// Standard VST3 and AU locations for the current platform (may not exist).
std::vector<std::filesystem::path> defaultPluginDirectories();

// Scans the given directories recursively for plugin bundles.
// Never throws: directories that cannot be read produce a Failed entry.
std::vector<PluginInfo> scanDirectories(const std::vector<std::filesystem::path>& dirs);

const char* toString(ScanState state);
const char* toString(PluginFormat format);

} // namespace pp
