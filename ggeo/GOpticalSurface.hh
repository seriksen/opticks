#pragma once

#include <string>
struct guint4 ; 

#include "GGEO_API_EXPORT.hh"

class GGEO_API GOpticalSurface {
  public:
      static const char* dielectric_dielectric_ ;
      static const char* dielectric_metal_      ;
      static const char* Type(unsigned type);
  public:
      static const char* polished_ ;
      static const char* polishedfrontpainted_ ;
      static const char* polishedbackpainted_  ;
      static const char* ground_ ;
      static const char* groundfrontpainted_ ;
      static const char* groundbackpainted_  ;
      static const char* Finish(unsigned finish);
  public:
      static std::string brief(const guint4& optical); 
  public:
      static GOpticalSurface* create(const char* name, guint4 opt );
      GOpticalSurface(GOpticalSurface* other);
      GOpticalSurface(const char* name, const char* type, const char* model, const char* finish, const char* value);
  private:
      void init();
      void findShortName(char marker='_');
      void checkValue() const ;
  public:
      virtual ~GOpticalSurface();

      guint4 getOptical() const ;
      bool isSpecular() const ;
       
      std::string description() const ;
      void Summary(const char* msg="GOpticalSurface::Summary", unsigned int imod=1) const ;
      const char* digest() const ;

  public:
      const char* getName() const ;
      const char* getType() const ;
      const char* getModel() const ;
      const char* getFinish() const ;
      const char* getValue() const ;
      const char* getShortName() const ;

  private:
      const char* m_name ;  
      const char* m_type ;  
      const char* m_model ;  
      const char* m_finish ;  
      const char* m_value ;  
      const char* m_shortname ;  

};


