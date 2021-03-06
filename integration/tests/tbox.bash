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

tbox-source(){   echo $BASH_SOURCE ; }
tbox-vi(){       vi $(tbox-source) ; }
tbox-usage(){ cat << \EOU

tbox- : Pyrex Cube inside Mineral Oil Cube Test 
==================================================


`tbox-vi`
    edit the bash functions 

`tbox--`
    create Opticks geometry, simulates photons in interop mode, visualize, saves evt 

`tbox-- --compute`
    create Opticks geometry, simulates photons in compute mode, saves evt 

`tbox-- --okg4` 
    create Opticks and Geant4 geometry, simulates photons with both Opticks and G4, saves two evt 

`tbox-- --vizg4 --load`
    visualize the Geant4 simulated photon propagation 

`tbox-cf`
    compare Opticks and Geant4 material/flag sequence histories


`tbox-test`
    simulates with Opticks and Geant4 and compares the results 



EOU
}
tbox-env(){      olocal- ;  }
tbox-dir(){ echo $(dirname $(tbox-source)) ; }
tbox-cd(){  cd $(tbox-dir); }

join(){ local IFS="$1"; shift; echo "$*"; }

tbox-tag(){ echo 1 ; }
tbox-det(){ echo BoxInBox ; }
tbox-src(){ echo torch ; }




tbox-m2(){ echo Vacuum ; }

#tbox-m2(){ echo GlassSchottF2 ; }
#tbox-m2(){ echo MainH2OHale ; }
#tbox-m2(){ echo GdDopedLS ; }
#tbox-m2(){ echo MineralOil ; }


tbox-testconfig()
{
    local test_config=(
                 name=$FUNCNAME
                 mode=BoxInBox
                 analytic=1

                 node=box parameters=0,0,0,300 boundary=Rock//perfectAbsorbSurface/$(tbox-m2) 
                 node=box parameters=0,0,0,100 boundary=$(tbox-m2)///Pyrex 

                 ) 
    echo "$(join _ ${test_config[@]})" 
}

tbox-torchconfig()
{
    local torch_config=(
                 type=disclin
                 photons=500000
                 wavelength=480
                 frame=1
                 source=0,0,299
                 target=0,0,0
                 radius=100
                 zenithazimuth=0,1,0,1
                 material=Vacuum
               )
    echo "$(join _ ${torch_config[@]})" 
}


tbox--(){
    local cmdline=$*
    local tag=$(tbox-tag)

    local testconfig
    if [ -n "$TESTCONFIG" ]; then
        testconfig=${TESTCONFIG}
    else
        testconfig=$(tbox-testconfig)
    fi

    local torchconfig
    if [ -n "$TORCHCONFIG" ]; then
        torchconfig=${TORCHCONFIG}
    else
        torchconfig=$(tbox-torchconfig $pol)
    fi

    op.sh \
        --test --testconfig "$testconfig" \
        --torch --torchconfig "$torchconfig" \
        --animtimemax 10 \
        --timemax 10 \
        --cat $(tbox-det) --tag $tag --save  \
        --eye 0.5,0.5,0.0 \
        --rendermode +global \
        $* 
}

tbox-args(){  echo  --tag $(tbox-tag) --det $(tbox-det) --src $(tbox-src) ; }
tbox-py(){    tbox.py  $(tbox-args) $* ; } 
tbox-ipy(){   ipython -i $(which tbox.py) --  $(tbox-args) $* ; } 
tbox-t()
{
    tbox-- --okg4 --compute $*
    tbox-py 
}

tbox-v()
{
    tbox-- --okg4 --load $*
}

tbox-vg4()
{
    tbox-- --okg4 --vizg4 --load $*
}



