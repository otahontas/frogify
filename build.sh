#!/bin/sh

# Example:
# ./build.sh VST3 Release

# Target is the project name
target=frogify

# Second arg: Format of the target VST3/AU(MacOS only)/Standalone(default)
format=${1:-Standalone}

# Thrid arg: Build config. Release/Debug(default)
config=${2:-Debug}

# Fourth arg: Number of concurrent build jobs, 4 by default
jobs=${4:-4}

cmake --build build --target ${target}_${format} --config ${config} --parallel ${jobs}
