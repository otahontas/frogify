# Frogify

Make your input to sound more and more froggy

## Pre-requisites

**MacOS**:
 - Command line tools for dev
 - clang (apple default is ok)
 - cmake

## Setup

Clone the repo with submodules:
```sh
git clone --recurse-submodules [repo url]
```

## Building

### Debug mode

(allows debug symbols + extra warnings for debug builds)
Run:
```sh
./configure.sh Debug
./build.sh dynamics_processor [Standalone/AU/VST3]
```

### Release mode

Run:
```sh
./configure.sh Release
./build.sh dynamics_processor [Standalone/AU/VST3]
```
