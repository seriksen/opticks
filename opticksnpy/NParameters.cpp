#include "NParameters.hpp"

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <iomanip>

#include "BList.hh"
#include "PLOG.hh"


NParameters::NParameters()
{
}

std::vector<std::string>& NParameters::getLines()
{
    if(m_lines.size() == 0 ) prepLines();
    return m_lines ;
}

const std::vector<std::pair<std::string,std::string> >& NParameters::getVec()
{
    return m_parameters ; 
}


std::string NParameters::getStringValue(const char* name)
{
    std::string value ; 
    for(VSS::const_iterator it=m_parameters.begin() ; it != m_parameters.end() ; it++)
    {
        std::string npar  = it->first ; 
        if( strncmp(npar.c_str(), name, strlen(name))==0) value = it->second ; 
    }
    return value ;  
}




void NParameters::load_(const char* path)
{
    BList<std::string, std::string>::load(&m_parameters, path);
}
void NParameters::load_(const char* dir, const char* name)
{
    BList<std::string, std::string>::load(&m_parameters, dir, name);
}
NParameters* NParameters::load(const char* path)
{
    NParameters* p = new NParameters ;
    p->load_(path); 
    return p ; 
}
NParameters* NParameters::load(const char* dir, const char* name)
{
    NParameters* p = new NParameters ;
    p->load_(dir, name); 
    return p ; 
}



void NParameters::save(const char* path)
{
    BList<std::string, std::string>::save(&m_parameters, path);
}
void NParameters::save(const char* dir, const char* name)
{
    BList<std::string, std::string>::save(&m_parameters, dir, name);
}


void NParameters::dump()
{
    dump("NParameters::dump");  // handy for debugging::   (lldb) expr m_parameters->dump()
}

std::string NParameters::desc()
{
    prepLines();
    std::stringstream ss ; 
    ss << "NParameters numItems " << getNumItems() ; 
    for(VS::const_iterator it=m_lines.begin() ; it != m_lines.end() ; it++) ss << *it << " : " ;  
    return ss.str();
}


void NParameters::dump(const char* msg)
{
   prepLines();
   std::cout << msg << std::endl ; 
   for(VS::const_iterator it=m_lines.begin() ; it != m_lines.end() ; it++) std::cout << *it << std::endl ;  
}

void NParameters::prepLines()
{
    m_lines.clear();
    for(VSS::const_iterator it=m_parameters.begin() ; it != m_parameters.end() ; it++)
    {
        std::string name  = it->first ; 
        std::string value = it->second ; 

        std::stringstream ss ;  
        ss 
             << std::fixed
             << std::setw(15) << name
             << " : " 
             << std::setw(15) << value 
             ;
        
        m_lines.push_back(ss.str());
    }
}



unsigned NParameters::getNumItems()
{
   return m_parameters.size(); 
}


template <typename T>
void NParameters::set(const char* name, T value)
{
    std::string svalue = boost::lexical_cast<std::string>(value) ;
    bool found(false);

    for(VSS::iterator it=m_parameters.begin() ; it != m_parameters.end() ; it++)
    {
        std::string npar  = it->first ; 
        if( strncmp(npar.c_str(), name, strlen(name))==0)
        {
            std::string prior = it->second ; 
            LOG(debug) << "NParameters::set changing "
                      << name 
                      << " from " << prior 
                      << " to " << svalue 
                      ;
                  
            it->second = svalue ; 
            found = true ; 
        }
    }

    if(!found) add<T>(name, value) ;
 
}


template <typename T>
void NParameters::add(const char* name, T value)
{
    m_parameters.push_back(SS(name, boost::lexical_cast<std::string>(value) ));
}



void NParameters::append(NParameters* other)
{
    if(!other) return ; 

    const VSS& other_v = other->getVec();
    for(VSS::const_iterator it=other_v.begin() ; it != other_v.end() ; it++)
    {
        std::string name  = it->first ; 
        std::string value = it->second ; 
        m_parameters.push_back(SS(name, value));
    }
}




template <typename T>
T NParameters::get(const char* name)
{
    std::string value = getStringValue(name);
    if(value.empty())
    {
        LOG(fatal) << "NParameters::get " << name << " EMPTY VALUE "  ;
    }
    return boost::lexical_cast<T>(value);
}


template <typename T>
T NParameters::get(const char* name, const char* fallback)
{
    std::string value = getStringValue(name);
    if(value.empty())
    {
        value = fallback ;  
        LOG(debug) << "NParameters::get " << name << " value empty, using fallback value: " << fallback  ;
    }
    return boost::lexical_cast<T>(value);
}







template NPY_API void NParameters::add(const char* name, bool value);
template NPY_API void NParameters::add(const char* name, int value);
template NPY_API void NParameters::add(const char* name, unsigned int value);
template NPY_API void NParameters::add(const char* name, std::string value);
template NPY_API void NParameters::add(const char* name, float value);
template NPY_API void NParameters::add(const char* name, char value);


template NPY_API void NParameters::set(const char* name, bool value);
template NPY_API void NParameters::set(const char* name, int value);
template NPY_API void NParameters::set(const char* name, unsigned int value);
template NPY_API void NParameters::set(const char* name, std::string value);
template NPY_API void NParameters::set(const char* name, float value);
template NPY_API void NParameters::set(const char* name, char value);


template NPY_API bool         NParameters::get(const char* name);
template NPY_API int          NParameters::get(const char* name);
template NPY_API unsigned int NParameters::get(const char* name);
template NPY_API std::string  NParameters::get(const char* name);
template NPY_API float        NParameters::get(const char* name);
template NPY_API char         NParameters::get(const char* name);


template NPY_API bool         NParameters::get(const char* name, const char* fallback);
template NPY_API int          NParameters::get(const char* name, const char* fallback);
template NPY_API unsigned int NParameters::get(const char* name, const char* fallback);
template NPY_API std::string  NParameters::get(const char* name, const char* fallback);
template NPY_API float        NParameters::get(const char* name, const char* fallback);
template NPY_API char         NParameters::get(const char* name, const char* fallback);






