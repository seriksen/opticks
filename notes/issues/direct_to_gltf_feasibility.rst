Direct to GLTF feasibility
===========================

Basic Tech
------------

GLTF Learning
~~~~~~~~~~~~~~

* https://github.com/KhronosGroup/glTF

* DONE : review gltf standard regarding binary data description, check how
  oyoctogl handles this, see if can integrate NPY buffers in a compliant way 

  * created YoctoGLRap to learn how to use ygltf with NPY buffers, the small 
    demo gltf files created can be loaded by GLTF viewers  

GLTF Questions 
~~~~~~~~~~~~~~~~

* binary data in extras/extensions, how to do that with ygltf ?
* extras vs extensions whats the difference ?

* what package (ie which dependencies to base on) further investigations

X4 : ExtG4 package
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

ExtG4 
   new package bringing together G4 and lower level Opticks 
   (SysRap, NPY, GGeo) : using lots of static enhancers to 
   single Geant4 classes  
 
   Subsequently find that use of GGeo is very natural here, as 
   that has persisting already implemented  

extg4/X4PhysicalVolume
   converts a G4 volume tree directly into a sofar sparsely populated GGeo instance

extg4/X4Solid

   1. DONE: polygonize G4VSolid instances into vtx and tri NPY buffers 
   2. DONE: get YOG to work with X4Solid::convert results , so can save them as GLTF

      * X4 is intended as an operator package to transform G4 volume trees
        into Opticks equivalents described in NPY GGeo YOG language (not in X4)

   3. NOT-NEEDED: add analytic type info regarding the eg G4Sphere into gltf json ?

      * note that the glTF is a side product of the conversion that allows 
        the cache to be visualized by glTF standard renderers, it does not 
        need to contain all the information : just the metadata describing the 
        buffers with the solid data and the node tree and transforms 

      * G4VSolid are converted directly into nnode trees, and GMesh 
        with no intermediary 

   For example to convert a G4VSolid into X4VSolid comprising 
   GMesh and GParts constituents


DONE : Opticks early geocache assumption workaround
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Can workaround okc.Opticks assuming a geocache too early, with 
mimimal code changes if can come up with a geometry identity early, 
and set the IdPath ? Avoiding the untruths.

* done this via BOpticksKey::SetKey from X4PhysicalVolume::X4PhysicalVolume

* as previously considered, a full digest is prohibitive in terms 
  of the code needed : but even a very partial one is liable to 
  avoid 99% of user errors 


How to hookup direct GGeo into higher level Opticks ?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

GGeo normally born into OpticksHub::init/OpticksGeometry::init, so 
how to work with a GGeo converted direct from G4VPhysicalVolume tree ?

Approaches:

1. void* pointing to GGeo in okc.Opticks (void* needed as OKCORE is beneath GGEO)
   which is consulted prior to instanciating a new GGeo in OpticksGeometry::init

2. GGeo::GetGGeo singletonwise which is consulted prior to instanciating 
   a new GGeo in OpticksGeometry::init. PREFER THIS ONE


How to hookup OpticksHub with direct GGeo into g4ok.G4OpticksManager ?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* At G4OpticksManager::BeginOfRunAction/G4OpticksManager::checkGeometry 
  the G4 geometry is closed, use X4PhysicalVolume::Convert
  to directly translate to a GGeo instance 

  * hmm need to define a geometry digest name at this point, to 
    check for preexisting conversion that could load ?

    * see X4PhysicalVolume::Digest 

  * actually the cache is only relevant for large geometries, so could 
    skip cache-check/load/save unless the user opts for it via OPTICKS_ARGS envvar  
    (the embedded commandline)

  * problem is lots of okc.Opticks assumes a cache : so either move the code that does that 
    or just define a digest name for one, even when not using the cache ? 
  

* okop.OpMgr is resident of G4OpticksManager

  * OpMgr/OpticksHub/OpticksGeometry/GGeo instanciation typically happen together,
    just modifying OpticksGeometry to check for a preexisting GGeo singleton
    instance before making a new one maybe all thats needed 
    

GLTF persisting of GGeo ?
~~~~~~~~~~~~~~~~~~~~~~~~~~~

* thats non-sensical : the glTF describes only whats needs to provide 
  a render, it does not need to contain the full information (unlike COLLADA+GDML
  or the Opticks geocache which do need to contain everything).
  Thus there is no need to stuff material/surface optical properties into
  the glTF, they can live quite happily in the geocache as they normally do and
  not be mentioned at all in the glTF json.  This is because the glTF is now
  regarded only as an output, not part of the geometry chain.

* GGeo currently saved in Opticks geocache format (which is .npy and .txt files in 
  a directory structure)

* modifying this to be GLTF compliant is a fairly non-disruptive change : just adding a
  top level .json in gltf dialect and referencing the NPY with the 
  meshes that can be visualized (hmm merged meshes)
  
  * DONE : hmm this means forming the node tree in ygltf form, so it aint simple : but its fairly self contained
  * TODO: complete GGeo by pushing into GMesh/GSolid/GParts?
     
  * actually the hookup of the non-visualized content into extra tags in the GLTF 
    is not really needed 

::

     600 void GGeo::save()
     601 {
     602     LOG(info) << "GGeo::save" ;
     603     m_geolib->dump("GGeo::save.geolib");
     604 
     605     m_geolib->save();   // map of GMergedMesh
     606     m_meshlib->save();  // vector of GMesh 
     607     m_nodelib->save();   // collection of GSolid/GNode instances
     608     m_materiallib->save();
     609     m_surfacelib->save();
     610     m_scintillatorlib->save();
     611     m_sourcelib->save();
     612     m_bndlib->save();
     613 }

::

    epsilon:ggeo blyth$ ll /usr/local/opticks/geocache/DayaBay_VGDX_20140414-1300/g4_00.dae/96ff965744a2f6b78c24e33c80d3a4cd/1/
    total 0
    drwxr-xr-x    3 blyth  staff    96 Apr  4 21:59 ..
    drwxr-xr-x    8 blyth  staff   256 Apr  4 21:59 GMergedMesh
    drwxr-xr-x    4 blyth  staff   128 Apr  4 21:59 MeshIndex
    drwxr-xr-x  251 blyth  staff  8032 Apr  4 21:59 GMeshLib
    drwxr-xr-x    5 blyth  staff   160 Apr  4 21:59 GNodeLib
    drwxr-xr-x    3 blyth  staff    96 Apr  4 21:59 GMaterialLib
    drwxr-xr-x    5 blyth  staff   160 Apr  4 21:59 GSurfaceLib
    drwxr-xr-x    5 blyth  staff   160 Apr  4 21:59 GScintillatorLib
    drwxr-xr-x    3 blyth  staff    96 Apr  4 21:59 GSourceLib
    drwxr-xr-x    6 blyth  staff   192 Apr  4 21:59 GItemList
    drwxr-xr-x    5 blyth  staff   160 Apr  4 22:00 GBndLib
    drwxr-xr-x    2 blyth  staff    64 Apr  5 10:02 MeshIndexAnalytic
    drwxr-xr-x   13 blyth  staff   416 Jun  2 09:54 .
    epsilon:ggeo blyth$ 





Solids/Nodes/Meshes/CSG Trees
---------------------------------

Many G4 solids (depending on parameter values) are represented in Opticks as CSG trees, so 
need to first decide which node class to use for the CSG tree structure.    


YOG/YOGTF
------------

* translation of analytic/sc.py Sc Nd Mh -> YOG::Sc YOG::Nd YOG::Mh 
  which provides intermediary node tree thats easy to convert to gltf with YOG::TF 

* added m_sc node collection to X4PhysicalVolume 


NEXT
--------

* DONE : create GMesh with X4Mesh

* DONE : describe GMesh buffers (just numpy underneath) 
  with GLTF buffers/bufferViews/accessors so standard GLTF renderers can render them

  * hmm using GBuffer, npy not exposed 

* need to hookup X4Solid conversion of G4VSolid into nnode into NCSG, 
  are missing boundary index... need materials first 

* review boundary formation in AssimpGGeo and do equivalent in X4PhysicalVolume

* review the mesh conversion, extras writing done in sc.py 
  and translate across into X4PhysicalVolume with YOG Sc etc..


Describing buffers/bufferView/accessors
-----------------------------------------

Hmm need the equivalent of the below for GBuffer, as GMesh 
still using GBuffer for vertices, indices, normals,

::

     763 template <typename T>
     764 std::size_t NPY<T>::getBufferSize(bool header_only, bool fortran_order) const
     765 {
     766     unsigned int itemcount = getShape(0);    // dimension 0, corresponds to "length/itemcount"
     767     std::string itemshape = getItemShape(1); // shape of dimensions > 0, corresponds to "item"
     768     return aoba::BufferSize<T>(itemcount, itemshape.c_str(), header_only, fortran_order );
     769 }
     770 
     771 template <typename T>
     772 NPYBufferSpec NPY<T>::getBufferSpec() const
     773 {
     774     bool fortran_order = false ;
     775     NPYBufferSpec spec ;
     776     spec.bufferByteLength = getBufferSize(false, fortran_order );
     777     spec.headerByteLength = getBufferSize(true, fortran_order );   // header_only
     778     return spec ;
     779 }

     733 template <typename T>
     734 NPYBufferSpec NPY<T>::saveToBuffer(std::vector<unsigned char>& vdst) const // including the header 
     735 {
     736    // This enables saving NPY arrays into standards compliant gltf buffers
     737    // allowing rendering by GLTF supporting renderers.
     738 
     739     NPYBufferSpec spec = getBufferSpec() ;
     740 
     741     bool fortran_order = false ;
     742 
     743     unsigned int itemcount = getShape(0);    // dimension 0, corresponds to "length/itemcount"
     744 
     745     std::string itemshape = getItemShape(1); // shape of dimensions > 0, corresponds to "item"
     746 
     747     vdst.clear();
     748 
     749     vdst.resize(spec.bufferByteLength);
     750 
     751     char* buffer = reinterpret_cast<char*>(vdst.data() ) ;
     752 
     753     std::size_t num_bytes = aoba::BufferSaveArrayAsNumpy<T>( buffer, fortran_order, itemcount, itemshape.c_str(), (T*)m_data.data() );
     754 
     755     assert( num_bytes == spec.bufferByteLength );
     756 
     757     assert( spec.headerByteLength == 16*5 || spec.headerByteLength == 16*6 ) ;
     758 
     759     return spec ;
     760 }


Review node classes
------------------------

npy.nnode subclasses
~~~~~~~~~~~~~~~~~~~~~~~~

::

    epsilon:npy blyth$ grep :\ nnode *.hpp
    NBox.hpp:struct NPY_API nbox : nnode 
    NCone.hpp:struct NPY_API ncone : nnode 
    NConvexPolyhedron.hpp:struct NPY_API nconvexpolyhedron : nnode 
    NCubic.hpp:struct NPY_API ncubic : nnode 
    NCylinder.hpp:struct NPY_API ncylinder : nnode 
    NDisc.hpp:struct NPY_API ndisc : nnode 
    NHyperboloid.hpp:struct NPY_API nhyperboloid : nnode 
    NNode.hpp:struct NPY_API nunion : nnode {
    NNode.hpp:struct NPY_API nintersection : nnode {
    NNode.hpp:struct NPY_API ndifference : nnode {
    NPlane.hpp:struct NPY_API nplane : nnode 
    NSlab.hpp:struct NPY_API nslab : nnode 
    NSphere.hpp:struct NPY_API nsphere : nnode {
    NTorus.hpp:struct NPY_API ntorus : nnode 
    NZSphere.hpp:struct NPY_API nzsphere : nnode {
    epsilon:npy blyth$ 









Approaches 
------------

Following current AssimpImprter ? NO
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* perhaps replace the AssimpImporter with a G4LiveImporter ? 
  ie that populates GGeo from a live G4 tree rather than 
  the Assimp tree loaded from G4DAE file 

  * this is too tied to current overly organic structure, 
    the aim is to simplify : ending up with significantly less code, 
    not to add more code

Unified geometry handling
~~~~~~~~~~~~~~~~~~~~~~~~~~~~


* ORIGINALLY THOUGHT : aim to replace GGeo/GScene .. ie to unify the analytic 
  and triangulated into a new GLTF based approach 

* but actually GGeo is OK, what I dont like is the historical split into 
  two lines of geometry that come from G4DAE (GGeo) and GDML (GScene)
  and the resulting duplication in the handling of two geometry trees :
  the analytic information that GDML adds (GParts) should live together with 
  the triangulated information (GMesh) in the unified GLTF based geometry 
  using NPY buffers

  * dont like split of analytic args to GGeo constituents : analytic/GDML 
    info lives beside the rest on equal footing

* aim to replace G4DAE+GDML writing, GDML python parsing with sc.py  

* aim to keep NPY and the GPropLibs : so geometry consumers 
  (OptiXRap, OGLRap) can be mostly unchanged  

* BUT what about GScene, analytic geometry hailing 
  from the python GDML parse

* structure of the consumers expects both triangulated and
  analytic in GGeo and GScene (dispensed by OpticksHub)


analytic/gdml.py 
~~~~~~~~~~~~~~~~~~

* converts some parsed raw GDML solid primitives (depending on their parameters, eg rmin) 
  into CSG boolean composities

  * line between solid and composite is not fixed  

  * treating such shapes as composite CSG avoids code duplication (so reduces bug potential)
    as would otherwise require reimplementing the same logic for multiple shapes

  * where is appropriate to do this kind of specialization ? how general to make the GLTF ?
    whichever the choice need to record all the parameters of the solids 


analytic/sc.py 
~~~~~~~~~~~~~~~~~~

* /Volumes/Delta/usr/local/opticks/opticksdata/export/DayaBay_VGDX_20140414-1300/g4_00.gltf

Observations on the GLTF:

* not in geocache, its regarded are source : living in opticksdata


gltf for materials ?
~~~~~~~~~~~~~~~~~~~~~~

* currently no materials in the gltf : that comes the trianulated route 
* need to come up with a structure to live in json extras.

Whats needed::

* material shortname
* list of uri of the properties (directory structure?) 
 
Can refer to the properties NPY using a list of GLTF buffers (
just needs list of uri with bytelengths, offsets).  

Is this needed, as are in extras. Depends on how can get ygltf to 
handle extras and writing binaries ? 

* "save_ygltf" expects memory data buffers std::vector<unsigned char> 
   which it can save, can do that, but maybe no point as will need to 
   implement separate saving and loading of extras 
 

Hmm can use standard buffers for the properties::

    2652 YGLTF_API void save_buffers(const glTF_t* gltf, const std::string& dirname) {
    2653     for (auto& buffer_ : gltf->buffers) {
    2654         auto buffer = &buffer_;
    2655         if (_startsiwith(buffer->uri, "data:"))
    2656             throw gltf_exception("saving of embedded data not supported");
    2657         _save_binfile(dirname + buffer->uri, buffer->data);
    2658     }
    2659 }

Will need to add handling of non existing and intermediate directories. OR just 
using existing persisting capabilities of GMaterial/GPropertyMap. 

Also no need for accessor descriptor machinery : as this data is 
intended for Opticks code (not OpenGL renderers).


/usr/local/opticks-cmake-overhaul/externals/g4dae/g4dae-opticks/src/G4DAEWriteMaterials.cc::

    088 void G4DAEWriteMaterials::MaterialWrite(const G4Material* const materialPtr)
     89 {
     90    const G4String matname = GenerateName(materialPtr->GetName(), materialPtr);
     91    const G4String fxname = GenerateName(materialPtr->GetName() + "_fx_", materialPtr);
     92 
     93    xercesc::DOMElement* materialElement = NewElementOneNCNameAtt("material","id",matname);
     94    xercesc::DOMElement* instanceEffectElement = NewElementOneNCNameAtt("instance_effect","url",fxname, true);
     95    materialElement->appendChild(instanceEffectElement);
     96 
     97    G4MaterialPropertiesTable* ptable = materialPtr->GetMaterialPropertiesTable();
     98    if(ptable)
     99    {
    100        xercesc::DOMElement* extraElement = NewElement("extra");
    101        PropertyWrite(extraElement, ptable);
    102        materialElement->appendChild(extraElement);
    103    }
    104 
    105    materialsElement->appendChild(materialElement);
    106 
    107      // Append the material AFTER all the possible components are appended!
    108 }

/usr/local/opticks-cmake-overhaul/externals/g4dae/g4dae-opticks/src/G4DAEWrite.cc


question : how much processing prior to forming the YGLTF structure ?
------------------------------------------------------------------------

* should GGeo constituent instances eg GMaterial be formed at that juncture or later ? 

GMaterialLib
~~~~~~~~~~~~~~~

* GMaterialLib focusses on the optical properties, should unigeo "G4GLTF" be more general ? 
* eg domain regularization of material/surface properties 


how to do direct shortcutting of material props ?
---------------------------------------------------------

1. devise gltf approach and file layout to hold the props that 
   is close to the geocache layout of GMaterialLib 
   with NPY buffers for binary data 

   * granularity decisions : per-material, per-property ? start with the existing G*Lib decisions

2. translate the COLLADA export in G4DAE to populate in memory gltf tree, from live G4 
   hmm how is binary handled in gltf world ?



reminders GMesh, GMergedMesh when is merge done ?
---------------------------------------------------




geocache description of materials
-------------------------------------

::

    epsilon:1 blyth$ l GMaterialLib/
    total 96
    -rw-r--r--  1 blyth  staff  - 47504 Apr  4 21:59 GMaterialLib.npy

    epsilon:1 blyth$ head -10 GItemList/GMaterialLib.txt 
    GdDopedLS
    LiquidScintillator
    Acrylic
    MineralOil
    Bialkali
    IwsWater
    Water
    DeadWater
    OwsWater
    ESR

    epsilon:1 blyth$ wc -l GItemList/GMaterialLib.txt 
          38 GItemList/GMaterialLib.txt


::

    In [1]: a = np.load("GMaterialLib.npy")

    In [2]: a.shape
    Out[2]: (38, 2, 39, 4)

    In [3]: pwd
    Out[3]: u'/usr/local/opticks/geocache/DayaBay_VGDX_20140414-1300/g4_00.dae/96ff965744a2f6b78c24e33c80d3a4cd/1/GMaterialLib'



GLTF materials : not relevant : will need to use extras
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

     644 struct material_t : glTFChildOfRootProperty_t {
     645     /// The emissive color of the material.
     646     std::array<float, 3> emissiveFactor = {{0, 0, 0}};
     647     /// The emissive map texture.
     648     textureInfo_t emissiveTexture = {};
     649     /// The normal map texture.
     650     material_normalTextureInfo_t normalTexture = {};
     651     /// The occlusion map texture.
     652     material_occlusionTextureInfo_t occlusionTexture = {};
     653     /// A set of parameter values that are used to define the metallic-roughness
     654     /// material model from Physically-Based Rendering (PBR) methodology.
     655     material_pbrMetallicRoughness_t pbrMetallicRoughness = {};
     656 };
     657 



G4DAEWrite::PropertyWrite 
~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    441 void G4DAEWrite::PropertyWrite(xercesc::DOMElement* extraElement,  const G4MaterialPropertiesTable* const ptable)
    442 {
    443    xercesc::DOMElement* propElement;
    444    const std::map< G4String, G4MaterialPropertyVector*,
    445                  std::less<G4String> >* pmap = ptable->GetPropertiesMap();
    446    const std::map< G4String, G4double,
    447                  std::less<G4String> >* cmap = ptable->GetPropertiesCMap();
    448    std::map< G4String, G4MaterialPropertyVector*,
    449                  std::less<G4String> >::const_iterator mpos;
    450    std::map< G4String, G4double,
    451                  std::less<G4String> >::const_iterator cpos;
    452    for (mpos=pmap->begin(); mpos!=pmap->end(); mpos++)
    453    {
    454       propElement = NewElement("property");
    455       propElement->setAttributeNode(NewAttribute("name", mpos->first));
    456       propElement->setAttributeNode(NewAttribute("ref",
    457                                     GenerateName(mpos->first, mpos->second)));
    458       if (mpos->second)
    459       {
    460          PropertyVectorWrite(mpos->first, mpos->second, extraElement);
    461          extraElement->appendChild(propElement);
    462       }
    463       else
    464       {
    465          G4String warn_message = "Null pointer for material property -" + mpos->first ;
    466          G4Exception("G4DAEWrite::PropertyWrite()", "NullPointer",
    467                      JustWarning, warn_message);
    468          continue;
    469       }
    470    }
    471    for (cpos=cmap->begin(); cpos!=cmap->end(); cpos++)
    472    {
    473       propElement = NewElement("property");
    474       propElement->setAttributeNode(NewAttribute("name", cpos->first));
    475       propElement->setAttributeNode(NewAttribute("ref", cpos->first));
    476       xercesc::DOMElement* constElement = NewElement("constant");
    477       constElement->setAttributeNode(NewAttribute("name", cpos->first));
    478       constElement->setAttributeNode(NewAttribute("value", cpos->second));
    479       // tacking onto a separate top level define element for GDML
    480       // but that would need separate access on reading 
    481 
    482       //defineElement->appendChild(constElement);
    483       extraElement->appendChild(constElement);
    484       extraElement->appendChild(propElement);
    485    }
    486 }





Start with something manageable : translating G4 materials to a gltf representation (oyoctogl- structs)
----------------------------------------------------------------------------------------------------------




