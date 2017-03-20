#include <cstring>
#include "NQuad.hpp"





void nuvec4::dump(const char* msg)
{
    printf("%20s : %10u %10u %10u %10u \n",msg, x,y,z,w ); 
}
void nivec4::dump(const char* msg)
{
    printf("%20s : %10d %10d %10d %10d \n",msg, x,y,z,w ); 
}

void nvec3::dump(const char* msg)
{
    printf("%20s : %10.4f %10.4f %10.4f  \n",msg, x,y,z ); 
}

const char* nvec3::desc()
{
    char _desc[64];
    snprintf(_desc, 64, " (%7.2f %7.2f %7.2f) ", x,y,z );
    return strdup(_desc);
}

const char* nvec4::desc()
{
    char _desc[64];
    snprintf(_desc, 64, " (%7.2f %7.2f %7.2f %7.2f) ", x,y,z,w );
    return strdup(_desc);
}


void nvec4::dump(const char* msg)
{
    printf("%20s : %10.4f %10.4f %10.4f %10.4f \n",msg, x,y,z,w ); 
}
void nquad::dump(const char* msg)
{
    printf("%s\n", msg);
    f.dump("f");
    u.dump("u");
    i.dump("i");
}


