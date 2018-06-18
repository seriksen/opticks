#pragma once


#include "GGEO_API_EXPORT.hh"


template <class T>
class GGEO_API GDomain {
  public: 
     static GDomain<T>* GetDefaultDomain() ; 
  private: 
     static GDomain<T>* fDefaultDomain ; 
  public: 
     GDomain(T low, T high, T step) : m_low(low), m_high(high), m_step(step) {}
     GDomain<T>* makeInterpolationDomain(T step);
     virtual ~GDomain() {}
  public: 
     T getLow(){  return m_low ; }   
     T getHigh(){ return m_high ; }   
     T getStep(){ return m_step ; }
  public: 
     void Summary(const char* msg="GDomain::Summary");
     bool isEqual(GDomain<T>* other); 
     unsigned int getLength();   
     T* getValues();   

  private:
     T m_low ; 
     T m_high ; 
     T m_step ; 
};



