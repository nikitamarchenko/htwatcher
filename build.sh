#!/usr/bin/env bash

if [ "${BUILD_TYPE}" = "Debug" ]; then
  mkdir -p cmake-build-debug
  cd cmake-build-debug
else
  BUILD_TYPE=Release
  mkdir -p cmake-build-release
  cd cmake-build-release
fi

if [ "${SERVER42_BUILD_TARGETS}" = "" ]; then
    SERVER42_BUILD_TARGETS="Debug"
fi

#boost dpdk pcapplusplus zeromq protobuf

echo "Build type set to ${BUILD_TYPE}"
echo "Build targets set to ${HTWATCHER_BUILD_TARGETS}"

../cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -G "CodeBlocks - Unix Makefiles" ../
../cmake --build . --target ${HTWATCHER_BUILD_TARGETS} -- -j4
