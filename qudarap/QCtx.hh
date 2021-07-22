#pragma once

#include <string>
#include "QUDARAP_API_EXPORT.hh"
#include "plog/Severity.h"

/**
QCtx
======

TODO: 

1. genstep provisioning 

**/

class GGeo ; 
class GScintillatorLib ; 
class GBndLib ; 

template <typename T> class NPY ; 
template <typename T> struct QTex ; 

struct QRng ; 
struct QScint ;
struct QBnd ; 

struct qctx ; 
struct quad4 ; 
union  quad ; 

struct QUDARAP_API QCtx
{
    static const plog::Severity LEVEL ; 
    static const char* PREFIX ; 
    static const QCtx* INSTANCE ; 
    static const QCtx* Get(); 
    static QScint* MakeScint(const GScintillatorLib* slib);
    static void Init(const GGeo* ggeo); 

    const QRng*    rng ; 
    const QScint*  scint ; 
    const QBnd*    bnd ; 
    qctx*          ctx ;  
    qctx*          d_ctx ;  

    QCtx();
    void init(); 
    char getScintTexFilterMode() const ;

    std::string desc() const ; 

    void configureLaunch( dim3& numBlocks, dim3& threadsPerBlock, unsigned width, unsigned height );
    void rng_sequence_0( float* rs, unsigned num_items );

    template <typename T> void rng_sequence_( dim3 numblocks, dim3 threadsPerBlock, qctx* d_ctx, T* d_seq, unsigned ni_tranche, unsigned nv, unsigned ioffset );
    template <typename T> static char typecode() ; 
    template <typename T> static std::string rng_sequence_reldir(const char* prefix, unsigned ni, unsigned nj, unsigned nk, unsigned ni_tranche_size );
    template <typename T> static std::string rng_sequence_name(const char* prefix, unsigned ni, unsigned nj, unsigned nk, unsigned ioffset );
    template <typename T> void rng_sequence( T* seq, unsigned ni, unsigned nj, unsigned ioffset ); 
    template <typename T> void rng_sequence( const char* dir, unsigned ni, unsigned nj, unsigned nk, unsigned ni_tranche_size );


    void generate_scint(    float* wavelength, unsigned num_wavelength, unsigned& hd_factor ); 
    void generate_cerenkov( float* wavelength, unsigned num_wavelength ); 
    void generate_cerenkov_photon( quad4* photon, unsigned num_photon, int print_id ) ; 


    void dump(              float* wavelength, unsigned num_wavelength, unsigned edgeitems=10 ); 

    void generate( quad4* photon,     unsigned num_photon ); 
    void dump(     quad4* photon,     unsigned num_photon, unsigned egdeitems=10 ); 

    template<typename T> static T* device_alloc( unsigned num_items ) ; 
    template<typename T> static void device_free( T* d ) ; 
    template<typename T> static void copy_device_to_host( T* h, T* d,  unsigned num_items);
    template<typename T> static void copy_device_to_host_and_free( T* h, T* d,  unsigned num_items);
    template<typename T> static void copy_host_to_device( T* d, const T* h,  unsigned num_items);


    unsigned getBoundaryTexWidth() const ;
    unsigned getBoundaryTexHeight() const ;
    const NPY<float>* getBoundaryTexSrc() const ; 

    void boundary_lookup_all(  quad* lookup, unsigned width, unsigned height ) ; 
    void boundary_lookup_line( quad* lookup, float* domain, unsigned num_lookup, unsigned line, unsigned k ) ; 

};
