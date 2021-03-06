/*
 * Copyright (c) 2019 Opticks Team. All Rights Reserved.
 *
 * This file is part of Opticks
 * (see https://bitbucket.org/simoncblyth/opticks).
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License.  
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */


#include "G4MaterialPropertyVector.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

#include "GProperty.hh"
#include "X4Property.hh"


template <typename T>
G4PhysicsVector* X4Property<T>::Convert(const GProperty<T>* prop) 
{
    size_t nval = prop->getLength();
    G4double* ddom = new G4double[nval] ;
    G4double* dval = new G4double[nval] ;

    for(unsigned j=0 ; j < nval ; j++)
    {   
        T fnm = prop->getDomain(j) ;
        T fval = prop->getValue(j) ; 

        G4double wavelength = G4double(fnm)*nm ; 
        G4double energy = h_Planck*c_light/wavelength ;
        G4double value = G4double(fval) ;

        ddom[nval-1-j] = energy ;   // reverse wavelength order to give increasing energy order
        dval[nval-1-j] = value ;
    }   
    return new G4MaterialPropertyVector( ddom, dval, nval ); 
}



template class X4Property<float>;
template class X4Property<double>;

