#include "OPTICKS_LOG.hh"
#include "SSys.hh"
#include "SPath.hh"
#include "NP.hh"

#include <cuda_runtime.h>
#include "scuda.h"
#include "squad.h"
#include "QSim.hh"
#include "QEvent.hh"

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

    const char* cfbase =  SPath::Resolve(SSys::getenvvar("CFBASE", "$TMP/CSG_GGeo" )) ; 
    NP* icdf = NP::Load(cfbase, "CSGFoundry", "icdf.npy"); 
    NP* bnd = NP::Load(cfbase, "CSGFoundry", "bnd.npy"); 

    if(icdf == nullptr || bnd == nullptr)
    {
        LOG(fatal) 
            << " MISSING QSim CSGFoundry input arrays "
            << " cfbase " << cfbase 
            << " icdf " << icdf 
            << " bnd " << bnd 
            << " (recreate these with : \"c ; om ; cg ; om ; ./run.sh \" ) "
            ;
        return 1 ; 
    }

    QSim<float>::UploadComponents(icdf, bnd); 
    QSim<float> qs ; 

    QEvent qe ; 

    std::vector<int> photon_counts_per_genstep = { 3, 5, 2, 0, 1, 3, 4, 2, 4 };  
    unsigned x_total = 0 ; 
    for(unsigned i=0 ; i < photon_counts_per_genstep.size() ; i++) x_total += photon_counts_per_genstep[i] ; 

    qe.setGenstepsFake(photon_counts_per_genstep); 
    assert( qe.getNumPhotons() == x_total ); 

    LOG(info) << qe.desc() ; 

    qe.checkEvt(); 

    qs.generate_photon(&qe); 

    std::vector<quad4> photon ; 
    qe.downloadPhoton(photon); 
    LOG(info) << " downloadPhoton photon.size " << photon.size() ; 

    qs.dump_photon( photon.data(), photon.size(), "i0,i1,i2,i3" );  

    cudaDeviceSynchronize(); 

    return 0 ; 
}
