#!/bin/bash -l

cmdline="$*"
ggeoview-

dbg=0
if [ "${cmdline/--dbg}" != "${cmdline}" ]; then
   dbg=1
fi

if [ "${cmdline/--oac}" != "${cmdline}" ]; then
   export OPTIX_API_CAPTURE=1
fi


if [ "${cmdline/--cmp}" != "${cmdline}" ]; then
   export GGEOVIEW_BINARY=$(ggeoview-compute-bin)
elif [ "${cmdline/--loader}" != "${cmdline}" ]; then
   export GGEOVIEW_BINARY=$(ggeoview-loader-bin)
else
   unset GGEOVIEW_BINARY 
fi



ggeoview_defaults_dyb(){

   export GGEOVIEW_GEOKEY=DAE_NAME_DYB
   export GGEOVIEW_QUERY="range:3153:12221"
   export GGEOVIEW_CTRL="volnames"
   export GGEOVIEW_MESHFIX="iav,oav"
   export GGEOVIEW_MESHFIX_CFG="100,100,10,-0.999"   # face barycenter xyz alignment and dot face normal cuts for faces to be removed 

   #export GGEOVIEW_QUERY="range:3153:4814"     #  transition to 2 AD happens at 4814 
   #export GGEOVIEW_QUERY="range:3153:4813"     #  this range constitutes full single AD
   #export GGEOVIEW_QUERY="range:3161:4813"      #  push up the start to get rid of plain outer volumes, cutaway view: udp.py --eye 1.5,0,1.5 --look 0,0,0 --near 5000
   #export GGEOVIEW_QUERY="index:5000"
   #export GGEOVIEW_QUERY="index:3153,depth:25"
   #export GGEOVIEW_QUERY="range:5000:8000"
}


if [ "${cmdline/--juno}" != "${cmdline}" ]; then

   export GGEOVIEW_GEOKEY=DAE_NAME_JUNO
   export GGEOVIEW_QUERY="range:1:50000"
   export GGEOVIEW_CTRL=""

elif [ "${cmdline/--jpmt}" != "${cmdline}" ]; then

   export GGEOVIEW_GEOKEY=DAE_NAME_JPMT
   export GGEOVIEW_QUERY="range:1:289734"  # 289733+1 all test3.dae volumes
   export GGEOVIEW_CTRL=""

elif [ "${cmdline/--jtst}" != "${cmdline}" ]; then

   export GGEOVIEW_GEOKEY=DAE_NAME_JTST
   export GGEOVIEW_QUERY="range:1:50000" 
   export GGEOVIEW_CTRL=""

elif [ "${cmdline/--dyb}" != "${cmdline}" ]; then

   ggeoview_defaults_dyb

elif [ "${cmdline/--idyb}" != "${cmdline}" ]; then

   ggeoview_defaults_dyb
   export GGEOVIEW_QUERY="range:3158:3160"       # 2 volumes : pvIAV and pvGDS

elif [ "${cmdline/--jdyb}" != "${cmdline}" ]; then

   ggeoview_defaults_dyb
   export GGEOVIEW_QUERY="range:3158:3159"       # 1 volume : pvIAV
   
elif [ "${cmdline/--kdyb}" != "${cmdline}" ]; then

   ggeoview_defaults_dyb
   export GGEOVIEW_QUERY="range:3159:3160"       # 1 volume : pvGDS

elif [ "${cmdline/--ldyb}" != "${cmdline}" ]; then

   ggeoview_defaults_dyb
   export GGEOVIEW_QUERY="range:3156:3157"       # 1 volume : pvOAV


else

   ggeoview_defaults_dyb

fi









if [ "${cmdline/--make}" != "${cmdline}" ]; then
   optixrap-
   cu=$(optixrap-sdir)/cu/generate.cu 
   touch $cu
   ls -l $cu
   ggeoview-install 
fi


if [ "${cmdline/--idp}" != "${cmdline}" ]; then

    echo $(ggeoview-run $*)

elif [ "${cmdline/--assimp}" != "${cmdline}" ]; then

    assimpwrap-
    ggeoview-export
    $(assimpwrap-bin) GGEOVIEW_

elif [ "${cmdline/--openmesh}" != "${cmdline}" ]; then

    openmeshrap-
    ggeoview-export
    $(openmeshrap-bin) 

else
    case $dbg in
       0)  ggeoview-run $*  ;;
       1)  ggeoview-dbg $*  ;;
    esac
fi

