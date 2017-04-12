#!/bin/sh

BUILD_ROOT=$(dirname $(readlink -f $0))

CONFIGURATION="$VIM_PROJECT_CONFIGURATION"
if [ "$CONFIGURATION" = "" ]; then
	CONFIGURATION=Debug
fi
TEMP_DIR=${BUILD_ROOT}/.tmp/linux-${CONFIGURATION}

echo "make: Entering Directory '${TEMP_DIR}'"
mkdir -p ${TEMP_DIR} 
cd ${TEMP_DIR} &&
CC=clang CXX=clang++ cmake \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
	-DCMAKE_BUILD_TYPE=${CONFIGURATION} \
	-G "Ninja" ${BUILD_ROOT} &&
ASAN_OPTIONS="detect_leaks=0" ninja
