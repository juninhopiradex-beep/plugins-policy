// Minimal self-contained tests for the scanner core. Run via CTest.
#include "pp/Scanner.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>

namespace fs = std::filesystem;

static int failures = 0;

#define EXPECT_TRUE(cond)                                                    \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++failures;                                                      \
        }                                                                    \
    } while (0)

static void testClassifyFormat()
{
    EXPECT_TRUE(pp::classifyFormat("ROOM-X.vst3") == pp::PluginFormat::VST3);
    EXPECT_TRUE(pp::classifyFormat("ROOM-X.VST3") == pp::PluginFormat::VST3);
    EXPECT_TRUE(pp::classifyFormat("EQ.component") == pp::PluginFormat::AudioUnit);
    EXPECT_TRUE(pp::classifyFormat("notes.txt") == pp::PluginFormat::Unknown);
    EXPECT_TRUE(pp::classifyFormat("noextension") == pp::PluginFormat::Unknown);
}

static void testDescribeUnknownIsUnsupported()
{
    const auto info = pp::describePlugin("readme.txt");
    EXPECT_TRUE(info.state == pp::ScanState::Unsupported);
    EXPECT_TRUE(!info.reason.empty());
}

static void testDescribeVst3IsAvailable()
{
    const auto info = pp::describePlugin("MIXSPACE.vst3");
    EXPECT_TRUE(info.state == pp::ScanState::Available);
    EXPECT_TRUE(info.name == "MIXSPACE");
    EXPECT_TRUE(info.format == pp::PluginFormat::VST3);
}

static void testScanMissingDirectoryIsEmpty()
{
    const auto results = pp::scanDirectories({ "/this/path/should/not/exist/pp-test" });
    EXPECT_TRUE(results.empty());
}

static void testScanFindsBundlesAndSkipsOthers()
{
    const fs::path root = fs::temp_directory_path() / "pp-scanner-test";
    fs::remove_all(root);
    fs::create_directories(root / "Vendor" / "Alpha.vst3" / "Contents");
    fs::create_directories(root / "Vendor" / "Beta.component");
    fs::create_directories(root / "Vendor" / "Docs");
    std::ofstream(root / "Vendor" / "Docs" / "manual.txt") << "x";

    const auto results = pp::scanDirectories({ root });

    EXPECT_TRUE(results.size() == 2);
    bool sawAlpha = false, sawBeta = false;
    for (const auto& r : results)
    {
        if (r.name == "Alpha" && r.format == pp::PluginFormat::VST3) sawAlpha = true;
        if (r.name == "Beta" && r.format == pp::PluginFormat::AudioUnit) sawBeta = true;
    }
    EXPECT_TRUE(sawAlpha);
    EXPECT_TRUE(sawBeta);

    fs::remove_all(root);
}

static void testBundlesAreNotDescendedInto()
{
    const fs::path root = fs::temp_directory_path() / "pp-scanner-nested-test";
    fs::remove_all(root);
    fs::create_directories(root / "Outer.vst3" / "Inner.vst3");

    const auto results = pp::scanDirectories({ root });
    EXPECT_TRUE(results.size() == 1);
    EXPECT_TRUE(!results.empty() && results[0].name == "Outer");

    fs::remove_all(root);
}

int main()
{
    testClassifyFormat();
    testDescribeUnknownIsUnsupported();
    testDescribeVst3IsAvailable();
    testScanMissingDirectoryIsEmpty();
    testScanFindsBundlesAndSkipsOthers();
    testBundlesAreNotDescendedInto();

    if (failures == 0)
        std::printf("All scanner tests passed.\n");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
