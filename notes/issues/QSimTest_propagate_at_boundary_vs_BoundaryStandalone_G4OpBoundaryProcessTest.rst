QSimTest_propagate_at_boundary_vs_BoundaryStandalone_G4OpBoundaryProcessTest
===============================================================================


How to do random aligned propagate_at_boundary comparison between QSim and Geant4
------------------------------------------------------------------------------------

::

    qu tests
         ## OR: cd ~/opticks/qudarap/tests      

    om  
        ## update build

    TEST=rng_sequence NUM=1000000 ./QSimTest.sh 
        ##  generate pre-cooked randoms so Geant4 can consume the curand generated random stream 




    TEST=hemisphere_s_polarized NUM=1000000 ./QSimTest.sh 
        ## generate a sample of 1M S-polarized photons all incident at origin  
        ## this sample of photons is used by both QSimTest.sh below and G4OpBoundaryProcessTest.sh

    TEST=hemisphere_p_polarized NUM=1000000 ./QSimTest.sh 
        ## generate a sample of 1M P-polarized photons all incident at origin  
        ## this sample of photons is used by both QSimTest.sh below and G4OpBoundaryProcessTest.sh



    TEST=propagate_at_boundary_s_polarized NUM=1000000 ./QSimTest.sh 
        ## mutate the hemisphere_s_polarized photons doing boundary reflect or transmit   

    TEST=propagate_at_boundary_p_polarized NUM=1000000 ./QSimTest.sh 
        ## mutate the hemisphere_p_polarized photons doing boundary reflect or transmit   



    bst 
        ## OR : cd ~/opticks/examples/Geant4/BoundaryStandalone 

    vi G4OpBoundaryProcessTest.sh
        ## check the envvars which control where the input pre-cooked randoms and photons will come from 

    ./G4OpBoundaryProcessTest.sh
        ## compiles and runs loading the photons from eg hemisphere_s_polarized and bouncing them 


Random Aligned comparison::

    epsilon:BoundaryStandalone blyth$ ./G4OpBoundaryProcessTest_cf_QSimTest.sh
    a.shape (1000000, 4, 4) : /tmp/blyth/opticks/QSimTest/propagate_at_boundary_s_polarized/p.npy  
    b.shape (1000000, 4, 4) : /tmp/G4OpBoundaryProcessTest/p.npy  
     a_flag (array([1024, 2048], dtype=uint32), array([ 45024, 954976])) 
     b_flag (array([1024, 2048], dtype=uint32), array([ 45025, 954975])) 
    a_TransCoeff [0.784 0.799 0.588 ... 0.853 0.481 0.959] 
    b_TransCoeff [0.784 0.799 0.588 ... 0.853 0.481 0.959] 
     a_flat [0.438 0.46  0.25  ... 0.557 0.184 0.992] 
     b_flat [0.438 0.46  0.25  ... 0.557 0.184 0.992] 

    In [1]:                                           

Once flag discrepancy to chase::

    In [2]: np.where( a_flag != b_flag )
    Out[2]: (array([209411]),)

    In [6]: np.where( np.abs(a_TransCoeff-b_TransCoeff) > 1e-6 )
    Out[6]: (array([], dtype=int64),)

    In [5]: np.all( a_flat == b_flat  )   ## the random numbers are input so perfect agreement is expected and is found
    Out[5]: True

    In [14]: np.where( np.abs(a[:,0] - b[:,0]) > 1e-6 )     
    Out[14]: (array([], dtype=int64), array([], dtype=int64))    

    In [15]: np.all( a[:,0] == b[:,0] )   ## this is input mom : so perfect agreement is expected and is found
    Out[15]: True

    In [17]: np.all( a[:,3,:3] == b[:,3,:3] )  ## input pol : so perfect agreement is expected and is found
    Out[17]: True 




    In [13]: np.where( np.abs(a[:,1] - b[:,1]) > 1e-6 )
    Out[13]: (array([209411, 209411, 209411]), array([0, 1, 2]))


    In [18]: np.where( np.abs(a[:,1] - b[:,1]) > 1e-6 )          ## one in a million with different mom  
    Out[18]: (array([209411, 209411, 209411]), array([0, 1, 2]))

    In [2]: np.where( a_flag != b_flag )     ## its the one with discrepant flag 
    Out[2]: (array([209411]),)


The flag jumper is at the TransCoeff cut::

    In [25]: a[209411]                                                                                                                                                                                        
    Out[25]: 
    array([[ -0.136,  -0.264,  -0.955,   0.955],
           [ -0.09 ,  -0.176,  -0.98 ,   0.955],
           [  0.89 ,  -0.456,  -0.   , 500.   ],
           [  0.89 ,  -0.456,   0.   ,   0.   ]], dtype=float32)

    In [26]: b[209411]                                                                                                                                                                                        
    Out[26]: 
    array([[ -0.136,  -0.264,  -0.955,   0.955],
           [ -0.136,  -0.264,   0.955,   0.955],
           [ -0.89 ,   0.456,  -0.   , 500.   ],
           [  0.89 ,  -0.456,   0.   ,   0.   ]], dtype=float32)

    In [14]: a[209411,0,3] > a[209411,1,3]                                                                                                                                                                    
    Out[14]: False

    In [15]: b[209411,0,3] > b[209411,1,3]                                                                                                                                                                    
    Out[15]: True




    In [19]: np.where( np.abs(a[:,2] - b[:,2]) > 1e-6 )   ## 3 with different polarization, 1 is the flag differ one 
    Out[19]: 
    (array([209411, 209411, 251959, 251959, 251959, 317933, 317933, 317933]),
     array([0, 1, 0, 1, 2, 0, 1, 2]))

    In [24]: np.where( np.abs(a[:,2] - b[:,2]) > 1e-1 )  ## difference in pol.x pol.y and it is not small 
    Out[24]: 
    (array([209411, 209411, 251959, 251959, 317933, 317933]),
     array([0, 1, 0, 1, 0, 1]))



The other discrepant two are very nearly at normal incidence and seems to have an x-y flip:: 

    In [16]: a[251959]
    Out[16]: 
    array([[ -0.   ,  -0.001,  -1.   ,   1.   ],
           [ -0.   ,  -0.001,   1.   ,   0.96 ],
           [  0.16 ,   0.987,   0.001, 500.   ],
           [  0.987,  -0.16 ,   0.   ,   0.   ]], dtype=float32)

    In [17]: b[251959]
    Out[17]: 
    array([[ -0.   ,  -0.001,  -1.   ,   1.   ],
           [ -0.   ,  -0.001,   1.   ,   0.96 ],
           [ -0.987,   0.16 ,  -0.   , 500.   ],
           [  0.987,  -0.16 ,   0.   ,   0.   ]], dtype=float32)


::

    2022-03-24 20:39:34.885 INFO  [570874] [QSimTest<float>::photon_launch_mutate@504]  loaded (1000000, 4, 4, ) from src_subfold hemisphere_s_polarized
    //QSim_photon_launch sim 0x703a40a00 photon 0x7042c0000 num_photon 1000000 dbg 0x703a40c00 type 22 name propagate_at_boundary_s_polarized 
    //qsim.propagate_at_boundary id 251959 
    //qsim.propagate_at_boundary surface_normal (    0.0000,     0.0000,     1.0000) 
    //qsim.propagate_at_boundary direction (   -0.0002,    -0.0011,    -1.0000) 
    //qsim.propagate_at_boundary polarization (    0.9871,    -0.1603,     0.0000) 
    //qsim.propagate_at_boundary c1     1.0000 normal_incidence 1 
    //qsim.propagate_at_boundary RR.x     0.0000 A_trans (    0.9871    -0.1603     0.0000 )  RR.y     1.0000  A_paral (    0.1603     0.9871     0.0011 ) 
    //qsim.propagate_at_boundary reflect 1  tir 0 polarization (    0.1603,     0.9871,     0.0011) 

At normal incidence the new polarization comes all from A_paral as RR.x is zero.



::


    G4OpBoundaryProcessTest::init  normal (     0.0000     0.0000     1.0000) n1     1.0000 n2     1.5000
    G4OpBoundaryProcessTest::set_prd_normal OPTICKS_INPUT_PRD  normal (     0.0000     0.0000     1.0000) n1     1.0000 n2     1.5000
    didi idx 251959 Rindex1 1.00000 Rindex2 1.50000
     TransCoeff     0.9600 E1_perp    -1.0000 E1_parl     0.0000 E2_perp    -0.8000 E2_parl     0.0000
     incident ray oblique  E2_parl 0.0000 E2_perp 0.2000 C_parl 0.0000 C_perp 1.0000  NewPolarization ( -0.9871 0.1603 -0.0000)

    G4OpBoundaryProcessTest::init  normal (     0.0000     0.0000     1.0000) n1     1.0000 n2     1.5000
    G4OpBoundaryProcessTest::set_prd_normal OPTICKS_INPUT_PRD  normal (     0.0000     0.0000     1.0000) n1     1.0000 n2     1.5000
    didi idx 251959 Rindex1 1.00000 Rindex2 1.50000
     TransCoeff     0.9600 E1_perp    -1.0000 E1_parl     0.0000 E2_perp    -0.8000 E2_parl     0.0000
     C_parl 0.0000 A_paral ( -0.1603 -0.9871 -0.0011) 
     C_perp 1.0000 A_trans ( -0.9871 0.1603 0.0000) 
     incident ray oblique  E2_parl 0.0000 E2_perp 0.2000  NewPolarization ( -0.9871 0.1603 -0.0000)
    p.shape (1000000, 4, 4) 


Notice sign flip for A_paral and A_trans between G4 and OK that is causing the deviation in polarization at normal incidence::


    1236                        E2_total  = E2_perp*E2_perp + E2_parl*E2_parl;
    1237                        A_paral   = NewMomentum.cross(A_trans);
    1238                        A_paral   = A_paral.unit();
    1239                        E2_abs    = std::sqrt(E2_total);


    0688     const float3 A_trans = normal_incidence ? *polarization : normalize(cross(*direction, surface_normal)) ; //   OLD POLARIZATION AT NORMAL 
    0727     const float3 A_paral = normalize(cross(*direction, A_trans));   ## thIS IS THE NEW DIRECTION 



::

    In [18]: a[317933]
    Out[18]: 
    array([[ -0.   ,  -0.   ,  -1.   ,   1.   ],
           [ -0.   ,  -0.   ,   1.   ,   0.96 ],
           [  0.479,   0.878,   0.   , 500.   ],
           [  0.878,  -0.479,   0.   ,   0.   ]], dtype=float32)

    In [19]: b[317933]
    Out[19]: 
    array([[ -0.   ,  -0.   ,  -1.   ,   1.   ],
           [ -0.   ,  -0.   ,   1.   ,   0.96 ],
           [ -0.878,   0.479,  -0.   , 500.   ],
           [  0.878,  -0.479,   0.   ,   0.   ]], dtype=float32)


* b (G4) at normal incidence the polarization is flipped
* a (OK) at normal incidence x and y get flipped 



That is strange the random number of the two discrepants is very close to 1::

    In [20]: a_flat[251959]
    Out[20]: 0.99999934

    In [21]: b_flat[251959]   ## exactly the same as a_flat as its an input 
    Out[21]: 0.99999934

    In [22]: b_flat[317933]
    Out[22]: 0.9999999

    In [23]: a_flat[317933]   ## again exact match 
    Out[23]: 0.9999999

Bizarre, surely that cannot be a coincidence ? The two near normal incidence discrepants consume a random very close to 1::

    In [25]: np.where( a_flat > 0.999999 )
    Out[25]: (array([251959, 317933]),)



Cross Product Sign Convention
--------------------------------

::

    255 inline double Hep3Vector::dot(const Hep3Vector & p) const {
    256   return dx*p.x() + dy*p.y() + dz*p.z();
    257 }
    258 

    259 inline Hep3Vector Hep3Vector::cross(const Hep3Vector & p) const {
    260   return Hep3Vector(dy*p.z()-p.y()*dz, dz*p.x()-p.z()*dx, dx*p.y()-p.x()*dy);
    261 }

        d.cross(p) 


    0539 /** cross product */
     540 SUTIL_INLINE SUTIL_HOSTDEVICE float3 cross(const float3& a, const float3& b)
     541 {
     542   return make_float3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
     543 }

        cross(d, p) 


       //                  a <-> d
       //                  b <-> p 

         So : OldMomentum.cross(theFacetNormal) 
         us  cross( 




    1152               if (sint1 > 0.0) {
    1153                  A_trans = OldMomentum.cross(theFacetNormal);
    1154                  A_trans = A_trans.unit();
    1155                  E1_perp = OldPolarization * A_trans;
    1156                  E1pp    = E1_perp * A_trans;
    1157                  E1pl    = OldPolarization - E1pp;
    1158                  E1_parl = E1pl.mag();
    1159               }
    1160               else {
    1161                  A_trans  = OldPolarization;
    1162                  // Here we Follow Jackson's conventions and we set the
    1163                  // parallel component = 1 in case of a ray perpendicular
    1164                  // to the surface
    1165                  E1_perp  = 0.0;
    1166                  E1_parl  = 1.0;
    1167               }




Aligning normal incidence
----------------------------

Change normal incidence cut to match Geant4 "sint1==0."::

    -    const bool normal_incidence = fabs(c1) > 0.999999f ; 
    +    //const bool normal_incidence = fabs(c1) > 0.999999f ; 
    +    const bool normal_incidence = fabs(c1) == 1.f ; 


    2022-03-25 09:51:12.182 INFO  [793717] [QSimTest<float>::photon_launch_mutate@504]  loaded (1000000, 4, 4, ) from src_subfold hemisphere_s_polarized
    //QSim_photon_launch sim 0x703a40a00 photon 0x7042c0000 num_photon 1000000 dbg 0x703a40c00 type 22 name propagate_at_boundary_s_polarized 
    //qsim.propagate_at_boundary id 251959 
    //qsim.propagate_at_boundary surface_normal (    0.0000,     0.0000,     1.0000) 
    //qsim.propagate_at_boundary direction (   -0.0002,    -0.0011,    -1.0000) 
    //qsim.propagate_at_boundary polarization (    0.9871,    -0.1603,     0.0000) 
    //qsim.propagate_at_boundary c1     1.0000 normal_incidence 0 
    //qsim.propagate_at_boundary RR.x     1.0000 A_trans (   -0.9871     0.1603     0.0000 )  RR.y     0.0000  A_paral (   -0.1603    -0.9871    -0.0011 ) 
    //qsim.propagate_at_boundary reflect 1  tir 0 polarization (   -0.9871,     0.1603,     0.0000) 
    NP::Write dtype <f4 ni        1 nj  4 nk  4 nl  -1 nm  -1 path /tmp/blyth/opticks/QSimTest/propagate_at_boundary_s_polarized/p0.npy
    NP::Write dtype <f4 ni        1 nj  4 nk  4 nl  -1 nm  -1 path /tmp/blyth/opticks/QSimTest/propagate_at_boundary_s_polarized/prd.npy
    === ./QSimTest.sh : invoking analysis script QSimTest_propagate_at_boundary_x_polarized.py



::

    In [1]: a[251959]                                                                                                                                                                               
    Out[1]: 
    array([[ -0.   ,  -0.001,  -1.   ,   1.   ],
           [ -0.   ,  -0.001,   1.   ,   0.96 ],
           [ -0.987,   0.16 ,   0.   , 500.   ],
           [  0.987,  -0.16 ,   0.   ,   0.   ]], dtype=float32)

    In [2]: b[251959]                                                                                                                                                                               
    Out[2]: 
    array([[ -0.   ,  -0.001,  -1.   ,   1.   ],
           [ -0.   ,  -0.001,   1.   ,   0.96 ],
           [ -0.987,   0.16 ,  -0.   , 500.   ],
           [  0.987,  -0.16 ,   0.   ,   0.   ]], dtype=float32)

    In [3]: np.where( np.abs(a[:,2] - b[:,2]) > 1e-6 )                                                                                                                                              
    Out[3]: (array([209411, 209411]), array([0, 1]))




Now left with the 1 in a million cut edger::

    In [4]: np.where( np.abs(a[:,0] - b[:,0]) > 1e-6 )
    Out[4]: (array([], dtype=int64), array([], dtype=int64))

    In [5]: np.where( np.abs(a[:,1] - b[:,1]) > 1e-6 )
    Out[5]: (array([209411, 209411, 209411]), array([0, 1, 2]))

    In [6]: np.where( np.abs(a[:,2] - b[:,2]) > 1e-6 )
    Out[6]: (array([209411, 209411]), array([0, 1]))

    In [7]: np.where( np.abs(a[:,3] - b[:,3]) > 1e-6 )
    Out[7]: (array([], dtype=int64), array([], dtype=int64))




P-polarized comparison
------------------------

