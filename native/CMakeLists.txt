cmake_minimum_required(VERSION 3.22)

# Plugins Policy by Piradex — native build (milestone M1).
# Targets:
#   pp_core       plugin discovery core, no JUCE dependency (built and tested everywhere)
#   pp_tests      unit tests for pp_core (run with ctest)
#   PluginsPolicy JUCE desktop app (only when PP_BUILD_APP=ON; built in CI)

project(PluginsPolicy VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Catalina (10.15) is the P0 macOS target. Do not raise without testing on Catalina.
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
endif()

option(PP_BUILD_APP "Build the JUCE desktop application" OFF)
option(PP_BUILD_TESTS "Build unit tests" ON)

add_library(pp_core STATIC
    core/src/Scanner.cpp
)
target_include_directories(pp_core PUBLIC core/include)
target_compile_options(pp_core PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

if(PP_BUILD_TESTS)
    enable_testing()
    add_executable(pp_tests tests/ScannerTests.cpp)
    target_link_libraries(pp_tests PRIVATE pp_core)
    add_test(NAME scanner COMMAND pp_tests)
endif()

if(PP_BUILD_APP)
    include(FetchContent)
    # JUCE is pinned. Bump deliberately, and re-test on Catalina after any change.
    set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
    set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(JUCE_ENABLE_MODULE_SOURCE_GROUPS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)

    juce_add_gui_app(PluginsPolicy
        PRODUCT_NAME "Plugins Policy"
        COMPANY_NAME "Piradex"
        BUNDLE_ID "com.piradex.pluginspolicy"
        VERSION ${PROJECT_VERSION}
    )

    target_sources(PluginsPolicy PRIVATE app/Main.cpp)
    target_include_directories(PluginsPolicy PRIVATE core/include)
    target_link_libraries(PluginsPolicy PRIVATE
        pp_core
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
    )
    target_compile_definitions(PluginsPolicy PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
    )
endif()
