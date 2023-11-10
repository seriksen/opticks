#!/bin/bash -l
sdir=$(pwd)
name=UseRelease

TMP=${TMP:-/tmp/$USER/opticks}
bdir=$TMP/${name}.build 
idir=$TMP/${name}.install
bin=$idir/lib/$name

# -DGeant4_DIR=$(opticks-envg4-Geant4_DIR)
# SHOULD BE IN ENV ALREADY ? 

echo === $BASH_SOURCE : CMAKE_PREFIX_PATH
echo $CMAKE_PREFIX_PATH | tr ":" "\n"

rm -rf $bdir 
rm -rf $idir 

if [ ! -d "$bdir" ]; then 
   mkdir -p $bdir && cd $bdir 
   cmake $sdir \
    -DCMAKE_BUILD_TYPE=Debug \
    -DOPTICKS_PREFIX=$OPTICKS_PREFIX \
    -DCMAKE_MODULE_PATH=$OPTICKS_PREFIX/cmake/Modules \
    -DCMAKE_PREFIX_PATH="$OPTICKS_PREFIX/externals;$OPTICKS_PREFIX" \
    -DCMAKE_INSTALL_PREFIX=$idir
else
   cd $bdir 
fi 

pwd
make
rc=$?
[ "$rc" != "0" ] && exit $rc 

make install   

echo === $BASH_SOURCE : $bin
$bin

