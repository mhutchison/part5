// File: symtab.h
// Written by: Michael Main
// Version: Jan 12, 2006
// This file provides a general symbol table class that can be
// used to keep track of the symbols in a compiler or other translator.

#ifndef COLORADO_SYMTAB
#define COLORADO_SYMTAB
#include <cstdlib>       // Provides NULL
#include <map>           // Provides vector template class
#include <stack>         // Provides stack template class
#include <string>        // Provides string class
#include <vector>        // Provides vector template class

namespace colorado
{
    template <class value_type>
    class symbol_table
    {
    public:
	struct symbol_info
	{
	    std::string name;
	    int depth;
	    int kind;
	    int offset;
	    value_type value;
	};

	struct kind_info
	{
	    int kind;
	    int start_offset;
	    int delta;
	    int current_offset;
	};
	
	symbol_table( ) { clear( ); }

	void clear( );
	void enter_scope( );
	void exit_scope( );
	bool insert(std::string name, int kind, value_type value);
	void register_kind(int kind, int start_offset, int delta, bool reset_for_new_scope);
	symbol_info* seek(std::string name);
	symbol_info* seek(int depth);

	int current_depth( ) const { return cd; }
	bool has_kind(int kind) const { return kinds.count(kind) > 0; }
	bool seek_ok( ) const { return sought != NULL; }
	int many_bytes(int kind) const;
	int many_symbols(int kind) const;
	
        std::string name( ) const;
	int depth( ) const;
	int kind( ) const;
	int offset( ) const;
	value_type value( ) const;
	
    private:
	int cd;
	symbol_info* sought;
	std::vector<symbol_info> symbols;
	std::map<int, kind_info> kinds;
	std::stack< kind_info> saved;
	std::vector<int> kinds_to_save;
    };
}
#include "symtab.template"
#endif
	
	
