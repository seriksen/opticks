#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include "NNodeCollector.hpp"

template <typename T>
NNodeCollector<T>::NNodeCollector( const T* root_)
   :
   root(root_)
{
   collect_preorder_r( root );
   collect_inorder_r( root );
   collect_postorder_r( root );
}

template <typename T>
NNodeCollector<T>::~NNodeCollector()
{
}
 
template <typename T>
void NNodeCollector<T>::Inorder_r( std::vector<T*>& inorder, T* node ) // static
{
    if( node == NULL ) return ; 
    Inorder_r( inorder,  node->left );
    inorder.push_back( node ) ;
    Inorder_r( inorder, node->right );
}


template <typename T>
void NNodeCollector<T>::collect_inorder_r( const T* node ) 
{
    if( node == NULL ) return ; 
    collect_inorder_r( node->left );
    inorder.push_back( node ) ;
    collect_inorder_r( node->right );
}
template <typename T>
void NNodeCollector<T>::collect_postorder_r(  const T* node ) 
{
    if( node == NULL ) return ; 
    collect_postorder_r( node->left );
    collect_postorder_r( node->right );
    postorder.push_back( node ) ;
}
template <typename T>
void NNodeCollector<T>::collect_preorder_r( const  T* node ) 
{
    if( node == NULL ) return ; 
    preorder.push_back( node ) ;
    collect_preorder_r( node->left );
    collect_preorder_r( node->right );
}

template <typename T>
void NNodeCollector<T>::dump(const char* msg, std::vector<const T*>& order ) 
{
    std::cout << msg  ; 
    std::cout << desc(order); 
}

template <typename T>
std::string NNodeCollector<T>::desc(std::vector<const T*>& order ) 
{
    std::stringstream ss ; 
    for(auto n: order) 
    {
        ss << std::setw(10) << n->tag() << " "  ; 
        const char* label = n->label ; 
        ss << label << " " ; 
        //if(strlen(label) > 4) ss << std::endl ; 
        ss << std::endl ; 
    }
    ss << std::endl ; 
    return ss.str();
}

template <typename T> std::string NNodeCollector<T>::desc_inorder() {   return desc(inorder); }
template <typename T> std::string NNodeCollector<T>::desc_preorder() {  return desc(preorder); }
template <typename T> std::string NNodeCollector<T>::desc_postorder() { return desc(postorder); }


template <typename T>
void NNodeCollector<T>::dump(const char* msg ) 
{
    std::cout << msg << std::endl ; 
    dump("preorder  : ", preorder );
    dump("inorder   : ", inorder );
    dump("postorder : ", postorder );
}


#include "No.hpp"
#include "NNode.hpp"

template struct NNodeCollector<no>;
template struct NNodeCollector<nnode>;


