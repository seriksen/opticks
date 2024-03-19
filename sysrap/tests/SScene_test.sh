#!/bin/bash -l 
usage(){ cat << EOU
SScene_test.sh
===============

Inits SScene from loaded stree.h::

   ~/o/sysrap/tests/SScene_test.sh 


To create and persist the stree starting from GEOM gdml::

   ~/o/u4/tests/U4TreeCreateTest.sh


EOU
}

cd $(dirname $(realpath $BASH_SOURCE))

name=SScene_test
export SSCENE_FOLD=/tmp/$name
mkdir -p $SSCENE_FOLD 
bin=$SSCENE_FOLD/$name

stree_fold=$TMP/U4TreeCreateTest
export STREE_FOLD=${STREE_FOLD:-$stree_fold}

if [ ! -d "$STREE_FOLD/stree" ]; then
   echo $BASH_SOURCE : ERROR STREE_FOLD $STREE_FOLD DOES NOT CONTAIN stree 
   exit 1
fi 

vars="BASH_SOURCE PWD stree_fold STREE_FOLD SSCENE_FOLD bin"

defarg=info_build_run
arg=${1:-$defarg}

cuda_prefix=/usr/local/cuda
CUDA_PREFIX=${CUDA_PREFIX:-$cuda_prefix}

opticks-  
glm-


export NPFold__load_DUMP=1
export NPFold__load_index_DUMP=1
export NPFold__load_dir_DUMP=1

if [ "${arg/info}" != "$arg" ]; then 
    for var in $vars ; do printf "%20s : %s \n" "$var" "${!var}" ; done
fi   

if [ "${arg/build}" != "$arg" ]; then 

    echo $BASH_SOURCE [ building 
    gcc  $name.cc \
         ../sn.cc \
         ../s_pa.cc \
         ../s_tv.cc \
         ../s_bb.cc \
         ../s_csg.cc \
         -DWITH_CHILD \
         -std=c++11 -lstdc++ -lm \
         -I.. -g \
         -I$(glm-prefix) \
         -I${CUDA_PREFIX}/include \
         -o $bin

    [ $? -ne 0 ] && echo $BASH_SOURCE build error && exit 1
    echo $BASH_SOURCE ] building 
fi 

if [ "${arg/run}" != "$arg" ]; then 
    $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE run error && exit 2
fi 

if [ "${arg/dbg}" != "$arg" ]; then 
    dbg__ $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE dbg error && exit 3
fi 


exit 0 
