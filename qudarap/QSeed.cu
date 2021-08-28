#include <stdio.h>

#include "scuda.h"

#include "QBuf.hh"
#include "iexpand.h"
#include "strided_range.h"
#include <thrust/device_vector.h>

/**
QSeed_create_photon_seeds
---------------------------

See thrustrap/tests/iexpand_stridedTest.cu for the lead up to this

**/

extern QBuf<int>* QSeed_create_photon_seeds(QBuf<float>* gs)
{
    printf("//QSeed_create_photon_seeds \n");      

    typedef typename thrust::device_vector<int>::iterator Iterator;

    thrust::device_ptr<int> pgs = thrust::device_pointer_cast( (int*)gs->ptr ) ; 

    unsigned itemsize = 6*4 ; 

    strided_range<Iterator> np( pgs + 3, pgs + gs->num_items, itemsize );    // begin, end, stride 

    int num_seeds = thrust::reduce(np.begin(), np.end() );

    QBuf<int>* dseed = nullptr ; 

    if( num_seeds > 0 )
    {
        dseed = QBuf<int>::Alloc(num_seeds); 

        thrust::device_ptr<int> pseed = thrust::device_pointer_cast((int*)dseed->ptr) ; 

        iexpand(np.begin(), np.end(), pseed, pseed + dseed->num_items );  
    }
    return dseed ; 
} 
