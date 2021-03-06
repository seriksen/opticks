cannot-get-device-pointers-from-non-CUDA-interop-buffers
===============================================================

Search
---------

* https://devtalk.nvidia.com/search/more/sitecommentsearch/OptiX+5+OpenGL+interop/2/


interop
--------

* "opticks-find interop" leads to env-;throgl-  but thats Thrust-interop-with-CUDA-and-OpenGL
  and its not very minimal

* creating opticks/examples/UseOptiXProgram as really minimal OptiX starting point for 
  subsequent example exercising minimal OptiX together with minimal OpenGL  



smoking gun : interop change between OptiX 411 and 501/510
------------------------------------------------------------- 

Same problem on macOS(OptiX_501) and Linux(OptiX_510) (linux provides a backtrace).
Rebooting back to Delta with OptiX 411 shows no such problem.

* likely cause : Looks like OpenGL/OptiX interop in 510 has changed cf 411.

::

    void* OBufBase::getDevicePtr()
    {
    //printf("OBufBase::getDevicePtr %s \n", ( m_name ? m_name : "-") ) ;
    //return (void*) m_buffer->getDevicePointer(m_device); 

    CUdeviceptr cu_ptr = (CUdeviceptr)m_buffer->getDevicePointer(m_device) ;
    return (void*)cu_ptr ;
    }


Fail to get the pointer thats used for the streamed indirection. 

::

    393 unsigned OEvent::downloadHits(OpticksEvent* evt)
    394 {
    395     OK_PROFILE("_OEvent::downloadHits");
    396 
    397     NPY<float>* hit = evt->getHitData();
    398 
    399     CBufSpec cpho = m_photon_buf->bufspec();
    400     assert( cpho.size % 4 == 0 );
    401     cpho.size /= 4 ;    //  decrease size by factor of 4, increases cpho "item" from 1*float4 to 4*float4 
    402 
    403     bool verbose = false ;
    404     TBuf tpho("tpho", cpho );
    405     unsigned nhit = tpho.downloadSelection4x4("OEvent::downloadHits", hit, verbose);
    406     // hit buffer (0,4,4) resized to fit downloaded hits (nhit,4,4)
    407     assert(hit->hasShape(nhit,4,4));
    408 
    409     OK_PROFILE("OEvent::downloadHits");
    410 
    411     return nhit ;
    412 }




Reproduce more simply with tboolean-box
-----------------------------------------

::
    
        tboolean-; tboolean-box

        2018-07-15 17:22:31.624 INFO  [4780] [OContext::download@434] OContext::download PROCEED for sequence as OPTIX_NON_INTEROP
        terminate called after throwing an instance of 'optix::Exception'
          what():  Invalid value (Details: Function "RTresult _rtBufferGetDevicePointer(RTbuffer, int, void**)" caught exception: Cannot get device pointers from non-CUDA interop buffers.
        ================================================================================
        Backtrace:
                (0) () +0x711547
                (1) () +0x70dd31
                (2) () +0x1a6816
                (3) rtBufferGetDevicePointer() +0x19e
                (4) OBufBase::getDevicePtr() +0x22
                (5) OBufBase::bufspec() +0x2e
                (6) OEvent::downloadHits(OpticksEvent*) +0x61
                (7) OEvent::download() +0x56
                (8) OpEngine::downloadEvent() +0x1c
                (9) OKPropagator::downloadEvent() +0x40
                (10) OKPropagator::propagate() +0x23c
                (11) OKMgr::propagate() +0x113
                (12) /home/blyth/local/opticks/lib/OKTest() [0x402e06]
                (13) __libc_start_main() +0xf5
                (14) /home/blyth/local/opticks/lib/OKTest() [0x402b89]

        ================================================================================
        )
        /home/blyth/opticks/bin/op.sh: line 845:  4780 Aborted                 (core dumped) /home/blyth/local/opticks/lib/OKTest --size 2560,1440,1 --rendermode +global,+axis --animtimemax 20 --timemax 20 --geocenter --stack 2180 --eye 1,0,0 --dbganalytic --test --testconfig autoseqmap=TO:0,SR:1,SA:0_name=tboolean-box--_outerfirst=1_analytic=1_csgpath=/tmp/blyth/opticks/tboolean-box--_mode=PyCsgInBox_autoobject=Vacuum/perfectSpecularSurface//GlassSchottF2_autoemitconfig=photons:600000,wavelength:380,time:0.2,posdelta:0.1,sheetmask:0x1,umin:0.45,umax:0.55,vmin:0.45,vmax:0.55,diffuse:1,ctmindiffuse:0.5,ctmaxdiffuse:1.0_autocontainer=Rock//perfectAbsorbSurface/Vacuum --torch --torchconfig type=disc_photons=100000_mode=fixpol_polarization=1,1,0_frame=-1_transform=1.000,0.000,0.000,0.000,0.000,1.000,0.000,0.000,0.000,0.000,1.000,0.000,0.000,0.000,0.000,1.000_source=0,0,599_target=0,0,0_time=0.1_radius=300_distance=200_zenithazimuth=0,1,0,1_material=Vacuum_wavelength=500 --torchdbg --tag 1 --cat tboolean-box --anakey tboolean --save
        /home/blyth/opticks/bin/op.sh RC 134
        [blyth@localhost optickscore]$ 
        [blyth@localhost optickscore]$ tboolean-box




OpticksBufferSpec::

     30 Meanings of the settings
     31 ---------------------------
     32 
     33 
     34 OPTIX_NON_INTEROP  
     35     creates OptiX buffer even in INTEROP mode, this is possible for buffers such as "sequence"
     36     which are not used from OpenGL shaders so there is no need for the INTEROP OpenGL buffer
     37     instead can profit from speed and simplicity of pure OptiX buffer
     38 
     39  INTEROP_PTR_FROM_OPENGL  
     40     adopted this with OptiX 4.0, as OpenGL/OptiX/CUDA 3-way interop not working in 400000 
     41     instead moved to 
     42            OpenGL/OptiX : to write the photon data
     43            OpenGL/CUDA  : to index the photons  
     44 
     45 
     46 NB have just moved to splitting the spec by compute and interop
     47    so some simplifications should be possible
     48 



Output from Delta timemachine
-------------------------------


::

    epsilon:issues blyth$ ll /Volumes/Delta/tmp/blyth/opticks/evt/tboolean-box/torch/1/
    total 68096
    -rw-r--r--   1 blyth  wheel       367 Jul 15 17:33 t_delta.ini
    -rw-r--r--   1 blyth  wheel       344 Jul 15 17:33 t_absolute.ini
    -rw-r--r--   1 blyth  wheel   6400080 Jul 15 17:33 so.npy
    -rw-r--r--   1 blyth  wheel  16000096 Jul 15 17:33 rx.npy
    -rw-r--r--   1 blyth  wheel   4000096 Jul 15 17:33 rs.npy
    -rw-r--r--   1 blyth  wheel      2726 Jul 15 17:33 report.txt
    -rw-r--r--   1 blyth  wheel    400080 Jul 15 17:33 ps.npy
    -rw-r--r--   1 blyth  wheel   1600080 Jul 15 17:33 ph.npy
    -rw-r--r--   1 blyth  wheel      2089 Jul 15 17:33 parameters.json
    -rw-r--r--   1 blyth  wheel   6400080 Jul 15 17:33 ox.npy
    -rw-r--r--   1 blyth  wheel        96 Jul 15 17:33 idom.npy
    -rw-r--r--   1 blyth  wheel       176 Jul 15 17:33 gs.npy
    -rw-r--r--   1 blyth  wheel       128 Jul 15 17:33 fdom.npy
    -rw-r--r--   1 blyth  wheel       452 Jul 15 17:33 Material_SequenceSource.json
    -rw-r--r--   1 blyth  wheel       450 Jul 15 17:33 Material_SequenceLocal.json
    -rw-r--r--   1 blyth  wheel       568 Jul 15 17:33 History_SequenceSource.json
    -rw-r--r--   1 blyth  wheel       572 Jul 15 17:33 History_SequenceLocal.json
    -rw-r--r--   1 blyth  wheel        51 Jul 15 17:33 Boundary_IndexSource.json
    -rw-r--r--   1 blyth  wheel        46 Jul 15 17:33 Boundary_IndexLocal.json
    drwxr-xr-x   6 blyth  wheel       204 Jul 15 17:33 20180715_173353
    drwxr-xr-x  22 blyth  wheel       748 Jul 15 17:33 .
    drwxr-xr-x   8 blyth  wheel       272 Jul 15 17:34 ..




