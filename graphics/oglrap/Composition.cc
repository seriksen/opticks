#include "Composition.hh"

#include "Camera.hh"
#include "Trackball.hh"
#include "Common.hh"
#include "View.hh"

#include <glm/glm.hpp>  
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>  
#include <glm/gtc/type_ptr.hpp>

const char* Composition::PRINT = "print" ; 

Composition::Composition()
  :
  m_camera(NULL),
  m_view(NULL),
  m_trackball(NULL),
  m_model_to_world(),
  m_extent(1.0f)
{
    m_camera = new Camera() ;
    m_view   = new View() ;
    m_trackball = new Trackball() ;
}

Composition::~Composition()
{
}


void Composition::configureI(const char* name, std::vector<int> values )
{
    printf("Composition::configureI\n");
    if(values.empty()) return ; 
    if(strcmp(name, PRINT)==0)
    {
        //int print = values.back();
        Summary("Composition::configureI");
    }
}



unsigned int Composition::getWidth()
{
   return m_camera->getWidth();
}
unsigned int Composition::getHeight()
{
   return m_camera->getHeight();
}
void Composition::setSize(unsigned int width, unsigned int height)
{
    m_camera->setSize(width, height);
}

Camera* Composition::getCamera()
{
    return m_camera ;
}
View* Composition::getView()
{
    return m_view ;
}
Trackball* Composition::getTrackball()
{
    return m_trackball ;
}
glm::mat4& Composition::getModelToWorld()
{
    return m_model_to_world ; 
}
float Composition::getExtent()
{
    return m_extent ; 
}
void Composition::setModelToWorld_Extent(float* m2w, float extent)
{
    m_model_to_world = glm::make_mat4(m2w);
    m_extent = extent ; 
    //
    // too small near or to large far leads to flikering mess
    // so initialize to something reasonable based on the 
    // extext of the drawable
    //
    m_camera->setNear( m_extent/10.f ); 
    m_camera->setFar(  m_extent*10.f );  
}




glm::mat4& Composition::getWorld2Eye()  // ModelView
{
     return m_world2eye ;
}
glm::mat4& Composition::getEye2World()  // ModelViewInverse
{
     return m_eye2world ;
}
glm::mat4& Composition::getWorld2Camera()  // just view, no trackballing
{
     return m_world2camera ;
}
glm::mat4& Composition::getCamera2World()  // just view, no trackballing
{
     return m_camera2world ;
}
glm::vec4& Composition::getGaze()
{
     return m_gaze ; 
}
float& Composition::getGazeLength()
{
     return m_gazelength;
}
glm::mat4& Composition::getWorld2Clip()  // ModelViewProjection
{
     return m_world2clip ;
}
glm::mat4& Composition::getProjection()  
{
     return m_projection ;
}
glm::mat4& Composition::getTrackballing()  
{
     return m_trackballing ;
}
glm::mat4& Composition::getITrackballing()  
{
     return m_itrackballing ;
}




void Composition::Details(const char* msg)
{
    update();
    print(m_gaze, "m_gaze = look - eye ");
    print(m_eye2look, "m_eye2look translation");
    print(m_look2eye, "m_look2eye translation");
    print(m_world2eye, "m_world2eye ");
    print(m_eye2world, "m_eye2world ");
    print(m_projection, "m_projection ");
    print(m_world2clip, "m_world2clip");
    print(m_world2camera, "m_world2camera");
    print(m_camera2world, "m_camera2world");
    print(m_trackballing, "m_trackballing");
    print(m_itrackballing, "m_itrackballing");
}



void Composition::update()
{
    //  Update matrices based on 
    //
    //      m_view
    //      m_camera
    //      m_trackball
    //
    // view inputs are in model coordinates (model coordinates are all within -1:1)
    // model_to_world matrix constructed from geometry center and extent
    // is used to construct the lookat matrix 
    //
    //   eye frame
    //       eye  (0,0,0)
    //       look (0,0,-m_gazelength) 
    //

    m_view->getTransforms(m_model_to_world, m_world2camera, m_camera2world, m_gaze );   // model_to_world is input, the others are updated

    m_gazelength = glm::length(m_gaze);

    m_eye2look = glm::translate( glm::mat4(1.), glm::vec3(0,0,m_gazelength));  

    m_look2eye = glm::translate( glm::mat4(1.), glm::vec3(0,0,-m_gazelength));

    m_trackball->getCombinedMatrices(m_trackballing, m_itrackballing);

    m_world2eye = m_look2eye * m_trackballing * m_eye2look * m_world2camera ;           // ModelView

    m_eye2world = m_camera2world * m_look2eye * m_itrackballing * m_eye2look ;          // InverseModelView

    m_projection = m_camera->getProjection();

    m_world2clip = m_projection * m_world2eye ;


/*
  //  env/geant4/geometry/collada/g4daeview/daetransform.py

 51     def _get_world2eye(self):
 52         """
 53         Objects are transformed from **world** space to **eye** space using GL_MODELVIEW matrix, 
 54         as daeviewgl regards model spaces as just input parameter conveniences
 55         that OpenGL never gets to know about those.  
 56 
 57         So need to invert MODELVIEW and apply it to the origin (eye position in eye space)
 58         to get world position of eye.  Can then convert that into model position.  
 59 
 60         Motivation:
 61 
 62            * determine effective view point (eye,look,up) after trackballing around
 ..
 88         return reduce(np.dot, [self.downscale,
 89                                self.trackball.translate,
 90                                self.view.translate_look2eye,   # (0,0,-distance)
 91                                self.trackball.rotation,
 92                                self.view.translate_eye2look,   # (0,0,+distance)
 93                                self.view.world2camera ])
 94     world2eye = property(_get_world2eye)   # this matches GL_MODELVIEW
 ..
 96     def _get_eye2world(self):
 97         return reduce(np.dot, [self.view.camera2world,
 98                                self.view.translate_look2eye,
 99                                self.trackball.rotation.T,
100                                self.view.translate_eye2look,
101                                self.trackball.untranslate,
102                                self.upscale])
103     eye2world = property(_get_eye2world)
*/


 }


void Composition::test_getEyeUVW()
{
    glm::vec3 eye ;
    glm::vec3 U ;
    glm::vec3 V ;
    glm::vec3 W ;

    printf("test_getEyeUVW\n");
    getEyeUVW(eye,U,V,W);

    print(eye, "eye");
    print(U,"U");
    print(V,"V");
    print(W,"W");

    printf("test_getEyeUVW (no trackball)\n");

    getEyeUVW_no_trackball(eye,U,V,W);
    print(eye, "eye");
    print(U,"U");
    print(V,"V");
    print(W,"W");

}


void Composition::getEyeUVW(glm::vec3& eye, glm::vec3& U, glm::vec3& V, glm::vec3& W)
{
   update();

   float tanYfov = m_camera->getTanYfov();
   float aspect = m_camera->getAspect();

   float v_half_height = m_gazelength * tanYfov ; 
   float u_half_width  = v_half_height * aspect ; 
   float w_depth       = m_gazelength ; 

   //  Eye frame axes and origin 
   //  transformed into world frame

   glm::vec4 right( 1., 0., 0., 0.);
   glm::vec4   top( 0., 1., 0., 0.);
   glm::vec4  gaze( 0., 0.,-1., 0.);

   glm::vec4 origin(0., 0., 0., 1.);

   // and scaled to focal plane dimensions 

   U = glm::vec3( m_eye2world * right ) * u_half_width ;  
   V = glm::vec3( m_eye2world * top   ) * v_half_height ;  
   W = glm::vec3( m_eye2world * gaze  ) * w_depth  ;  

   eye = glm::vec3( m_eye2world * origin );   

}

    
void Composition::getEyeUVW_no_trackball(glm::vec3& eye, glm::vec3& U, glm::vec3& V, glm::vec3& W)
{
   glm::vec3 e ;  
   glm::vec3 unorm ;  
   glm::vec3 vnorm ;  
   glm::vec3 gaze ;  

   m_view->getFocalBasis( m_model_to_world, e,unorm,vnorm, gaze );

   float tanYfov = m_camera->getTanYfov();
   float aspect = m_camera->getAspect();
   float v_half_height = glm::length(gaze) * tanYfov ; 
   float u_half_width  = v_half_height * aspect ; 

   eye = e ;
   U = unorm * u_half_width ; 
   V = vnorm * v_half_height ; 
   W = gaze ; 
}


void Composition::Summary(const char* msg)
{
    printf("%s\n", msg);
    m_trackball->Summary("m_trackball");
    m_camera->Summary("m_camera");
    m_view->Summary("m_view");
}



