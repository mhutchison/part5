// File: tree.cxx
// Written by Michael Main (Sep 4, 2005)
// This is the implementation file for an attribute tree class.

#include <cstdlib>       // Provides NULL
#include <iomanip>       // Provides setw
#include <iostream>      // Provides ostream
#include <string>        // Provides string class
#include <map>           // Provides map class
#include <typeinfo>      // Provides type_info and typeid
#include <vector>        // Provides vector class
#include <assert.h>      // Provides assert macro
#include <stdarg.h>      // Provides va_list, va_start, va_arg, va_end
#include "tree.h"        // Provides tree class definition
using namespace std;

namespace colorado
{
    //----------------------------------------------------------------------
    bool tree::new_called = false;
    //----------------------------------------------------------------------

    //----------------------------------------------------------------------
    tree::tree(const std::string& label, size_t n, ...)
    {
	va_list arguments;    // List of the pointers to children
	size_t i;             // Loop control variable 

	// Trees can be created only through a call to new; otherwise the
	// the destructor fails.  Therefore, trees are never local
	// or global variables; they occur only on the heap.
	// When new is called, it sets new_called to true.
	// We check that it is true here, then reset it to false
	// for the next constructor call.
	// Note that this approach is not thread-safe.
	assert(new_called);
	new_called = false;
	
	root_label = label;  // Set the label for the root
	uplink = NULL;       // The root has no parent
	
	// Add the pointers to the children
	va_start(arguments, n); // Start after n
	for(i = 0; i < n; ++i)
	{
	    append_child(va_arg(arguments, tree*));
	}
	va_end(arguments);
    }
    //----------------------------------------------------------------------

    
    //---------------------------------------------------------------------
    tree::tree(const tree& source)
    {
	// Trees can be created only through a call to new; otherwise the
	// the destructor fails.  Therefore, trees are never local
	// or global variables; they occur only on the heap.
	// When new is called, it sets new_called to true.
	// We check that it is true here, then reset it to false
	// for the next constructor call.
	// Note that this approach is not thread-safe.
	assert(new_called);
	new_called = false;

	uplink = NULL; // Since this is a new tree.
	*this = source;
    }
    //---------------------------------------------------------------------


    //---------------------------------------------------------------------
    tree& tree::operator =(const tree& source)
    {
	size_t i;
	map<string, attribute_struct>::const_iterator it;
	attribute_struct a;

	// Check whether this is a self-assignment
	if (this == &source) return *this;

	// Clear away anything in this tree (but leave its parent intact)
	clear( ); 
        
	root_label = source.root_label;            
	for (i = 0; i < source.children.size( ); ++i)
	{
	    append_child(new tree(*(source.children[i])));
	}
	for (it = source.attributes.begin( ); it != source.attributes.end( ); ++it)
	{
	    a = it->second;
	    a.data_ptr = a.copier(a.data_ptr);
	    attributes[it->first] = a;
	}            
	return *this;
    }
    //---------------------------------------------------------------------


    //---------------------------------------------------------------------
    void* tree::operator new(size_t bytes)
    {
	new_called = true;
	return malloc(bytes);
    }
    void tree::operator delete(void* p)
    {
	free(p);
    }
    //---------------------------------------------------------------------
        

    //---------------------------------------------------------------------
    void tree::append_child(tree* p)
    {
	assert(p != NULL);
	assert(p->uplink == NULL);
	children.push_back(p);
	p->uplink = this;
    }
    //---------------------------------------------------------------------


    //---------------------------------------------------------------------
    const std::type_info& tree::attribute_type(const std::string& key) const
    {
	if (attributes.count(key) > 0)
	{
	    // Cannot use attributes[key] on a const map; use find instead.
	    return *(attributes.find(key)->second.data_type);
	}
	else
	    return typeid(void);
    }
    //---------------------------------------------------------------------


    //---------------------------------------------------------------------
    tree* tree::child(size_t n)
    {
	return (n >= children.size( )) ? NULL : children[n];
    }
    const tree* tree::child(size_t n) const
    {
	return (n >= children.size( )) ? NULL : children[n];
    }
    //---------------------------------------------------------------------


    //---------------------------------------------------------------------
    void tree::clear( )
    {
	size_t i;
	map<string, attribute_struct>::iterator it;
	attribute_struct a;
	    
	// Reset the label to the default value
	root_label = "";

	// Clear all the children
	for (i = 0; i < children.size( ); ++i)
	{
	    delete children[i];
	}
	children.resize(0);
	
	// Clear all the attributes
	for (it = attributes.begin( ); it != attributes.end( ); ++it)
	{
	    a = it->second;
	    a.destroyer(a.data_ptr);
	}
	attributes.clear( );
    }
    //---------------------------------------------------------------------


    //---------------------------------------------------------------------
    size_t tree::depth( ) const
    {
	size_t answer = 0;
	size_t many = many_children( );
	size_t i;
        size_t child_depth;
	
	for (i = 0; i < many; ++i)
	{
	    child_depth = child(i)->depth( );
	    if (child_depth >= answer)
		answer = child_depth + 1;
	}
	return answer;
    }
    //---------------------------------------------------------------------

	
    //---------------------------------------------------------------------
    size_t tree::depth_left( ) const
    {
	if (many_children( ) == 0)
	    return 0;
	else
	    return child(0)->depth_left( ) + 1;
    }
    //---------------------------------------------------------------------

    
    //---------------------------------------------------------------------
    size_t tree::depth_right( ) const
    {
	size_t many = many_children( );
	if (many_children( ) == 0)
	    return 0;
	else
	    return child(many-1)->depth_right( ) + 1;
    }
    //---------------------------------------------------------------------

    
    //---------------------------------------------------------------------
    bool tree::erase_attribute(const string& key)
    {
	map<string, attribute_struct>::iterator it;
	attribute_struct a;
	
	it = attributes.find(key);
	if (it == attributes.end( ))
	    return false;
	a = it->second;	    
	a.destroyer(a.data_ptr);
	attributes.erase(it);
	return true;
    }
    //---------------------------------------------------------------------


    //---------------------------------------------------------------------
    bool tree::erase_child(size_t n)
    {
	if (n < children.size( ))
	    return false;
	delete children[n];
	children.erase(children.begin( )+n);
	return true;
    }
    //---------------------------------------------------------------------


    //---------------------------------------------------------------------
    void tree::insert_child(size_t n, tree* p)
    {
	assert(p != NULL);
	assert(p->uplink == NULL);
	children.insert(children.begin( ) + n, p);
	p->uplink = this;
    }
    //---------------------------------------------------------------------


    //---------------------------------------------------------------------
    bool tree::is_any_attribute(const string& key) const
    {
	return (attributes.find(key) != attributes.end( ));
    }
    //---------------------------------------------------------------------


    //---------------------------------------------------------------------
    void tree::write(ostream& out, bool print_attributes, int indentation) const
    {
	size_t i;
	map<string, attribute_struct>::const_iterator it;
	attribute_struct a;

	// Print the label and attributes of the root:
	out << setw(indentation) << "" << "==>" << root_label << endl;
	if (print_attributes)
	{
	    for (it = attributes.begin( ); it != attributes.end( ); ++it)
	    {
		out << setw(indentation) << "   " << "|" << it->first << "| ";
		a = it->second;
		a.printer(out, a.data_ptr);
		out << endl;
	    }
	}

	// Print the children with +3 indentation:
	for (i = 0; i < children.size( ); ++i)
	{
	    children[i]->write(out, print_attributes, indentation+3);
	}
    }
}
