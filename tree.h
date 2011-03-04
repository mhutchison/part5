// File: tree.h
// Written by Michael Main (Sep 4, 2005)
// This is the header file for an attribute tree class.
//
// A tree object is a non-empty tree.
// Threads: The tree is not safe to use in multi-threaded applications.
// Dynamic restriction: Trees can be created only through a call to new.
// Therefore, trees cannot be local variables, global variables, or
// value parameters.  In general, use a pointer to a tree object rather
// than declaring a tree variable directly.

// The root of each tree has these items:
// 1. A string label.
//    This label can be set by the constructor or by the set_label( ) member
//    function.  It can be retrieved with the label( ) member function.
// 2. A collection of named attributes.
//    Activating the member function set_attribute<T>(key, v) sets an attribute.
//    The key argument is the name of the attribute; the v argument (of type
//    const T&) is the value that is attached to the attribute.  This value can
//    later be retrieved via the member function attribute<T>(key).
//    Warning: If an attribute is set using a set_attribute<T> with a type T,
//    then it must be retrieved using the same type <T> with attribute<T>.
// 3. Zero or more children.
//    Each child is implemented as a pointer to a non-empty subtree.
//    These children may be set by the constructor, the append_child member
//    function, or the insert_child member function.  The children are
//    numbered from 0 to many_children( )-1.  A pointer to child number n
//    is obtained by calling the member function child(n).
// 
// CONSTRUCTORS, ASSIGNMENTS and for the tree class:
//   tree(const string& label = string( ), size_t n = 0, ...)
//     Example: tree* p = new tree("Example", 2, child0_ptr, child1_ptr);
//     The first argument is the label for the root.  The second argument,
//     n, indicates how many children the root will have.  The rest of
//     the arguments are pointers to these children.
//     Precondition: The number of pointers to children must be exactly
//     equal to n.  Each child pointer, cp, must satisfy:
//     (cp != NULL) && (cp->parent( ) == NULL).
//     Postcondition: The new tree has been constructed with the given
//     root label and children.  The root does not yet have any named
//     attributes.
//
//   tree(const tree& source)
//     Postcondition: This tree has been created as a deep copy of the
//     source tree.  This new tree is not a subtree of a larger tree,
//     even if the source was.  Subsequent changes to this tree will not
//     change the source tree, nor vice versa.  
//
//   ~tree( )
//     The destructor calls clear( ).
//
//   tree& operator =(const tree& source)
//     Postcondition: This tree is now a deep copy of the source tree.
//     If this tree was a subtree of some larger tree, then it is still
//     a subtree of that tree; otherwise it is not a subtree of any
//     larger tree (even if the source was a subtree).
//     Subsequent changes to this tree will not
//     change the source tree, nor vice versa.  
//
// MEMBER FUNCTIONS for the tree class:
//   void append_child(tree* p)
//     Precondition: (p != NULL) && (p->parent( ) == NULL)
//     Postcondition: *p is the new rightmost child of this tree.
//     The parent pointer of *p has been set to this tree.
//
//   template <typename T> const T& attribute(const string& key) const
//   template <typename T> T& attribute(const string& key)
//     Precondition: is_attribute<T>(key)
//     Postcondition: The return value is a reference to the value of
//     the attribute with the specified key.
//
//   const type_info& attribute_type(const string& key) const  
//     Postcondition: If is_attribute<T>(key) is true for some type T,
//     then the return value is a reference to typeid(T).  Otherwise, the
//     return value is a reference to typeid(void).
//     return value is a reference to typeid(void).
//
//   tree* child(size_t n)
//   const tree* child(size_t n) const
//     Precondition: n < many_children( )
//     Postcondition: The return value is a pointer to child number n of
//     this tree.
//
//   void clear( )
//     Postcondition: All children and attributes have been removed from this
//     tree (and their memory released).  The label of the root has been reset
//     to the empty string.  If this tree was a subtree of some larger tree,
//     then it still is a subtree of that larger tree.
//
//   size_t depth( ) const
//     Postcondition: The return value is the depth of the tree, which the
//     maximum length of a path from the root to a leaf.
//
//   size_t depth_left( ) const
//     Postcondition: The return value is the left depth of the tree, which the
//     length of a path from the root to a leaf by following only the leftmost
//     child.
//
//   size_t depth_right( ) const
//     Postcondition: The return value is the right depth of the tree, which the
//     length of a path from the root to a leaf by following only the rightmost
//     child.
//
//   bool erase_attribute(const string& key)
//     Postcondition: If is_any_attribute(key), then the attribute with that key
//     has been removed from this tree (and its memory released).  If there
//     was an erasure, then the return value is true; otherwise the return
//     value is false.
//
//   bool erase_child(size_t n)
//     Postcondition: If many_children( ) > n, then child number n has been
//     removed from this tree (and its memory released).  If there was an
//     erasure, any previous children numbered higher than n have all
//     had their index numbers decreased by one to fill in the gap, and the
//     return value is true; otherwise the return value is false.
//
//   void insert_child(size_t n, tree* p)
//     Precondition: (p!=NULL) && (p->parent()==NULL) && (n<=many_children()).
//     Postcondition: *p is the new child number n of this tree.
//     Any previous children numbered n or higher have all had their
//     index numbers increased by one to make room for the new child.
//
//   template <typename T> bool is_attribute(const string& key) const
//     Postcondition: The return value is true if the root of this tree
//     as an attribute with type T and the specified key.  Otherwise,
//     the return value is false.
//
//   bool is_any_attribute(const string& key) const
//     Postcondition: The return value is true if the root of this tree
//     has an attribute with the specified key (and any type).  Otherwise,
//     the return value is false.
//
//   const string& label( ) const
//   string& label( ) { return root_label; }
//     Postcondition: The return value is a reference to the label of the
//     root of this tree.
//
//   size_t many_children( ) const
//     Postcondition: The return value is the number of children possessed by
//     the root of this tree.  These children are numbered from 0 to
//     many_children( )-1;
//
//   tree* parent( )
//   const tree* parent( ) const
//     Postcondition: The return value is a pointer to the larger tree that
//     has this tree as its subtree.  If there is no such larger tree, then the
//     return value is NULL.
//
//   template <typename T> void set_attribute(const string& key, T data)
//     Postcondition: An attribute with the specified key has been set
//     for the root of this tree.  The data parameter gives the value
//     of this parameter.  If there was a previous attribute with the same
//     key, then that attribute has been removed (and its memory released).
//     The attribute can later be retrieved with attribute<T>(key).
//
//   void set_label(const string label)
//     Postcondition: The label of the root of this tree has been changed
//     to the specified value.
//
//   void write(ostream& out, bool print_attributes = true, int indentation = 0)
//     Postcondition: A representation of this tree (including all labels and
//     attributes) has been printed to the specified ostream.  All output lines
//     in this representation are indented by the specified amount.  If
//     print_attributes is true, then each nodes attributes will be printed
//     below its label.

#ifndef COLORADO_TREE
#define COLORADO_TREE
#include <iostream>      // Provides ostream
#include <string>        // Provides string class
#include <map>           // Provides map class
#include <typeinfo>      // Provides type_info and typeid
#include <vector>        // Provides vector class
#include <assert.h>      // Provides assert macro

namespace colorado
{
    // destroy<T>(p) interprets *p as a T data value that was allocated
    // on the heap.  This function deletes this T object (calling its
    // destructor and returning its memory to the heap).
    template<typename T> void destroy(void* p)
    {
	delete static_cast<T*>(p);
    }

    // copy<T>(p) interprets *p as a T data value.  This function makes
    // a copy of this value, using T's copy constructor. The return value
    // is a pointer to the newly created T object.
    template<typename T> void* copy(const void* p)
    {
	return static_cast<void*> (new T(*(static_cast<const T*>(p))));
    }

    // If data of type T can be printed with the usual << operator, then
    // print<T>(out, p) will interpret *p as a T object and print its
    // value to out.  Otherwise, a message is printed to out, indicating
    // that objects of type T are not printable.
    template<typename T> void print(std::ostream& out, const void* p)
    {
	// The first part of this code sets an enum value, is_printable, to
	// be 1 if the data type T can be printed with the usual <<
	// operator.  Otherwise, is_printable is set to zero.  The programming
	// technique is based on a note from Herb Sutter at
	// http://www.gotw.ca/gotw/071.htm
	class object
	{
	public:
	    object(T convert) { };
	};
	char operator << (std::ostream&, const object&);
	enum { is_printable = sizeof(std::cout << (*static_cast<T*>(0))) == sizeof(char) ? 0 : 1 };
	
        // Notice that the boolean expression in the if-statement is known at
	// compile time, so that only one of the two output statements will be
	// compiled into object code.
	if (is_printable)
	    out << *static_cast<const T*>(p);
	else
	    out << "(value of type " << typeid(T).name() << " cannot be printed)";
    }

    class tree
    {
    protected:
	// An attribute_struct contains information about an attribute that's
	// attached to the root of a tree.  The data_ptr points to the actual
	// value of the attribute. The type_info for this information is in
	// *data_type.  The other three members are pointers to functions.
	// If the type of the attribute value is T, then:
	//   destroyer(p) points to destroy<T>
	//   copier(p) points to  copy<T>
	//   print(p) points to print<T>
        struct attribute_struct
	{
	    void* data_ptr;
	    const std::type_info* data_type;
	    void (*destroyer)(void*);
	    void* (*copier)(const void*);
	    void (*printer)(std::ostream&, const void*); 
	};
	
	// Information about the root of this tree:
        std::string root_label;        // Label of the root
        tree* uplink;                  // Pointer to parent
        std::vector<tree*> children;   // Ptrs to children
    
	// Information about the attributes attached to the root.
	// attributes[xxx] is the attribute that was attached with key xxx.
	std::map<std::string, attribute_struct> attributes;

	// The following varible is set to true by the new operator
	// whenever a new tree is being allocated.  The constructor
	// sets it back to false.
	static bool new_called;
    public:
        tree(const std::string& label = std::string( ), size_t n = 0, ...);
        tree(const tree& source);
        ~tree( ) { clear( ); }
        tree& operator =(const tree& source);
	void* operator new(size_t bytes);
	void operator delete(void* p);
	
        void append_child(tree* p);
	template <typename T> const T& attribute(const std::string& key) const
	{
	    assert(is_attribute_details<T>(key));
	    // Cannot use attributes[key] on a const map; use find instead.
	    return *(static_cast<T*>(attributes.find(key)->second.data_ptr)); 
	}
	template <typename T> T& attribute(const std::string& key)
	{
	    assert(is_attribute_details<T>(key));
	    return *(static_cast<T*>(attributes[key].data_ptr)); 
	}
	const std::type_info& attribute_type(const std::string& key) const;  
        tree* child(size_t n);
        const tree* child(size_t n) const;
        void clear( );
        size_t depth( ) const;
        size_t depth_left( ) const;
        size_t depth_right( ) const;
        bool erase_attribute(const std::string& key);
	bool erase_child(size_t n);
        void insert_child(size_t n, tree* p);
        template <typename T> bool is_attribute(const std::string& key) const
	{
	    if (attributes.count(key) > 0)
	    {
		// Cannot use attributes[key] on a const map; use find instead.
		return typeid(T) == *(attributes.find(key)->second.data_type);
	    }
	    else
		return false;
	}
        template <typename T> bool is_attribute_details(const std::string& key) const
	{
	    if (!is_attribute<T>(key))
	    {
		write(std::cerr);
		std::cerr << "Trying to find "  << key << " attribute "
			  << "in tree shown above." << std::endl;
		return false;
	    }
	    else
		return true;
	}
        bool is_any_attribute(const std::string& key) const;
        const std::string& label( ) const { return root_label; }
        std::string& label( ) { return root_label; }
        size_t many_children( ) const { return children.size( ); }
        tree* parent( ) { return uplink; }
        const tree* parent( ) const { return uplink; }
        template <typename T> void set_attribute(const std::string& key, T data)
	{
	    T* p = new T(data); // A new T object that is a copy of data
            attribute_struct a;

            if (attributes.count(key) > 0)
	    {
		a = attributes[key];	    
		a.destroyer(a.data_ptr);
	    }
	    a.data_ptr = p;
	    a.data_type = &(typeid(T));
	    a.destroyer = destroy<T>;
	    a.copier = copy<T>;
	    a.printer = print<T>;
	    attributes[key] = a;
	}
	void set_label(const std::string label) { root_label = label; }
	void write(std::ostream& out, bool print_attributes = true, int indentation = 0) const;
    };

}
#endif
