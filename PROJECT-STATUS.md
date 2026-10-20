# Plugins Policy by Piradex

Early build preview: scans installed VST3/AU package names and inspects WAV technical metadata.

**Not implemented:** plugin hosting, automatic DSP testing, scores, comparison, PDF reports, AAX support.

GitHub Actions creates an unsigned Windows portable EXE and an unsigned Intel Mac DMG. macOS Catalina 10.15.7 is a target, not verified: test on a Catalina machine before claiming support. Windows output is not a Setup installer.

Future work: JUCE-based native VST3/AU host, controlled test signals, category-aware calibrated scoring, detailed graphs and reports.
