#!/bin/sh
#
# data-cleanup.sh <datasrc_dir>
#
# Cleans up temporary, intermediate and backup files from a source game data directory.
#

if [[ -z $1 ]]; then
  echo "Usage: $0 <datasrc_dir>"
  exit
fi

dir=$1

echo "rm -vf "$dir"/baseq3/maps/*{autosave.map,.bak,.prt,.srf}"
rm -vf "$dir"/baseq3/maps/*{autosave.map,.bak,.prt,.srf,autosave.bsp}
echo "rm -vf $dir/mdl/*.{blend1,blend2}"
rm -vf "$dir"/mdl/*.{blend1,blend2}