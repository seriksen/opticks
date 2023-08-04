#!/bin/bash -l 
usage(){ cat << EOU
G4CXAppTest.sh 
================

::

    MODE=2 ./G4CXAppTest.sh ana


EOU
}

SDIR=$(cd $(dirname $BASH_SOURCE) && pwd)
U4TDIR=$(cd $SDIR/../../u4/tests && pwd)
BINDIR=$(cd $SDIR/../../bin && pwd)

bin=G4CXAppTest

defarg="info_run_ana"
#defarg="info_dbg_ana"
arg=${1:-$defarg}

source $HOME/.opticks/GEOM/GEOM.sh 

geomscript=$U4TDIR/$GEOM.sh
if [ -f "$geomscript" ]; then  
    source $geomscript


else
    echo $BASH_SOURCE : no geomscript $geomscript
fi 
ana=$SDIR/$bin.py 


export VERSION=0
export BASE=/tmp/$USER/opticks/GEOM/$GEOM/$bin
export EVT=001
export AFOLD=$BASE/ALL${VERSION}/p${EVT}
export BFOLD=$BASE/ALL${VERSION}/n${EVT}

#num_photons=1
#num_photons=10
#num_photons=100
#num_photons=1000      # 1k
num_photons=10000    # 10k
#num_photons=50000    # 50k 
#num_photons=100000   # 100k
#num_photons=1000000  # 1M

NUM_PHOTONS=${NUM_PHOTONS:-$num_photons}

export G4CXApp__PRIMARY_MODE=torch
export OPTICKS_MAX_BOUNCE=31  
export OPTICKS_EVENT_MODE=StandardFullDebug
export OPTICKS_INTEGRATION_MODE=3
export OPTICKS_MAX_PHOTON=${NUM_PHOTONS}

export SEvent_MakeGensteps_num_ph=${NUM_PHOTONS}
source $U4TDIR/storch_FillGenstep.sh
env | grep storch

logging(){
   export G4CXOpticks=INFO
   export QSim=INFO
   export QEvent=INFO
}
logging



vars="BASH_SOURCE SDIR U4TDIR BINDIR GEOM bin geomscript BASE FOLD ana PMTSimParamData_BASE" 

if [ "${arg/info}" != "$arg" ]; then 
    for var in $vars ; do printf "%20s : %s \n" "$var" "${!var}" ; done 
fi 

if [ "${arg/run}" != "$arg" ]; then
    $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE : run error && exit 1 
fi 

if [ "${arg/dbg}" != "$arg" ]; then
    case $(uname) in 
       Darwin) lldb__ $bin ;;
       Linux)  gdb__  $bin ;;
    esac
    [ $? -ne 0 ] && echo $BASH_SOURCE : dbg error && exit 2 
fi 

if [ "${arg/grab}" != "$arg" ]; then
    source $BINDIR/rsync.sh $BASE
    [ $? -ne 0 ] && echo $BASH_SOURCE : grab error && exit 3
fi 

if [ "${arg/ana}" != "$arg" ]; then
    ${IPYTHON:-ipython} --pdb -i $ana
    [ $? -ne 0 ] && echo $BASH_SOURCE : ana error && exit 4
fi 

exit 0 

