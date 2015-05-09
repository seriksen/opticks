#pragma once

#include <string>
#include <boost/property_tree/ptree.hpp>

#include "GPropertyMap.hh"


class GSubstanceLibMetadata {
  public:
      static const char* filename ; 
      GSubstanceLibMetadata();

  public:
      static GSubstanceLibMetadata* load(const char* dir);
      void save(const char* dir);
      void Summary(const char* msg);
      void createMaterialMap();

   public:
      std::string getSubstanceQty(unsigned int isub, const char* cat, const char* key);
      std::string getSubstanceQtyByIndex(unsigned int isub, unsigned int icat, const char* tag);

   public:
      std::string get(const char* kfmt, const char* idx);
      std::string get(const char* kfmt, unsigned int idx);
      void add(const char* kfmt, unsigned int isub, const char* cat, GPropertyMap<float>* pmap );
      void addDigest(const char* kfmt, unsigned int isub, const char* cat, char* dig );
      void addMaterial(unsigned int isub, const char* cat, char* shortname, char* digest );
      void addMapEntry(unsigned int line, const char* shortname);

  private:
      void read(const char* path);
      void add(const char* kfmt, unsigned int isub, const char* cat, const char* tag, const char* val);

  private:
      boost::property_tree::ptree   m_tree;

};

