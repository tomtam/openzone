#!/bin/sh

[[ -z "$1" ]] && exit

dir="$1"

echo rm -f $dir'/lingua/*/*.{ozCat,mo}'
rm -f $dir/lingua/*/*.{ozCat,mo}
echo rm -f $dir'/ui/cur/*.ozcCur'
rm -f $dir/ui/cur/*.ozcCur
echo rm -f $dir'/ui/icon/*.ozcTex'
rm -f $dir/ui/icon/*.ozcTex
echo rm -f $dir'/terra/*.{ozTerra,ozcTerra}'
rm -f $dir/terra/*.{ozTerra,ozcTerra}
echo rm -f $dir'/caelum/*.ozcCaelum'
rm -f $dir/caelum/*.ozcCaelum
echo rm -f $dir'/data/maps/*{autosave.map,.bak}'
rm -f $dir/data/maps/*{autosave.map,.bak}
echo rm -f $dir'/data/maps/*.{bsp,prt,srf}'
rm -f $dir/data/maps/*.{bsp,prt,srf}
echo rm -f $dir'/bsp/*.{ozBSP,ozcBSP}'
rm -f $dir/bsp/*.{ozBSP,ozcBSP}
echo rm -f $dir'/bsp/*/*.ozcTex'
rm -f $dir/bsp/*/*.ozcTex
echo rm -f $dir'/mdl/*.{ozcSMM,ozcMD2,ozcMD3}'
rm -f $dir/mdl/*.{ozcSMM,ozcMD2,ozcMD3}
echo rm -f $dir'/mdl/*/*.{blend1,blend2}'
rm -f $dir/mdl/*/*.{blend1,blend2}
echo rm -f $dir'/ui/galileo/*.ozcTex'
rm -f $dir/ui/galileo/*.ozcTex
