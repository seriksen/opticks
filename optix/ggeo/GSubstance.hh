#ifndef GSUBSTANCE_H
#define GSUBSTANCE_H

//
// Excluding "outer" material from GSubstance (more natural and prevents compounding)
// means that will always need to consider pairs of substances, 
// ordered according to light direction.
//
// Which surface is relevant (inner or outer) depends on light direction.  eg think about
// inside of SST tank as opposed to it outside. 
//
//  
//  Questions:
//
//  * how many distinct substances ?
//  * what is substance identity ?
//     
//    * may via a hash of all the properties of the PropertyMaps
// 
//   OptiX has no notion of surface, so using GSubstance to hold 
//   surface and material properties 
//

class GPropertyMap ;

class GSubstance {
  public:
      GSubstance(GPropertyMap* imaterial, GPropertyMap* isurface, GPropertyMap* osurface );
      virtual ~GSubstance();

  public:
      bool matches(GSubstance* other);
      void Summary(const char* msg="GSubstance::Summary");

  public:
      void setInnerMaterial(GPropertyMap* imaterial);
      void setInnerSurface(GPropertyMap* isurface);
      void setOuterSurface(GPropertyMap* osurface);

      GPropertyMap* getInnerMaterial();
      GPropertyMap* getInnerSurface();
      GPropertyMap* getOuterSurface();

  private:
      GPropertyMap*  m_imaterial ; 
      GPropertyMap*  m_isurface ; 
      GPropertyMap*  m_osurface ; 


};

#endif
