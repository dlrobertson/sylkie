#!/bin/bash

if [ -z "${GEN}" -o -z "${BUILD_TYPE}" ]
then
    echo "Must define GEN and BUILD_TYPE"
    exit 1
fi

mkdir ./build
cd build

cmake -G "${GEN}" -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DBUILD_TESTS=ON ..

if [[ "${GEN}" == "Ninja" ]]
then
    ninja
else
    make
fi

./sylkie_test
