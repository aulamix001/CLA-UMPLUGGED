cmake_minimum_required(VERSION 3.22)
project(UnpluggedStudy VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(UNPLUGGED_BUILD_PLUGIN "Build JUCE VST3 and standalone" ON)
option(UNPLUGGED_SANITIZERS "Enable ASan/UBSan for native tests" OFF)
include(CTest)

add_library(unplugged_dsp STATIC Source/DspEngine.cpp Source/Presets.cpp)
target_include_directories(unplugged_dsp PUBLIC Source)
set_target_properties(unplugged_dsp PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_executable(preset_export Tools/ExportPresets.cpp)
target_link_libraries(preset_export PRIVATE unplugged_dsp)
if(MSVC)
    target_compile_options(unplugged_dsp PRIVATE /W4)
else()
    target_compile_options(unplugged_dsp PRIVATE -Wall -Wextra -Wpedantic)
    if(UNPLUGGED_SANITIZERS)
        target_compile_options(unplugged_dsp PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(unplugged_dsp PUBLIC -fsanitize=address,undefined)
    endif()
endif()
if(BUILD_TESTING)
    add_executable(dsp_tests Tests/DspTests.cpp)
    target_link_libraries(dsp_tests PRIVATE unplugged_dsp)
    add_test(NAME dsp_tests COMMAND dsp_tests)
endif()

if(UNPLUGGED_BUILD_PLUGIN)
    file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/Assets/cla-unplugged-original.png" artwork_sha256)
    if(NOT artwork_sha256 STREQUAL "1179a8a2e3db3da08a19e2cb31396130cd84ed0928a6b606129ef62ab0a75ba3")
        message(FATAL_ERROR "The approved artwork must remain byte-for-byte unchanged.")
    endif()
    if(WIN32 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "This project requires Windows x64.")
    endif()
    set(JUCE_DIR "" CACHE PATH "Optional existing JUCE 8.0.12 checkout")
    if(JUCE_DIR)
        add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")
    else()
        include(FetchContent)
        FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG 29396c22c93392d6738e021b83196283d6e4d850)
        FetchContent_MakeAvailable(JUCE)
    endif()
    juce_add_binary_data(UnpluggedArtwork SOURCES Assets/cla-unplugged-original.png)
    juce_add_plugin(UnpluggedStudy
        COMPANY_NAME "Aulamix"
        BUNDLE_ID "com.aulamix.unpluggedstudy"
        PLUGIN_MANUFACTURER_CODE Aumx
        PLUGIN_CODE Uplg
        PRODUCT_NAME "Unplugged Study"
        FORMATS VST3 Standalone
        IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        VST3_AUTO_MANIFEST FALSE
        VST3_CATEGORIES Fx Reverb)
    target_sources(UnpluggedStudy PRIVATE
        Source/PluginProcessor.cpp Source/PluginProcessor.h
        Source/PluginEditor.cpp Source/PluginEditor.h)
    target_compile_definitions(UnpluggedStudy PUBLIC
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
    target_link_libraries(UnpluggedStudy PRIVATE
        unplugged_dsp UnpluggedArtwork
        juce::juce_audio_utils
        PUBLIC juce::juce_recommended_config_flags)
    if(BUILD_TESTING)
        juce_add_console_app(processor_tests PRODUCT_NAME "Processor tests")
        target_sources(processor_tests PRIVATE Tests/ProcessorTests.cpp)
        target_include_directories(processor_tests PRIVATE Source)
        target_link_libraries(processor_tests PRIVATE UnpluggedStudy juce::juce_audio_utils)
        target_compile_definitions(processor_tests PRIVATE JUCE_PLUGINHOST_VST3=1)
        add_dependencies(processor_tests UnpluggedStudy_VST3)
        add_test(NAME processor_tests COMMAND processor_tests
            "$<TARGET_FILE_DIR:UnpluggedStudy_VST3>/../..")
    endif()
endif()
