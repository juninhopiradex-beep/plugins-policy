# Plugins Policy by Piradex

Early build preview: scans installed VST3/AU package names and inspects WAV technical metadata.

**Not implemented:** plugin hosting, automatic DSP testing, scores, comparison, PDF reports, AAX support.

GitHub Actions creates an unsigned Windows portable EXE and an unsigned Intel Mac DMG. macOS Catalina 10.15.7 is a target, not verified: test on a Catalina machine before claiming support. Windows output is not a Setup installer.

Future work: JUCE-based native VST3/AU host, controlled test signals, category-aware calibrated scoring, detailed graphs and reports.

## Native build — milestone M1 (Fundação)

Location: `native/`. Status per feature:

| Item | Status |
|---|---|
| Scanner core: VST3/AU classification, bundle discovery, Scan/Available/Unsupported/Failed states | Implemented; unit-tested (`pp_tests`) |
| Default VST3/AU directories (macOS and Windows) | Implemented; not verified on a real machine |
| JUCE desktop shell: sidebar, plugin list, Analyze control, "Not tested" score area | Implemented; compiles only in CI (not compiled locally) |
| Plugin loading, DSP measurements, scoring, graphs, PDF/CSV reports, comparison, history | Not implemented (M2–M5) |
| Windows Setup installer (.exe/.msi) and macOS .dmg from native build | Not implemented (M5); CI currently produces unverified artifacts only |
| Code signing and Apple notarization | Not implemented; no certificates available |

Limitations:
- macOS 10.15.7 Catalina is the target. The CI job checks that the app builds against the 10.15 deployment target on an Intel runner. It does not prove the app runs on Catalina; that needs a real test on a Catalina machine.
- The JUCE version is pinned to 8.0.4 in `native/CMakeLists.txt`.

Build and test the core locally:

    cmake -S native -B native/build -DPP_BUILD_TESTS=ON
    cmake --build native/build
    ctest --test-dir native/build --output-on-failure
