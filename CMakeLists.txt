cmake_minimum_required(VERSION 3.25)

# Windows RunTime Library
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Set universal binary build on MacOS
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")

# Set macOS target deployment version
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")

# Define project
project(frogify VERSION 0.0.1 LANGUAGES C CXX)

# Xcode 15 linker workaround
# If you are using Link Time Optimisation (LTO), the new linker
# introduced in Xcode 15 may produce a broken binary.
# As a workaround, add either '-Wl,-weak_reference_mismatches,weak'
# or '-Wl,-ld_classic' to your linker flags.
# Once you've selected a workaround, you can add
# JUCE_SILENCE_XCODE_15_LINKER_WARNING to your preprocessor
# definitions to silence this warning.
if(APPLE)
    set(xcode_15_linker -Wl,-ld_classic)
endif()

# Some windows specific defines
if(WIN32)
    set(windows_defines _USE_MATH_DEFINES WIN32_LEAN_AND_MEAN)
endif()

# Some Linux specific defines (libcurl does not link for whatever reason)
if(LINUX)
    set(linux_defines JUCE_USE_CURL=0 JUCE_JACK=1)
endif()

# Add JUCE
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/JUCE)

# Add MRTA module
juce_add_module(${CMAKE_CURRENT_SOURCE_DIR}/modules/mrta_utils)

# General variables
set(company_name "Otahontas")
set(company_copyright "Copyright (c) 2025 Otahontas")
set(company_code "Otah")
set(company_bundle_id "com.otahontas")

# This function adds a JUCE plugin target to this project
# Arguments:
#   - TARGET: The name of your plugin target, this allows you to identify
#   - which of your plugins will be built.
#   - VERSION: Version on your plugin.
#   - PLUGIN_NAME: The name which will be displayed on the DAW.
#   - PROD_NAME: Internal product name, cannot contain whitespace.
#   - PROD_CODE: 4 letter code unique identifier to your plugin, at least
#   - 1 capitalized letter.
#   - SYNTH: Set to true if your plugin is a synth, false otherwise.
#   - SOURCES: A list of all the source files of you plugin.
#   - INCLUDE_DIRS: A list of the include directories required by your sources.
function(add_plugin target)
    # parse input args
    set(one_value_args
        TARGET
        VERSION
        PLUGIN_NAME
        PROD_NAME
        PROD_CODE
        SYNTH
    )
    set(multi_value_args SOURCES INCLUDE_DIRS)
    cmake_parse_arguments(
        AP
        ""
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )

    # info and debug
    message(STATUS "Adding JUCE plugin target: ${target}")
    message(STATUS "  PLUGIN_NAME: ${AP_PLUGIN_NAME}")
    message(STATUS "  VERSION: ${AP_VERSION}")
    message(STATUS "  PROD_NAME: ${AP_PROD_NAME}")
    message(STATUS "  PROD_CODE: ${AP_PROD_CODE}")
    message(STATUS "  SYNTH: ${AP_SYNTH}")

    # Add juce plugin target
    juce_add_plugin(${target}
        PRODUCT_NAME ${AP_PROD_NAME}
        VERSION ${AP_VERSION}
        MICROPHONE_PERMISSION_ENABLED TRUE
        COMPANY_COPYRIGHT ${company_copyright}
        COMPANY_NAME ${company_name}
        FORMATS Standalone VST3 AU Standalone
        PLUGIN_NAME ${AP_PLUGIN_NAME}
        PLUGIN_MANUFACTURER_CODE ${company_code}
        PLUGIN_CODE ${AP_PROD_CODE}
        BUNDLE_ID ${company_bundle_id}.${AP_PROD_NAME}
        IS_SYNTH ${AP_SYNTH}
        NEEDS_MIDI_INPUT ${AP_SYNTH}
        NEEDS_MIDI_OUTPUT FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
    )
    juce_generate_juce_header(${target})

    target_sources(${target} PRIVATE ${AP_SOURCES})

    target_include_directories(
        ${target}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/asiosdk/common
            ${AP_INCLUDE_DIRS}
    )

    target_compile_features(${target} PUBLIC cxx_std_17)

    target_compile_definitions(
        ${target}
        PRIVATE
            JUCE_ASIO=1
            JUCE_DIRECTSOUND=0
            JUCE_USE_FLAC=0
            JUCE_USE_OGGVORBIS=0
            JUCE_USE_WINDOWS_MEDIA_FORMAT=0
            JUCE_WEB_BROWSER=0
            JUCE_VST3_CAN_REPLACE_VST2=0
            JUCE_SILENCE_XCODE_15_LINKER_WARNING=1
            ${windows_defines}
            ${linux_defines}
    )

    target_link_libraries(
        ${target}
        PRIVATE juce::juce_audio_utils juce::juce_dsp mrta_utils
        PUBLIC
            ${xcode_15_linker}
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
    )
    # Apply -Wall, -Wshadow, and -Werror specifically for Debug builds
    # when using a Clang-based compiler.
    if(
        CMAKE_CXX_COMPILER_ID MATCHES "Clang"
        OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang"
    )
        message(
            STATUS
            "Configuring Clang specific warning flags for ${target} (active in Debug builds only: -Wall -Wshadow -Werror)"
        )
        target_compile_options(
            ${target}
            PRIVATE "$<$<CONFIG:Debug>:-Wall;-Wshadow;-Werror>"
        )
    else()
        message(
            STATUS
            "Skipping custom Clang-specific warning flags for ${target} (compiler is ${CMAKE_CXX_COMPILER_ID}). This block only targets Clang."
        )
    endif()
endfunction()

# project files
set(source ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_plugin(frogify
  VERSION 0.1.0
  PLUGIN_NAME "Frogify"
  PROD_NAME Frogify
  PROD_CODE FROG
  SYNTH FALSE
  SOURCES
    ${source}/PluginEditor.cpp
    ${source}/PluginProcessor.cpp
  INCLUDE_DIRS
        ${source}
)
