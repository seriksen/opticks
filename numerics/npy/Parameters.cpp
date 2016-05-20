#include "Parameters.hpp"

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <iomanip>

#include "jsonutil.hpp"
#include "NLog.hpp"


template <typename T>
void Parameters::add(const char* name, T value)
{
    m_parameters.push_back(SS(name, boost::lexical_cast<std::string>(value) ));
}


std::string Parameters::getStringValue(const char* name)
{
    std::string value ; 
    for(VSS::const_iterator it=m_parameters.begin() ; it != m_parameters.end() ; it++)
    {
        std::string npar  = it->first ; 
        if( strncmp(npar.c_str(), name, strlen(name))==0) value = it->second ; 
    }
    return value ;  
}


template <typename T>
T Parameters::get(const char* name)
{
    std::string value = getStringValue(name);
    if(value.empty())
    {
        LOG(fatal) << "Parameters::get " << name << " EMPTY VALUE "  ;
    }
    return boost::lexical_cast<T>(value);
}


template <typename T>
T Parameters::get(const char* name, const char* fallback)
{
    std::string value = getStringValue(name);
    if(value.empty())
    {
        value = fallback ;  
        LOG(warning) << "Parameters::get " << name << " value empty, using fallback value: " << fallback  ;
    }
    return boost::lexical_cast<T>(value);
}






void Parameters::load_(const char* path)
{
    loadList<std::string, std::string>(m_parameters, path);
}
void Parameters::load_(const char* dir, const char* name)
{
    loadList<std::string, std::string>(m_parameters, dir, name);
}
Parameters* Parameters::load(const char* path)
{
    Parameters* p = new Parameters ;
    p->load_(path); 
    return p ; 
}
Parameters* Parameters::load(const char* dir, const char* name)
{
    Parameters* p = new Parameters ;
    p->load_(dir, name); 
    return p ; 
}



void Parameters::save(const char* path)
{
    saveList<std::string, std::string>(m_parameters, path);
}
void Parameters::save(const char* dir, const char* name)
{
    saveList<std::string, std::string>(m_parameters, dir, name);
}





void Parameters::dump(const char* msg)
{
   prepLines();
   std::cout << msg << std::endl ; 
   for(VS::const_iterator it=m_lines.begin() ; it != m_lines.end() ; it++) std::cout << *it << std::endl ;  
}

void Parameters::prepLines()
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



template void Parameters::add(const char* name, int value);
template void Parameters::add(const char* name, unsigned int value);
template void Parameters::add(const char* name, std::string value);
template void Parameters::add(const char* name, float value);


template int          Parameters::get(const char* name);
template unsigned int Parameters::get(const char* name);
template std::string  Parameters::get(const char* name);
template float        Parameters::get(const char* name);







