#pragma once

#include <map>
#include <string>
#include <vector>

#include "NPY_API_EXPORT.hh"
#include "NPY_HEAD.hh"

/**
NParameters
==============

Simple (key,value) parameter collection and persisting based on brap/BList. 
Underlying storage of everything as strings which are 
lexically cast/converted at input/output.

See also NMeta based on nlohmann json for 
more complicated storage of objects.

**/

class NPY_API NParameters {
   public:
       static NParameters* Load(const char* path);   // returns NULL for non-existing
       static NParameters* Load(const char* dir, const char* name);
   public:
       typedef std::pair<std::string, std::string>   SS ; 
       typedef std::vector<SS>                      VSS ; 
       typedef std::vector<std::string>              VS ; 
   public:
       NParameters();
      const std::vector<std::pair<std::string,std::string> >& getVec() ;

       std::string getStringValue(const char* name) const ;
   public:
       void append(NParameters* other);
   public:

       template <typename T> 
       void add(const char* name, T value);

       template <typename T> 
       void set(const char* name, T value);

       template <typename T> 
       void append(const char* name, T value, const char* delim=" ");

       template <typename T> 
       T get(const char* name) const ;

       template <typename T> 
       T get(const char* name, const char* fallback) const ;

       template <typename T> 
       T get_fallback(const char* fallback) const ;

   public:
       unsigned getNumItems();
       void dump();
       void dump(const char* msg);
       std::string desc();
       void prepLines();
       std::vector<std::string>& getLines();
   public:
       void save(const char* path);
       void save(const char* dir, const char* name);
   public:
       bool load_(const char* path);
       bool load_(const char* dir, const char* name);
   private:
       VSS m_parameters ; 
       VS  m_lines ;  

};

#include "NPY_TAIL.hh"


