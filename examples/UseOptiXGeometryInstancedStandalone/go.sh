#!/bin/bash -l
##
## Copyright (c) 2019 Opticks Team. All Rights Reserved.
##
## This file is part of Opticks
## (see https://bitbucket.org/simoncblyth/opticks).
##
## Licensed under the Apache License, Version 2.0 (the "License"); 
## you may not use this file except in compliance with the License.  
## You may obtain a copy of the License at
##
##   http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software 
## distributed under the License is distributed on an "AS IS" BASIS, 
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
## See the License for the specific language governing permissions and 
## limitations under the License.
##


sdir=$(pwd)
name=$(basename $sdir)

prefix=/tmp/$USER/opticks/$name

export EXAMPLE_NAME=$name
export EXAMPLE_PREFIX=$prefix
export PATH=$prefix/bin:$PATH

bdir=$prefix/build 
echo bdir $bdir name $name prefix $prefix



glm-dir(){  echo $prefix/externals/glm/$(glm-name) ; }
glm-version(){ echo 0.9.9.5 ; }
glm-name(){    echo glm-$(glm-version) ; }
glm-url(){    echo https://github.com/g-truc/glm/releases/download/$(glm-version)/$(glm-name).zip ; }
glm-dist(){    echo $(dirname $(glm-dir))/$(basename $(glm-url)) ; }

glm-get(){
   local msg="=== $FUNCNAME :"
   local dir=$(dirname $(glm-dir)) &&  mkdir -p $dir && cd $dir
   local url=$(glm-url)
   local zip=$(basename $url)
   local nam=$(glm-name)
   local opt=$( [ -n "${VERBOSE}" ] && echo "" || echo "-q" )

   local hpp=$nam/glm/glm/glm.hpp
   echo $msg nam $nam PWD $PWD hpp $hpp
   ## curiously directories under /tmp being emptied but directory structure
   ## remains, so have to check file rather than directory existance  

   [ ! -f "$zip" ] && curl -L -O $url
   [ ! -f "$hpp" ] && unzip $opt $zip -d $nam
   ln -sfnv $(glm-name)/glm glm 
   echo symbolic link for access without version in path
}


glm-get

optix-install-dir(){ opticks- ; echo ${OPTIX_INSTALL_DIR:-$(opticks-dir)/externals/OptiX} ; }
echo optix-install-dir : $(optix-install-dir)

#rm -rf $bdir && mkdir -p $bdir 
cd $bdir && pwd 
ls -l 

if [ ! -f CMakeCache.txt ] ; then 
    cmake $sdir \
       -DCMAKE_BUILD_TYPE=Debug \
       -DCMAKE_PREFIX_PATH=$prefix/externals \
       -DCMAKE_INSTALL_PREFIX=$prefix \
       -DCMAKE_MODULE_PATH=$(optix-install-dir)/SDK/CMake \
       -DOptiX_INSTALL_DIR=$(optix-install-dir) 

    rm -rf $prefix/ptx
    rm -rf $prefix/bin
fi 


rm -rf $prefix/ppm
mkdir -p $prefix/{ptx,bin,ppm} 

make
make install   


export CUDA_VISIBLE_DEVICES=0

echo running $(which $name)
RTX=0 $name
rc=$? ; [ ! $rc -eq 0 ] && echo non-zero RC && exit

RTX=1 PPM=1 $name
#RTX=-1 PPM=1 $name

ppm=$prefix/ppm/$name.ppm
[ ! -f "$ppm" ] && echo failed to write ppm $ppm && exit

echo ppm $ppm
ls -l $ppm
open $ppm    ## create an open function such as "gio open" if using gnome


