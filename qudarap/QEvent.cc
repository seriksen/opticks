#include <cuda_runtime.h>

#include "SPath.hh"
#include "scuda.h"
#include "squad.h"
#include "NP.hh"
#include "PLOG.hh"

#include "OpticksGenstep.h"

#include "QEvent.hh"
#include "QBuf.hh"
#include "QBuf.hh"
#include "QSeed.hh"
#include "QU.hh"

#include "qevent.h"

template struct QBuf<quad6> ; 

const plog::Severity QEvent::LEVEL = PLOG::EnvLevel("QEvent", "INFO"); 
const QEvent* QEvent::INSTANCE = nullptr ; 
const QEvent* QEvent::Get(){ return INSTANCE ; }


NP* QEvent::MakeFakeGensteps()
{
    std::vector<int> photon_counts_per_genstep = { 3, 5, 2, 0, 1, 3, 4, 2, 4 };
    return MakeFakeGensteps(photon_counts_per_genstep); 
}

NP* QEvent::MakeFakeGensteps(const std::vector<int>& counts)
{
    std::vector<quad6> gs ; 
    unsigned ni = counts.size(); 
    for(unsigned i=0 ; i < ni ; i++)
    {   
        int gencode = OpticksGenstep_TORCH ; 
        quad6 qq ; 
        qq.q0.i.x = gencode  ;   qq.q0.i.y = -1 ;   qq.q0.i.z = -1 ;   qq.q0.i.w = counts[i] ; 
        qq.q1.f.x = 0.f ;  qq.q1.f.y = 0.f ;  qq.q1.f.z = 0.f ;   qq.q1.f.w = 0.f ; 
        qq.q2.i.x = -1 ;   qq.q2.i.y = -1 ;   qq.q2.i.z = -1 ;   qq.q2.i.w = -1 ; 
        qq.q3.i.x = -1 ;   qq.q3.i.y = -1 ;   qq.q3.i.z = -1 ;   qq.q3.i.w = -1 ; 
        qq.q4.i.x = -1 ;   qq.q4.i.y = -1 ;   qq.q4.i.z = -1 ;   qq.q4.i.w = -1 ; 
        qq.q5.i.x = -1 ;   qq.q5.i.y = -1 ;   qq.q5.i.z = -1 ;   qq.q5.i.w = -1 ; 
        gs.push_back(qq); 
    }  

    NP* a = NP::Make<float>( gs.size(), 6, 4 ); 
    a->read2<float>( (float*)gs.data() ); 
    return a ; 
}

QEvent::QEvent()
    :
    evt(new qevent),
    d_evt(QU::device_alloc<qevent>(1)),
    genstep(nullptr),
    seed(nullptr)
{
    INSTANCE = this ; 
}

void QEvent::setGensteps(const NP* gs) 
{ 
    assert( gs->uifc == 'f' && gs->ebyte == 4 ); 
    assert( gs->has_shape(-1, 6, 4) ); 
    unsigned num_gs = gs->shape[0] ; 
    LOG(info) << " num_gs " << num_gs ; 

    QBuf<float>* dgs = QBuf<float>::Upload( gs );   // TODO: this is allocating every time, better to resize to avoid GPU leaking  
    setGensteps(dgs); 
}

void QEvent::setGensteps(QBuf<float>* dgs) // QBuf::ptr references already uploaded gensteps
{
    genstep = dgs ; 
    seed = QSeed::CreatePhotonSeeds(genstep);

    evt->genstep = (quad6*)genstep->ptr ; 
    evt->seed = seed->ptr ; 
    evt->num_photon = seed->num_items ; 
    evt->photon = QU::device_alloc<quad4>(evt->num_photon) ; 

    QU::copy_host_to_device<qevent>(d_evt, evt, 1 );  
}


void QEvent::downloadPhoton( std::vector<quad4>& photon )
{
    photon.resize(evt->num_photon); 
    QU::copy_device_to_host_and_free<quad4>( photon.data(), evt->photon, evt->num_photon ); 
}

void QEvent::savePhoton( const char* dir_, const char* name )
{
    const char* dir = SPath::Resolve(dir_); 
    LOG(info) << dir ; 
    std::vector<quad4> photon ; 
    downloadPhoton(photon); 
    NP::Write( dir, name,  (float*)photon.data(), photon.size(), 4, 4  );
}


std::string QEvent::desc() const
{
    std::stringstream ss ; 
    ss 
        << " genstep " << ( genstep ? genstep->desc() : "-" ) 
        << " seed " << ( seed ? seed->desc() : "-" ) 
        ;
    return ss.str(); 
}


qevent* QEvent::getDevicePtr() const
{
    return d_evt ; 
}
unsigned QEvent::getNumPhotons() const
{
    return evt->num_photon ; 
}


extern "C" void QEvent_checkEvt(dim3 numBlocks, dim3 threadsPerBlock, qevent* evt, unsigned width, unsigned height ) ; 

void QEvent::checkEvt() 
{ 
    unsigned width = getNumPhotons() ; 
    unsigned height = 1 ; 
    LOG(info) << " width " << width << " height " << height ; 

    dim3 numBlocks ; 
    dim3 threadsPerBlock ; 
    QU::ConfigureLaunch( numBlocks, threadsPerBlock, width, height ); 
 
    assert( d_evt ); 
    QEvent_checkEvt(numBlocks, threadsPerBlock, d_evt, width, height );   
}



