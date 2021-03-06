lifting-the-3M-photon-limitation
==================================


Next
-----------

* :doc:`plugging-cfg4-leaks`
* :doc:`py-analysis-too-slow-for-big-events`


Issues : 3M photon limit from curandState and 100k limit from precooked rng 
--------------------------------------------------------------------------------

::

     ts box --generateoverride 3000001                ## asserts from enoughRng too many photons
     ts box --generateoverride 3000001 --rngmax 10    ## asserts while doing the cfg4 sim, from out_of_range 10k 
                                                      

Huh CRandomEngine::initCurand not asserting immediately::

    2019-07-08 14:03:45.161 INFO  [271165] [OpticksHub::loadGeometry@543] ]
    2019-07-08 14:03:45.311 INFO  [271165] [nnode::selectSheets@1778] nnode::selectSheets nsa 6 sheetmask 1 ns 1
    2019-07-08 14:03:45.311 INFO  [271165] [BRng::setSeed@45] U setSeed(42)
    2019-07-08 14:03:45.311 INFO  [271165] [BRng::setSeed@45] V setSeed(43)
    2019-07-08 14:03:48.648 ERROR [271165] [BFile::ResolveKey@177] replacing allowed envvar token TMP with default value /tmp/blyth/opticks as envvar not defined 
    2019-07-08 14:03:48.880 INFO  [271165] [CRandomEngine::initCurand@103] 100000,16,16 curand_ni 100000 curand_nv 256
    2019-07-08 14:03:48.881 INFO  [271165] [CRandomEngine::dumpDouble@86] v0
    0.7402193546 0.4384511411 0.5170126557 0.1569886208 
    0.0713675097 0.4625083804 0.2276432663 0.3293584883 
    0.1440653056 0.1877991110 0.9153834581 0.5401248336 
    0.9746608734 0.5474692583 0.6531602740 0.2302378118 
    2019-07-08 14:03:48.881 INFO  [271165] [CRandomEngine::dumpDouble@86] v1
    0.9209938049 0.4603644311 0.3334640563 0.3725204170 
    0.4896024764 0.5672709346 0.0799058080 0.2333681583 
    0.5093778372 0.0889785364 0.0067097610 0.9542270899 
    0.5467113256 0.8245469332 0.5270628929 0.9301316142 
    2019-07-08 14:03:48.881 INFO  [271165] [CRandomEngine::dumpDouble@86] v99999
    0.9140115380 0.4403249323 0.9478355646 0.0900180787 
    0.9587481022 0.9879503846 0.2274523973 0.0438494608 


3M limitation is from the persisted curand init states in the installcache::

    [blyth@localhost RNG]$ pwd
    /home/blyth/local/opticks/installcache/RNG
    [blyth@localhost RNG]$ l
    total 129348
    -rw-rw-r--. 1 blyth blyth    450560 Jul  6  2018 cuRANDWrapper_10240_0_0.bin
    -rw-rw-r--. 1 blyth blyth 132000000 Jul  6  2018 cuRANDWrapper_3000000_0_0.bin
    [blyth@localhost RNG]$ 


There is also be a 100k restriction for aligned running from the precooked randoms created by TRngBufTest, 
which is distinct from the curand states::

    [blyth@localhost opticks]$ l TRngBufTest.npy
    -rw-rw-r--. 1 blyth blyth 204800096 Jul  5 13:03 TRngBufTest.npy
    [blyth@localhost opticks]$ np.py TRngBufTest.npy
    a :                                              TRngBufTest.npy :     (100000, 16, 16) : afb214a9216dbe43c8a3631eefa7dd72 : 20190705-1303 
    [blyth@localhost opticks]$ pwd
    /tmp/blyth/opticks


    [blyth@localhost opticks]$ du -h TRngBufTest.npy
    196M    TRngBufTest.npy

    In [5]: 100000.*16*16*8/1e6    ## 16*16=256 doubles(8 bytes) per photon are precooked 
    Out[5]: 204.8


* file size looking unreasonable if push by factor 10 to get to 1M, are up at 2GB file level.
  And at 100M are at 200GB 

* recall this only needed on CPU, as on GPU just use curand directly, and its only needed
  for aligned running  

* solution is to move to dynamically doing the pre-cooking, so CRandomEngine will 
  need a higher level index to know when its needs to do some precooking : which 
  complicates performance measurement   



Hmm the precooked RNG for aligned running would be better placed in the installcache.

Hmm that begs question what happens between 100k and 3M. 
Is it cycling in ni and well as nv ? Nope, there are asserts.  
Seems have not pushed aligned comparisons beyond 100k yet.

* this cycling can still happen in CAlignEngine (not CRandomEngine)



Only asserts later::

    2019-07-08 14:08:55.886 INFO  [279004] [OScene::init@157] ]
    2019-07-08 14:08:56.727 INFO  [279004] [Opticks::makeEvent@2459]  G4  tagoffset 0 id 0
    2019-07-08 14:08:56.728 INFO  [279004] [Opticks::makeEvent@2459]  OK  tagoffset 0 id 1
    2019-07-08 14:08:56.729 FATAL [279004] [OpticksEvent::resize@1064] OpticksEvent::resize  NOT ENOUGH RNG  num_photons 3000001 rng_max 3000000
    OKG4Test: /home/blyth/opticks/optickscore/OpticksEvent.cc:1068: void OpticksEvent::resize(): Assertion `enoughRng && " need to prepare and persist more RNG states up to maximual per propagation number"' failed.
    
    Program received signal SIGABRT, Aborted.
    0x00007fffe2006207 in raise () from /lib64/libc.so.6
    Missing separate debuginfos, use: debuginfo-install boost-filesystem-1.53.0-27.el7.x86_64 boost-program-options-1.53.0-27.el7.x86_64 boost-regex-1.53.0-27.el7.x86_64 boost-system-1.53.0-27.el7.x86_64 expat-2.1.0-10.el7_3.x86_64 glfw-3.2.1-2.el7.x86_64 glibc-2.17-260.el7_6.3.x86_64 keyutils-libs-1.5.8-3.el7.x86_64 krb5-libs-1.15.1-37.el7_6.x86_64 libX11-1.6.5-2.el7.x86_64 libX11-devel-1.6.5-2.el7.x86_64 libXau-1.0.8-2.1.el7.x86_64 libXcursor-1.1.15-1.el7.x86_64 libXext-1.3.3-3.el7.x86_64 libXfixes-5.0.3-1.el7.x86_64 libXinerama-1.1.3-2.1.el7.x86_64 libXrandr-1.5.1-2.el7.x86_64 libXrender-0.9.10-1.el7.x86_64 libXxf86vm-1.1.4-1.el7.x86_64 libcom_err-1.42.9-13.el7.x86_64 libdrm-2.4.91-3.el7.x86_64 libgcc-4.8.5-36.el7_6.1.x86_64 libglvnd-1.0.1-0.8.git5baa1e5.el7.x86_64 libglvnd-glx-1.0.1-0.8.git5baa1e5.el7.x86_64 libicu-50.1.2-17.el7.x86_64 libselinux-2.5-14.1.el7.x86_64 libstdc++-4.8.5-36.el7_6.1.x86_64 libxcb-1.13-1.el7.x86_64 openssl-libs-1.0.2k-16.el7_6.1.x86_64 pcre-8.32-17.el7.x86_64 xerces-c-3.1.1-9.el7.x86_64 zlib-1.2.7-18.el7.x86_64
    (gdb) bt
    #0  0x00007fffe2006207 in raise () from /lib64/libc.so.6
    #1  0x00007fffe20078f8 in abort () from /lib64/libc.so.6
    #2  0x00007fffe1fff026 in __assert_fail_base () from /lib64/libc.so.6
    #3  0x00007fffe1fff0d2 in __assert_fail () from /lib64/libc.so.6
    #4  0x00007fffe5617ac4 in OpticksEvent::resize (this=0x80536e0) at /home/blyth/opticks/optickscore/OpticksEvent.cc:1068
    #5  0x00007fffe56134f9 in OpticksEvent::setNumPhotons (this=0x80536e0, num_photons=3000001, resize_=true) at /home/blyth/opticks/optickscore/OpticksEvent.cc:266
    #6  0x00007fffe5618164 in OpticksEvent::setGenstepData (this=0x80536e0, genstep_data=0x61d86c0, progenitor=true) at /home/blyth/opticks/optickscore/OpticksEvent.cc:1146
    #7  0x00007fffe56209be in OpticksRun::setGensteps (this=0x6b7540, gensteps=0x61d86c0) at /home/blyth/opticks/optickscore/OpticksRun.cc:179
    #8  0x00007ffff7bd56e3 in OKG4Mgr::propagate_ (this=0x7fffffffcc00) at /home/blyth/opticks/okg4/OKG4Mgr.cc:172
    #9  0x00007ffff7bd55cf in OKG4Mgr::propagate (this=0x7fffffffcc00) at /home/blyth/opticks/okg4/OKG4Mgr.cc:117
    #10 0x00000000004039a9 in main (argc=34, argv=0x7fffffffcf38) at /home/blyth/opticks/okg4/tests/OKG4Test.cc:9
    (gdb) 


::

     713 opticks-prepare-installcache()
     714 {
     715     local msg="=== $FUNCNAME :"
     716     echo $msg generating RNG seeds into installcache 
     717 
     718     cudarap-
     719     cudarap-prepare-installcache
     720 
     721     OpticksPrepareInstallCache_OKC
     722 }


    557 cudarap-rngmax(){ echo $(( 3*1000*1000 )) ; } # maximal number of photons that can be handled
    558 #cudarap-rngdir(){ echo $(opticks-prefix)/cache/rng  ; }
    559 
    560 cudarap-rngdir(){ echo $(opticks-prefix)/installcache/RNG  ; }
    561 cudarap-prepare-installcache()
    562 {
    563    CUDARAP_RNG_DIR=$(cudarap-rngdir) CUDARAP_RNG_MAX=$(cudarap-rngmax) $(cudarap-ibin)
    564 }


::

    [blyth@localhost RNG]$ du -h cuRANDWrapper_3000000_0_0.bin
    126M    cuRANDWrapper_3000000_0_0.bin




Upping to 10M with no tuning of threads_per_launch, blocks took around 30s
------------------------------------------------------------------------------

::

    [blyth@localhost cudarap]$ cudarap-prepare-installcache 
    2019-07-08 21:02:30.115 INFO  [7777] [main@35]  work 10000000 max_blocks 128 seed 0 offset 0 threads_per_block 256 cachedir /home/blyth/local/opticks/installcache/RNG
     init_rng_wrapper sequence_index   0  thread_offset       0  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     3.2512 ms 
     init_rng_wrapper sequence_index   1  thread_offset   32768  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     1.4705 ms 
     init_rng_wrapper sequence_index   2  thread_offset   65536  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     2.0541 ms 
     init_rng_wrapper sequence_index   3  thread_offset   98304  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     2.5815 ms 
     init_rng_wrapper sequence_index   4  thread_offset  131072  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     2.9399 ms 
     init_rng_wrapper sequence_index   5  thread_offset  163840  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     3.4478 ms 
     init_rng_wrapper sequence_index   6  thread_offset  196608  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     3.9670 ms 
     init_rng_wrapper sequence_index   7  thread_offset  229376  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     4.5076 ms 
     init_rng_wrapper sequence_index   8  thread_offset  262144  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time    61.0181 ms 
     init_rng_wrapper sequence_index   9  thread_offset  294912  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time    61.5414 ms 
     init_rng_wrapper sequence_index  10  thread_offset  327680  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time    62.0380 ms 
     init_rng_wrapper sequence_index  11  thread_offset  360448  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time    62.5264 ms 
     init_rng_wrapper sequence_index  12  thread_offset  393216  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time    63.0651 ms 
     init_rng_wrapper sequence_index  13  thread_offset  425984  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time    63.4890 ms 
     ...
     init_rng_wrapper sequence_index 300  thread_offset 9830400  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   124.5993 ms 
     init_rng_wrapper sequence_index 301  thread_offset 9863168  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   125.1031 ms 
     init_rng_wrapper sequence_index 302  thread_offset 9895936  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   125.5352 ms 
     init_rng_wrapper sequence_index 303  thread_offset 9928704  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   126.0339 ms 
     init_rng_wrapper sequence_index 304  thread_offset 9961472  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   122.8083 ms 
     init_rng_wrapper sequence_index 305  thread_offset 9994240  threads_per_launch   5760 blocks_per_launch     23   threads_per_block    256  kernel_time    33.7541 ms 
    init_rng_wrapper tag init workitems 10000000  threads_per_block   256  max_blocks    128 reverse 0 nlaunch 306 TotalTime 29843.5508 ms 
    2019-07-08 21:03:03.255 INFO  [7777] [cuRANDWrapper::test_rng@237]  tag test_0 items 10000000 imod 100000 test_digest c2a2de538adfc989e122098d2b85751d
    ...
    2019-07-08 21:03:03.747 INFO  [7777] [cuRANDWrapper::test_rng@237]  tag test_4 items 10240 imod 100000 test_digest 4cf2394078b7b8c5125b7e50b51e5dfa
    0.0714 0.4896 0.5206 0.1202 0.5666 0.4028 0.7056 0.1623 0.7804 0.2598 
    cuRANDWrapperTest::main after resize tag init workitems 10000000  threads_per_block   256  max_blocks    128 reverse 0 nlaunch 306 TotalTime 29843.5508 ms 
    cuRANDWrapperTest::main after resize tag test_0 workitems 10000000  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch  10 TotalTime     2.6877 ms 
    cuRANDWrapperTest::main after resize tag test_1 workitems 10000000  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch  10 TotalTime     2.5895 ms 
    cuRANDWrapperTest::main after resize tag test_2 workitems 10000000  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch  10 TotalTime     2.6763 ms 
    cuRANDWrapperTest::main after resize tag test_3 workitems 10000000  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch  10 TotalTime     2.6072 ms 
    cuRANDWrapperTest::main after resize tag test_4 workitems 10000000  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch  10 TotalTime     2.5717 ms 
    cuRANDWrapperTest::main after resize tag test_0 workitems   10240  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime     0.0185 ms 
    cuRANDWrapperTest::main after resize tag test_1 workitems   10240  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime     0.0137 ms 
    cuRANDWrapperTest::main after resize tag test_2 workitems   10240  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime     0.0123 ms 
    cuRANDWrapperTest::main after resize tag test_3 workitems   10240  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime     0.0106 ms 
    cuRANDWrapperTest::main after resize tag test_4 workitems   10240  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime     0.0120 ms 



Using the 10M curandState takes about 3s to load and upload::

    blyth@localhost cudarap]$ cuRANDWrapper=INFO cudarap-prepare-installcache
    PLOG::EnvLevel adjusting loglevel by envvar   key cuRANDWrapper level INFO fallback DEBUG
    2019-07-08 21:18:46.264 INFO  [34196] [main@35]  work 10000000 max_blocks 128 seed 0 offset 0 threads_per_block 256 cachedir /home/blyth/local/opticks/installcache/RNG
    2019-07-08 21:18:46.264 INFO  [34196] [cuRANDWrapper::instanciate@37]  num_items 10000000 cachedir /home/blyth/local/opticks/installcache/RNG
    2019-07-08 21:18:46.264 INFO  [34196] [cuRANDWrapper::instanciate@47]  cache enabled  /home/blyth/local/opticks/installcache/RNG
    2019-07-08 21:18:46.264 INFO  [34196] [cuRANDWrapper::Allocate@365] [
    2019-07-08 21:18:46.709 INFO  [34196] [cuRANDWrapper::Allocate@369] ]
    2019-07-08 21:18:46.710 INFO  [34196] [cuRANDWrapper::InitFromCacheIfPossible@320] 
    2019-07-08 21:18:46.710 INFO  [34196] [cuRANDWrapper::InitFromCacheIfPossible@330]  has-cache -> Load 
    2019-07-08 21:18:46.710 INFO  [34196] [cuRANDWrapper::Load@466] [
    2019-07-08 21:18:49.170 INFO  [34196] [cuRANDWrapper::Load@473]  items 10000000 path /home/blyth/local/opticks/installcache/RNG/cuRANDWrapper_10000000_0_0.bin load_digest 8a52c997355c02258febd79de900699d
    2019-07-08 21:18:49.271 INFO  [34196] [cuRANDWrapper::Load@500] ]



Up the ante to 100M, took 8 minutes or so with 3051 launches::

    [blyth@localhost cudarap]$ cuRANDWrapper=INFO cudarap-prepare-installcache
    PLOG::EnvLevel adjusting loglevel by envvar   key cuRANDWrapper level INFO fallback DEBUG
    2019-07-08 21:23:26.173 INFO  [41441] [main@35]  work 100000000 max_blocks 128 seed 0 offset 0 threads_per_block 256 cachedir /home/blyth/local/opticks/installcache/RNG
    2019-07-08 21:23:26.173 INFO  [41441] [cuRANDWrapper::instanciate@37]  num_items 100000000 cachedir /home/blyth/local/opticks/installcache/RNG
    2019-07-08 21:23:26.174 INFO  [41441] [cuRANDWrapper::instanciate@47]  cache enabled  /home/blyth/local/opticks/installcache/RNG
    2019-07-08 21:23:26.174 INFO  [41441] [cuRANDWrapper::Allocate@365] [
    2019-07-08 21:23:26.541 INFO  [41441] [cuRANDWrapper::Allocate@369] ]
    2019-07-08 21:23:26.541 INFO  [41441] [cuRANDWrapper::InitFromCacheIfPossible@320] 
    2019-07-08 21:23:26.541 INFO  [41441] [cuRANDWrapper::InitFromCacheIfPossible@335]  no-cache -> Init+Save 
    2019-07-08 21:23:26.541 INFO  [41441] [cuRANDWrapper::Init@404] [
     init_rng_wrapper sequence_index   0  thread_offset       0  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     3.3219 ms 
     init_rng_wrapper sequence_index   1  thread_offset   32768  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     1.4561 ms 
     init_rng_wrapper sequence_index   2  thread_offset   65536  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     2.0511 ms 
     init_rng_wrapper sequence_index   3  thread_offset   98304  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     2.5733 ms 
     init_rng_wrapper sequence_index   4  thread_offset  131072  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     2.9123 ms 
     init_rng_wrapper sequence_index   5  thread_offset  163840  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     3.4478 ms 
     init_rng_wrapper sequence_index   6  thread_offset  196608  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     3.9844 ms 
     init_rng_wrapper sequence_index   7  thread_offset  229376  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time     4.5087 ms 
     init_rng_wrapper sequence_index   8  thread_offset  262144  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time    60.8563 ms 
     init_rng_wrapper sequence_index   9  thread_offset  294912  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time    61.3325 ms 
     init_rng_wrapper sequence_index  10  thread_offset  327680  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time    61.9305 ms 
     init_rng_wrapper sequence_index  11  thread_offset  360448  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time    62.3002 ms 
     init_rng_wrapper sequence_index  12  thread_offset  393216  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time    62.8398 ms 
     init_rng_wrapper sequence_index  13  thread_offset  425984  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time    63.2648 ms 
     init_rng_wrapper sequence_index  14  thread_offset  458752  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time    63.7788 ms 
     ...
     init_rng_wrapper sequence_index 3047  thread_offset 99844096  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   189.4175 ms 
     init_rng_wrapper sequence_index 3048  thread_offset 99876864  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   186.1151 ms 
     init_rng_wrapper sequence_index 3049  thread_offset 99909632  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   186.7284 ms 
     init_rng_wrapper sequence_index 3050  thread_offset 99942400  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   187.2415 ms 
     init_rng_wrapper sequence_index 3051  thread_offset 99975168  threads_per_launch  24832 blocks_per_launch     97   threads_per_block    256  kernel_time   145.4244 ms 

    init_rng_wrapper tag init workitems 100000000  threads_per_block   256  max_blocks    128 reverse 0 nlaunch 3052 TotalTime 436924.7188 ms 
    2019-07-08 21:30:43.634 INFO  [41441] [cuRANDWrapper::Init@412] ]
    2019-07-08 21:30:43.634 INFO  [41441] [cuRANDWrapper::Save@428] [
    2019-07-08 21:30:46.240 INFO  [41441] [cuRANDWrapper::Save@437]  items 100000000 path /home/blyth/local/opticks/installcache/RNG/cuRANDWrapper_100000000_0_0.bin save_digest 0f422ebd7e91af1e138fcdd5c50916b9
    2019-07-08 21:30:46.240 INFO  [41441] [cuRANDWrapper::SaveToFile@528]  create directory  path /home/blyth/local/opticks/installcache/RNG/cuRANDWrapper_100000000_0_0.bin cache_dir /home/blyth/local/opticks/installcache/RNG
    2019-07-08 21:31:04.745 INFO  [41441] [cuRANDWrapper::Save@446] ]
      

Attempt to tweak MAX_BLOCKS in cudarap-test-1M shows no significant change in total kernel time
when rearrange to do fewer launches.

Loading+uploading the 100M file (which is 4.1G) takes under 20s::

    [blyth@localhost cudarap]$ cuRANDWrapper=INFO cudarap-prepare-installcache
    PLOG::EnvLevel adjusting loglevel by envvar   key cuRANDWrapper level INFO fallback DEBUG
    2019-07-08 21:34:42.030 INFO  [59110] [main@35]  work 100000000 max_blocks 128 seed 0 offset 0 threads_per_block 256 cachedir /home/blyth/local/opticks/installcache/RNG
    2019-07-08 21:34:42.031 INFO  [59110] [cuRANDWrapper::instanciate@37]  num_items 100000000 cachedir /home/blyth/local/opticks/installcache/RNG
    2019-07-08 21:34:42.032 INFO  [59110] [cuRANDWrapper::instanciate@47]  cache enabled  /home/blyth/local/opticks/installcache/RNG
    2019-07-08 21:34:42.032 INFO  [59110] [cuRANDWrapper::Allocate@365] [
    2019-07-08 21:34:42.462 INFO  [59110] [cuRANDWrapper::Allocate@369] ]
    2019-07-08 21:34:42.462 INFO  [59110] [cuRANDWrapper::InitFromCacheIfPossible@320] 
    2019-07-08 21:34:42.462 INFO  [59110] [cuRANDWrapper::InitFromCacheIfPossible@330]  has-cache -> Load 
    2019-07-08 21:34:42.462 INFO  [59110] [cuRANDWrapper::Load@466] [
    2019-07-08 21:35:00.092 INFO  [59110] [cuRANDWrapper::Load@473]  items 100000000 path /home/blyth/local/opticks/installcache/RNG/cuRANDWrapper_100000000_0_0.bin load_digest 0f422ebd7e91af1e138fcdd5c50916b9
    2019-07-08 21:35:01.112 INFO  [59110] [cuRANDWrapper::Load@500] ]


::

    [blyth@localhost RNG]$ du -h *
    4.1G    cuRANDWrapper_100000000_0_0.bin
    420M    cuRANDWrapper_10000000_0_0.bin
    42M     cuRANDWrapper_1000000_0_0.bin
    126M    cuRANDWrapper_3000000_0_0.bin
    440K    cuRANDWrapper_10240_0_0.bin


Rearranged rngmax option to be in millions, so can easily switch between which curandStates to load with::

   --rngmax 3    # default
   --rngmax 10
   --rngmax 100


Checking the byte digests match
-------------------------------------

DONE : added SDigest::DigestPathInByteRange  so can check the persisted curandStates match appropriately  


::

    [blyth@localhost RNG]$ l
    total 4898892
    -rw-r--r--. 1 blyth blyth       7453 Jul  8 22:34 cuRANDWrapperTest.log
    -rw-rw-r--. 1 blyth blyth   44000000 Jul  8 22:34 cuRANDWrapper_1000000_0_0.bin
    -rw-rw-r--. 1 blyth blyth 4400000000 Jul  8 21:31 cuRANDWrapper_100000000_0_0.bin
    -rw-rw-r--. 1 blyth blyth  440000000 Jul  8 21:03 cuRANDWrapper_10000000_0_0.bin
    -rw-rw-r--. 1 blyth blyth          0 Jul  8 21:01 cuRANDWrapper_0_0_0.bin
    -rw-rw-r--. 1 blyth blyth     450560 Jul  6  2018 cuRANDWrapper_10240_0_0.bin
    -rw-rw-r--. 1 blyth blyth  132000000 Jul  6  2018 cuRANDWrapper_3000000_0_0.bin

    [blyth@localhost RNG]$ SDigestTest cuRANDWrapper_1000000_0_0.bin                 ## digest of 1M file  
    cd6b269c6f64b8e03329e1fc349c21f9
    [blyth@localhost RNG]$ SDigestTest cuRANDWrapper_10000000_0_0.bin 0 44000000     ## 1st 1M portion of 10M file matches 1M file
    2019-07-09 19:52:26.149 INFO  [352212] [test_DigestPathInByteRange@99]  path cuRANDWrapper_10000000_0_0.bin i0 0 i1 44000000
    cd6b269c6f64b8e03329e1fc349c21f9
    [blyth@localhost RNG]$ 
    [blyth@localhost RNG]$ SDigestTest cuRANDWrapper_100000000_0_0.bin 0 44000000    ## 1st 1M portion of 100M file matches 1M file
    2019-07-09 19:52:54.181 INFO  [352937] [test_DigestPathInByteRange@99]  path cuRANDWrapper_100000000_0_0.bin i0 0 i1 44000000
    cd6b269c6f64b8e03329e1fc349c21f9


::

    [blyth@localhost RNG]$ SDigestTest  cuRANDWrapper_10000000_0_0.bin           ## digest of 10M file
    ffe00cfef9d97aeef4c1cf085fd46a6a

    [blyth@localhost RNG]$ SDigestTest cuRANDWrapper_100000000_0_0.bin 0 440000000    ## 1st 10M portion of 100M file matches 10M file digest 
    2019-07-09 19:56:16.421 INFO  [358104] [test_DigestPathInByteRange@99]  path cuRANDWrapper_100000000_0_0.bin i0 0 i1 440000000
    ffe00cfef9d97aeef4c1cf085fd46a6a



deviceQuery, thinking about optimizing kernel launch params 
----------------------------------------------------------------

At 12G and 24G available memory on TITAN V and TITAN RTX there is no probably no issue from the 4.1G::

    [blyth@localhost cuda-10.1]$ cuda-samples-bin-deviceQuery 
    running /home/blyth/local/NVIDIA_CUDA-10.1_Samples/bin/x86_64/linux/release/deviceQuery
    /home/blyth/local/NVIDIA_CUDA-10.1_Samples/bin/x86_64/linux/release/deviceQuery Starting...

     CUDA Device Query (Runtime API) version (CUDART static linking)

    Detected 2 CUDA Capable device(s)

    Device 0: "TITAN V"
      CUDA Driver Version / Runtime Version          10.1 / 10.1
      CUDA Capability Major/Minor version number:    7.0
      Total amount of global memory:                 12037 MBytes (12621381632 bytes)
      (80) Multiprocessors, ( 64) CUDA Cores/MP:     5120 CUDA Cores
      GPU Max Clock rate:                            1455 MHz (1.46 GHz)
      Memory Clock rate:                             850 Mhz
      Memory Bus Width:                              3072-bit
      L2 Cache Size:                                 4718592 bytes
      Maximum Texture Dimension Size (x,y,z)         1D=(131072), 2D=(131072, 65536), 3D=(16384, 16384, 16384)
      Maximum Layered 1D Texture Size, (num) layers  1D=(32768), 2048 layers
      Maximum Layered 2D Texture Size, (num) layers  2D=(32768, 32768), 2048 layers
      Total amount of constant memory:               65536 bytes
      Total amount of shared memory per block:       49152 bytes
      Total number of registers available per block: 65536
      Warp size:                                     32
      Maximum number of threads per multiprocessor:  2048
      Maximum number of threads per block:           1024
      Max dimension size of a thread block (x,y,z): (1024, 1024, 64)
      Max dimension size of a grid size    (x,y,z): (2147483647, 65535, 65535)
      Maximum memory pitch:                          2147483647 bytes
      Texture alignment:                             512 bytes
      Concurrent copy and kernel execution:          Yes with 7 copy engine(s)
      Run time limit on kernels:                     No
      Integrated GPU sharing Host Memory:            No
      Support host page-locked memory mapping:       Yes
      Alignment requirement for Surfaces:            Yes
      Device has ECC support:                        Disabled
      Device supports Unified Addressing (UVA):      Yes
      Device supports Compute Preemption:            Yes
      Supports Cooperative Kernel Launch:            Yes
      Supports MultiDevice Co-op Kernel Launch:      Yes
      Device PCI Domain ID / Bus ID / location ID:   0 / 166 / 0
      Compute Mode:
         < Default (multiple host threads can use ::cudaSetDevice() with device simultaneously) >

    Device 1: "TITAN RTX"
      CUDA Driver Version / Runtime Version          10.1 / 10.1
      CUDA Capability Major/Minor version number:    7.5
      Total amount of global memory:                 24190 MBytes (25364987904 bytes)
      (72) Multiprocessors, ( 64) CUDA Cores/MP:     4608 CUDA Cores
      GPU Max Clock rate:                            1770 MHz (1.77 GHz)
      Memory Clock rate:                             7001 Mhz
      Memory Bus Width:                              384-bit
      L2 Cache Size:                                 6291456 bytes
      Maximum Texture Dimension Size (x,y,z)         1D=(131072), 2D=(131072, 65536), 3D=(16384, 16384, 16384)
      Maximum Layered 1D Texture Size, (num) layers  1D=(32768), 2048 layers
      Maximum Layered 2D Texture Size, (num) layers  2D=(32768, 32768), 2048 layers
      Total amount of constant memory:               65536 bytes
      Total amount of shared memory per block:       49152 bytes
      Total number of registers available per block: 65536
      Warp size:                                     32
      Maximum number of threads per multiprocessor:  1024
      Maximum number of threads per block:           1024
      Max dimension size of a thread block (x,y,z): (1024, 1024, 64)
      Max dimension size of a grid size    (x,y,z): (2147483647, 65535, 65535)
      Maximum memory pitch:                          2147483647 bytes
      Texture alignment:                             512 bytes
      Concurrent copy and kernel execution:          Yes with 3 copy engine(s)
      Run time limit on kernels:                     Yes
      Integrated GPU sharing Host Memory:            No
      Support host page-locked memory mapping:       Yes
      Alignment requirement for Surfaces:            Yes
      Device has ECC support:                        Disabled
      Device supports Unified Addressing (UVA):      Yes
      Device supports Compute Preemption:            Yes
      Supports Cooperative Kernel Launch:            Yes
      Supports MultiDevice Co-op Kernel Launch:      Yes
      Device PCI Domain ID / Bus ID / location ID:   0 / 115 / 0
      Compute Mode:
         < Default (multiple host threads can use ::cudaSetDevice() with device simultaneously) >
    > Peer access from TITAN V (GPU0) -> TITAN RTX (GPU1) : No
    > Peer access from TITAN RTX (GPU1) -> TITAN V (GPU0) : No

    deviceQuery, CUDA Driver = CUDART, CUDA Driver Version = 10.1, CUDA Runtime Version = 10.1, NumDevs = 2
    Result = PASS



Add IDBASE offset to TRngBuf 
------------------------------

::

    [blyth@localhost tests]$ TRngBufTest 
    2019-07-09 11:23:57.512 INFO  [442589] [main@23] TRngBufTest
    TRngBuf::generate ibase 0 ni 100000 id_max 1000
    TRngBuf::generate seq 0 id_offset          0 id_per_gen       1000 remaining     100000
    TRngBuf::generate seq 1 id_offset       1000 id_per_gen       1000 remaining      99000
    TRngBuf::generate seq 2 id_offset       2000 id_per_gen       1000 remaining      98000
    ...
    2019-07-09 11:23:58.225 INFO  [442589] [main@45]  save $TMP/TRngBufTest_0.npy
    2019-07-09 11:23:58.226 ERROR [442589] [BFile::ResolveKey@177] replacing allowed envvar token TMP with default value /tmp/blyth/opticks as envvar not defined 
    2019-07-09 11:23:58.598 ERROR [442589] [BFile::ResolveKey@177] replacing allowed envvar token TMP with default value /tmp/blyth/opticks as envvar not defined 
    (100000, 16, 16)
    [[[0.74021935 0.43845114 0.51701266 ... 0.54746926 0.65316027 0.23023781]
      [0.33885619 0.76138884 0.54568148 ... 0.85521436 0.48867753 0.18854636]
      [0.50652462 0.02055138 0.95822281 ... 0.74793386 0.48760796 0.31805685]
      ...
      [0.15299392 0.32710499 0.89352018 ... 0.93996674 0.9458555  0.19730906]
      [0.85649884 0.65747958 0.06287431 ... 0.62356168 0.96832794 0.5317995 ]
      [0.90195084 0.42885613 0.67444962 ... 0.59804755 0.8195923  0.14472319]]

     [[0.9209938  0.46036443 0.33346406 ... 0.82454693 0.52706289 0.93013161]
      [0.16302098 0.78515881 0.94194758 ... 0.49194995 0.54269171 0.93439281]
      [0.47857913 0.44942591 0.12570204 ... 0.04226144 0.37903434 0.71457326]
      ...
      [0.86578399 0.5101068  0.15340619 ... 0.50591779 0.04876163 0.19413081]
      [0.48308805 0.90896726 0.08700182 ... 0.64719349 0.97155493 0.01153351]
      [0.60377067 0.73156077 0.71719307 ... 0.90172863 0.4543947  0.58696574]]

     [[0.03902049 0.25021473 0.18448432 ... 0.21368156 0.34242383 0.22407883]
      [0.52365208 0.92059976 0.99477363 ... 0.52063215 0.8407405  0.78147382]
      [0.80086279 0.32258108 0.69522661 ... 0.59361392 0.01363767 0.67075318]
      ...
      [0.39412248 0.54762876 0.36133623 ... 0.74547082 0.25674096 0.83685589]
      [0.76392841 0.87556869 0.48814872 ... 0.04153943 0.49912784 0.53801984]
      [0.81724483 0.61697578 0.71426886 ... 0.52501678 0.02500784 0.47300982]]

     ...


::

    [blyth@localhost tests]$ TRngBuf_IBASE=1 TRngBufTest 
    2019-07-09 11:26:50.873 INFO  [447084] [main@23] TRngBufTest
    TRngBuf::generate ibase 1 ni 100000 id_max 1000
    TRngBuf::generate seq 0 id_offset          0 id_per_gen       1000 remaining     100000
    TRngBuf::generate seq 1 id_offset       1000 id_per_gen       1000 remaining      99000
    TRngBuf::generate seq 2 id_offset       2000 id_per_gen       1000 remaining      98000
    TRngBuf::generate seq 3 id_offset       3000 id_per_gen       1000 remaining      97000
    ...
    TRngBuf::generate seq 98 id_offset      98000 id_per_gen       1000 remaining       2000
    TRngBuf::generate seq 99 id_offset      99000 id_per_gen       1000 remaining       1000
    2019-07-09 11:26:51.524 INFO  [447084] [main@45]  save $TMP/TRngBufTest_1.npy
    2019-07-09 11:26:51.525 ERROR [447084] [BFile::ResolveKey@177] replacing allowed envvar token TMP with default value /tmp/blyth/opticks as envvar not defined 
    2019-07-09 11:26:51.619 ERROR [447084] [BFile::ResolveKey@177] replacing allowed envvar token TMP with default value /tmp/blyth/opticks as envvar not defined 
    (100000, 16, 16)
    [[[0.9209938  0.46036443 0.33346406 ... 0.82454693 0.52706289 0.93013161]
      [0.16302098 0.78515881 0.94194758 ... 0.49194995 0.54269171 0.93439281]
      [0.47857913 0.44942591 0.12570204 ... 0.04226144 0.37903434 0.71457326]
      ...
      [0.86578399 0.5101068  0.15340619 ... 0.50591779 0.04876163 0.19413081]
      [0.48308805 0.90896726 0.08700182 ... 0.64719349 0.97155493 0.01153351]
      [0.60377067 0.73156077 0.71719307 ... 0.90172863 0.4543947  0.58696574]]

     [[0.03902049 0.25021473 0.18448432 ... 0.21368156 0.34242383 0.22407883]
      [0.52365208 0.92059976 0.99477363 ... 0.52063215 0.8407405  0.78147382]
      [0.80086279 0.32258108 0.69522661 ... 0.59361392 0.01363767 0.67075318]
      ...
      [0.39412248 0.54762876 0.36133623 ... 0.74547082 0.25674096 0.83685589]
      [0.76392841 0.87556869 0.48814872 ... 0.04153943 0.49912784 0.53801984]
      [0.81724483 0.61697578 0.71426886 ... 0.52501678 0.02500784 0.47300982]]




::

    In [1]: a = np.load("TRngBufTest_0.npy")

    In [2]: b = np.load("TRngBufTest_1.npy")

    In [3]: a.shape
    Out[3]: (100000, 16, 16)

    In [4]: b.shape
    Out[4]: (100000, 16, 16)

    In [5]: np.all(a[1] == b[0])
    Out[5]: True

    In [6]: np.all(a[1:] == b[:-1])
    Out[6]: True




Now need higher level interface to get these randoms within CRandomEngine without going via file
----------------------------------------------------------------------------------------------------

::

     43 CRandomEngine::CRandomEngine(CG4* g4)
     44     :
     45     m_g4(g4),
     46     m_ctx(g4->getCtx()),
     47     m_ok(g4->getOpticks()),
     48     m_dbgkludgeflatzero(m_ok->isDbgKludgeFlatZero()),    // --dbgkludgeflatzero
     49     m_run(g4->getRun()),
     50     m_okevt(NULL),
     51     m_okevt_seqhis(0),
     52     m_okevt_pt(NULL),
     53     m_g4evt(NULL),
     54     m_mask(m_ok->getMask()),
     55     m_masked(m_mask.size() > 0),
     56     m_path("$TMP/TRngBufTest.npy"),
     57     m_alignlevel(m_ok->getAlignLevel()),
     58     m_seed(9876),
     59     m_internal(false),
     60     m_skipdupe(true),
     61     m_locseq(m_alignlevel > 1 ? new BLocSeq<unsigned long long>(m_skipdupe) : NULL ),
     62     m_curand(NPY<double>::load(m_path)),
     63     m_curand_index(-1),
     64     m_curand_ni(m_curand ? m_curand->getShape(0) : 0 ),
     65     m_curand_nv(m_curand ? m_curand->getNumValues(1) : 0 ),  // itemvalues
     66     m_current_record_flat_count(0),
     67     m_current_step_flat_count(0),
     68     m_jump(0),
     69     m_jump_count(0),


::

   hg commit -m "TCURAND a high level class for using GPU generated CURAND randoms on the host, enabling GPU and CPU to use the same randoms " 



Added DYNAMIC_CURAND section to CRandomEngine
-----------------------------------------------

::

    TCURAND=ERROR CRandomEngine=ERROR CRandomEngineTest
    TCURAND=ERROR CRandomEngine=ERROR CRandomEngineTest 100000 
    TCURAND=ERROR CRandomEngine=ERROR CRandomEngineTest 99999 100001    # check crossing tranches


::

    hg commit -m "add ThrustRap dependency to CFG4 for TCURAND, use it in CRandomEngine to remove the 100k aligned photon limit within DYNAMIC_CURAND macro"



Succeed to run 3M + 1 photons : but python analysis is taking forever and memory ramping up 5G
---------------------------------------------------------------------------------------------------

::

    ts box --generateoverride 3000001 --rngmax 10


Surely have a whopper memory leak::

    374818 blyth     20   0   45.7g  34.5g 196640 R 100.0 55.1   7:41.51 OKG4Test 


* :doc:`plugging-cfg4-leaks`
* :doc:`py-analysis-too-slow-for-big-events`



