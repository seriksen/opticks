#pragma once

#include <string>
#include <map>

class BCfg ; 
class Timer ; 

class Opticks ; 
class OpticksGeometry ; 
class OpticksAttrSeq ; 
class OpticksEvent ; 
class GGeo ; 
class Composition ; 
class Bookmarks ; 

class GItemIndex ; 

class NState ; 
class NLookup ; 
class NConfigurable ; 
template <typename> class NPY ;
template <typename> class OpticksCfg ;

#ifdef WITH_NPYSERVER
class numpydelegate ; 
template <typename> class numpyserver ;
#endif

#include "OKGEO_API_EXPORT.hh"

// non-viz, hostside intersection of config, geometry and event
// task: slurp up pieces of App that match the above tagline 

class OKGEO_API OpticksHub {
   public:
       OpticksHub(Opticks* opticks);
       void add(BCfg* cfg);
   public:
       void         configure();
       bool         hasOpt(const char* name);
       bool         isCompute();
#ifdef WITH_NPYSERVER
    private:
       void         configureServer();
#endif
   public:
       OpticksEvent* createG4Event();
       OpticksEvent* createOKEvent();
       OpticksEvent* getG4Event();
       OpticksEvent* getOKEvent();
   private:
       OpticksEvent* createEvent(bool ok);
       OpticksEvent* getEvent();
   public:
       Composition*         getComposition();
       GGeo*                getGGeo();
       Opticks*             getOpticks();
       OpticksCfg<Opticks>* getCfg();
       std::string          getCfgString();
       NState*              getState();
       NLookup*             getLookup();    // material code translation
       Bookmarks*           getBookmarks();
       NPY<unsigned char>*  getColorBuffer();
       Timer*               getTimer(); 

       OpticksAttrSeq*      getFlagNames();
       OpticksAttrSeq*      getMaterialNames();
       OpticksAttrSeq*      getBoundaryNames();
       std::map<unsigned int, std::string> getBoundaryNamesMap();

   public:
       void loadGeometry();
   public:
       NPY<float>* loadGenstep();
       void translateGensteps(NPY<float>* gs);  // into Opticks lingo
       void loadEventBuffers();
       void targetGenstep();
       void configureState(NConfigurable* scene);
       void cleanup();
       void setMaterialMap( std::map<std::string, unsigned>& materialMap, const char* prefix="" );
   private:
       void init();
       void configureCompositionSize();
       void configureLookup();
   private:
       NPY<float>* loadGenstepFile();
       NPY<float>* loadGenstepTorch();
   private:
       Opticks*         m_opticks ; 
       OpticksGeometry* m_geometry ; 
       GGeo*            m_ggeo ;  
       Composition*     m_composition ; 
   private:
       OpticksEvent*    m_evt ;    // points to last evt created, which is either m_g4evt OR m_okevt 
       OpticksEvent*    m_g4evt ; 
       OpticksEvent*    m_okevt ; 

#ifdef WITH_NPYSERVER
       numpydelegate*              m_delegate ; 
       numpyserver<numpydelegate>* m_server ;
#endif
       BCfg*                m_cfg ;
       OpticksCfg<Opticks>* m_fcfg ;   
       NState*              m_state ; 
       NLookup*             m_lookup ; 
       Bookmarks*           m_bookmarks ; 
 
};



