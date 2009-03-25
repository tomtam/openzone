#!/bin/sh

echo Generating src/base/CMakeLists.txt
cd src/base    && ./CMakeLists.gen && cd ../..
echo Generating src/matrix/CMakeLists.txt
cd src/matrix  && ./CMakeLists.gen && cd ../..
echo Generating src/nirvana/CMakeLists.txt
cd src/nirvana && ./CMakeLists.gen && cd ../..
echo Generating src/client/CMakeLists.txt
cd src/client  && ./CMakeLists.gen && cd ../..
echo Generating src/server/CMakeLists.txt
cd src/server  && ./CMakeLists.gen && cd ../..

cmake .
