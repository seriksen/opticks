/*
 * Copyright (c) 2019 Opticks Team. All Rights Reserved.
 *
 * This file is part of Opticks
 * (see https://bitbucket.org/simoncblyth/opticks).
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License.  
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */

#pragma once


#include <string>
#include <map>
#include <vector>


// npy-
template <typename T> class NPY ; 
class Index ; 
class Types ; 


// okc-
class OpticksColors ; 
class OpticksAttrSeq ; 


// ggeo-
class GColorMap ; 

#include "plog/Severity.h"
#include "NQuad.hpp"

//
// TODO: sort out tension/duplication between GItemIndex and Index
//   NB indices are used in a variety of circumstances, so this aint trivial
//
//   * materials
//   * flags
//   * material sequences
//   * flag sequences
//
//  This got overcomplicated due to the GBoundaryLib attempt 
//  to jump several steps and not have persistable GMaterialLib GSurfaceLib GPropertyLib
// 
//
//  TODO:
//      rejig labelling functionality, remove from here 
//      and use from the OpticksAttrSeq constituent
//      of GMaterialLib GSurfaceLib GBndLib GFlags 
//
//  
//
// adds colors and gui to basis Index constituent 

#include "GGEO_API_EXPORT.hh"
#include "GGEO_HEAD.hh"

class GGEO_API GItemIndex {

        static const plog::Severity LEVEL ; 
   public:
        GItemIndex(Index* index);
        GItemIndex(const char* itemtype, const char* reldir);
        void setTitle(const char* title);
        bool hasIndex() const ; 
   public:
        ///////// AIMING TO KILL ///////////////////////////////////////////////////////////
        typedef std::string (*GItemIndexLabellerPtr)(GItemIndex*, const char*, unsigned int& );
        typedef enum { DEFAULT, COLORKEY, MATERIALSEQ, HISTORYSEQ } Labeller_t ;
   public:
        static std::string defaultLabeller(GItemIndex* self, const char* key, unsigned int& colorcode);
        static std::string colorKeyLabeller(GItemIndex* self, const char* key, unsigned int& colorcode);
        static std::string materialSeqLabeller(GItemIndex* self, const char* key, unsigned int& colorcode);
        static std::string historySeqLabeller(GItemIndex* self, const char* key, unsigned int& colorcode);
   public:
        static const char* ShortenLabel(const char* label, unsigned ntail );
   public:
        std::string gui_radio_select_debug();
        void setLabeller(GItemIndexLabellerPtr labeller);
        void setLabeller(Labeller_t labeller );
        std::string getLabel(const char* key, unsigned int& colorcode);
        //////////////////////////////////////////////////////////////////////////////////////
   public:
        void     setTypes(Types* types);
        Types*   getTypes();
   public:
        // aiming to replace generic Types + messy multiple labellers
        // with specific handler for the type of thing in question 
        // eg material, surface, material sequence, history sequence, history 
        // 
        // all the color stuff belongs in the handler, not here 
        //
        void     setHandler(OpticksAttrSeq* handler);  
        //
   public:
        void add(const char* name, unsigned int source);
   public:
        bool         hasItem(const char* name);
        unsigned int getIndexLocal(const char* name, unsigned int missing=0);
        unsigned int getIndexSource(const char* name, unsigned int missing=0);
        unsigned int getIndexSourceStarting(const char* name, unsigned int missing=0);
        const char*  getNameSource(unsigned int source, const char* missing=NULL) ; // source index 
        const char*  getNameLocal( unsigned int local, const char* missing=NULL) ;  // local index


        unsigned int getNumItems();
        unsigned int getColorCode(const char* key);
        const char*  getColorName(const char* key);
        static nvec3 makeColor( unsigned int rgb );

   public:
        Index*       getIndex();
        int          getSelected();
        const char*  getSelectedKey();
        const char*  getSelectedLabel();
   public:
        void loadIndex(const char* idpath, const char* override=NULL);
   private:
        void init(const char* itemtype, const char* reldir);
   public:
        static GItemIndex* load(const char* idpath, const char* itemtype, const char* reldir);
        void save(const char* idpath);
        void dump(const char* msg="GItemIndex::dump");
        void test(const char* msg="GItemIndex::test", bool verbose=true);

   public:
        // color
        void     setColorSource(OpticksColors* colors);
        void     setColorMap(GColorMap* colormap);

        OpticksColors* getColorSource();
        GColorMap* getColorMap();

        NPY<unsigned char>* makeColorBuffer();
        NPY<unsigned char>* getColorBuffer();

        std::vector<unsigned int>&    getCodes();
        std::vector<std::string>&     getLabels();
        const char* getLabel(unsigned index);
        const char* getShortLabel(unsigned index);
   public:
        void     formTable(bool compact=false);

   private:
        Index*                               m_index ; 
        GItemIndexLabellerPtr                m_labeller ;
   private:
        OpticksColors*                       m_colors ; 
        GColorMap*                           m_colormap ; 
        NPY<unsigned char>*                  m_colorbuffer ; 
        Types*                               m_types ; 
   private:
        OpticksAttrSeq*                            m_handler ; 
   private:
        // populated by formTable
        std::vector<std::string>             m_labels ; 
        std::vector<std::string>             m_labels_short ; 
        std::vector<unsigned int>            m_codes ; 
};

#include "GGEO_TAIL.hh"

