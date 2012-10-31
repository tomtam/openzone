#!/bin/sh

cat << EOF > CMakeLists.txt
add_library( common STATIC
  `echo *.{hh,cc} | sed 's| |\n  |g'` )
use_pch( common pch )
target_link_libraries( common \${LUA_LIBRARY} \${SDL_LIBRARY} oz )
EOF
