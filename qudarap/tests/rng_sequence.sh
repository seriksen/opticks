#!/bin/bash -l

usage(){ cat << EOU
rng_sequence.sh 
==================

Usage::

   ./rng_sequence.sh run      # generates and persists precooked random .npy arrays

   ./rng_sequence.sh ana      # load the random .npy arrays 


EOU
}


arg=${1:-ana}

## defining the below envvar changes the output directory to be within ~/.opticks/precooked
export QSimTest__rng_sequence_PRECOOKED=1    

TEST=rng_sequence ./QSimTest.sh $arg


