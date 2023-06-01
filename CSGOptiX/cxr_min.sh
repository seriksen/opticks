#!/bin/bash -l 
usage(){ cat << EOU
cxr_min.sh
===========

Shakedown cxr.sh scripts using this minimal approach. 

::

    EYE=0.2,0.2,0.2 TMIN=0.1 ./cxr_min.sh
    EYE=0.3,0.3,0.3 TMIN=0.1 ./cxr_min.sh


    EYE=10,10,10 TMIN=0.5 MOI=Hama:0:0 ./cxr_min.sh    ## invisible 
    EYE=100,100,100 TMIN=0.1 MOI=Hama:0:1000 ./cxr_min.sh 
    EYE=1000,1000,1000 TMIN=0.5 MOI=NNVT:0:0 ./cxr_min.sh  ## makes sense

HUH: suspect problem with frame targetting messing up extent units
TODO: dump the frame for debugging and save view config with renders
    

EOU
}

defarg=run_info
arg=${1:-$defarg}

export OPTICKS_HASH=$(git -C $OPTICKS_HOME rev-parse --short HEAD)

pkg=CSGOptiX
#bin=CSGOptiXRenderTest
bin=CSGOptiXRdrTest

geom=V0J008
tmin=0.5
moi=sWorld:0:0

export MOI=${MOI:-$moi}
export CSGOptiX=INFO
export GEOM=${GEOM:-$geom}
export ${GEOM}_CFBaseFromGEOM=$HOME/.opticks/GEOM/$GEOM
export TMIN=${TMIN:-$tmin}

# as a file is written in pwd need to cd 


BASE=/tmp/$USER/opticks/GEOM/$GEOM/$bin
export LOGDIR=$BASE
mkdir -p $LOGDIR 
cd $LOGDIR 

LOG=$bin.log

vars="GEOM TMIN LOGDIR OPTICKS_HASH"
for var in $vars ; do printf "%20s : %s \n" $var ${!var} ; done 

if [ "${arg/run}" != "$arg" ]; then

   if [ -f "$LOG" ]; then 
       echo $BASH_SOURCE : run : delete prior LOG $LOG 
       rm "$LOG" 
   fi 
   $bin

   [ $? -ne 0 ] && echo $BASH_SOURCE run error && exit 1 
   # HMM: rename the log using the hash or some input var ? 
fi 

if [ "${arg/info}" != "$arg" ]; then
   for var in $vars ; do printf "%20s : %s \n" $var ${!var} ; done 
fi 
