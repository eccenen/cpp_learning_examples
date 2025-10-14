#!/usr/bin/env bash
# build.sh - convenience script to configure, build and run tests for the project.
# Usage:
#   ./build.sh            # configure+build using env VCPKG_PATH if set, else FetchContent fallback
#   VCPKG_PATH=/path/to/vcpkg ./build.sh
#   ./build.sh --toolchain /path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

set -euo pipefail
ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"

TOOLCHAIN_ARG=""
while [[ $# -gt 0 ]]; do
  case "$1" in
    -t|--toolchain)
      TOOLCHAIN_ARG="$2"
      shift 2
      ;;
    -h|--help)
      echo "Usage: $0 [--toolchain /path/to/vcpkg/scripts/buildsystems/vcpkg.cmake]"
      exit 0
      ;;
    *)
      echo "Unknown arg: $1"
      exit 1
      ;;
  esac
done

mkdir -p "$BUILD_DIR"

CMAKE_ARGS=( -S "$ROOT_DIR" -B "$BUILD_DIR" )
EXPORT_COMPILE_COMMANDS="ON"
CMAKE_ARGS+=( -DEXPORT_COMPILE_COMMANDS=${EXPORT_COMPILE_COMMANDS} )
if [[ -n "$TOOLCHAIN_ARG" ]]; then
  CMAKE_ARGS+=( -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_ARG" )
elif [[ -n "${VCPKG_PATH-}" ]]; then
  # If VCPKG_PATH is set but no explicit toolchain provided, automatically
  # use the vcpkg toolchain file so find_package can discover vcpkg-installed
  # packages (like doctest). Also pass VCPKG_PATH into CMake for fallback include lookup.
  TOOLCHAIN_FILE="$VCPKG_PATH/scripts/buildsystems/vcpkg.cmake"
  if [[ -f "$TOOLCHAIN_FILE" && -z "$TOOLCHAIN_ARG" ]]; then
    CMAKE_ARGS+=( -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" )
  fi
  CMAKE_ARGS+=( -DVCPKG_PATH="$VCPKG_PATH" )
  # Optionally pass triplet if provided in env
  if [[ -n "${VCPKG_TARGET_TRIPLET-}" ]]; then
    CMAKE_ARGS+=( -DVCPKG_TARGET_TRIPLET="$VCPKG_TARGET_TRIPLET" )
  fi
fi

# If no toolchain arg and no VCPKG_PATH, try common install locations for vcpkg
if [[ -z "$TOOLCHAIN_ARG" && -z "${VCPKG_PATH-}" ]]; then
  for candidate in "$HOME/software/vcpkg" "$HOME/vcpkg" "/opt/vcpkg"; do
    if [[ -f "$candidate/scripts/buildsystems/vcpkg.cmake" ]]; then
      TOOLCHAIN_ARG="$candidate/scripts/buildsystems/vcpkg.cmake"
      echo "Auto-detected vcpkg toolchain at: $TOOLCHAIN_ARG"
      CMAKE_ARGS+=( -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_ARG" )
      # Also expose VCPKG_PATH for CMakeLists fallback logic
      CMAKE_ARGS+=( -DVCPKG_PATH="$candidate" )
      break
    fi
  done
fi

echo "Configuring with: cmake ${CMAKE_ARGS[*]}"
cmake "${CMAKE_ARGS[@]}"

echo "Building..."
cmake --build "$BUILD_DIR" -- -j

# # Run tests if available
# if [[ -x "$BUILD_DIR/test_heap_only" ]]; then
#   echo "Running tests..."
#   "$BUILD_DIR/test_heap_only"
# else
#   echo "No test binary found in $BUILD_DIR"
# fi

# For clangd and other tools, place compile_commands.json at repository root as a symlink
if [[ -f "$BUILD_DIR/compile_commands.json" ]]; then
  ln -sf "$BUILD_DIR/compile_commands.json" "$ROOT_DIR/compile_commands.json"
  echo "Created symlink: $ROOT_DIR/compile_commands.json -> $BUILD_DIR/compile_commands.json"
fi
