#!/bin/sh

source=`echo src/*/*.{hh,cc} src/stable.hh`
data=`echo data/*/*.rc data/lua/*/*.lua`
build=`echo src/**/CMakeLists.gen {src,data}/**/CMakeLists.txt CMakeLists.txt src/*.in *.sh`

function count()
{
  printf '%s (' "$1"
  shift
  (( nFiles = 0 ))
  for file in $@; do (( nFiles++ )); done
  printf '%d files):\n' $nFiles
  wc -lc $@
  echo
}

count 'C++ Source' $source
count 'Data config & Lua scripts' $data
count 'Build system etc.' $build

if [ -x /usr/bin/sloccount ]; then
  LANG=C /usr/bin/sloccount src *.sh data/lua
fi
