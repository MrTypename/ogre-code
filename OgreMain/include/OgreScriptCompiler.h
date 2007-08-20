/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#ifndef __SCRIPTCOMPILER_H_
#define __SCRIPTCOMPILER_H_

#include "OgreScriptParser.h"
#include "OgreDataStream.h"

namespace Ogre{

	/** This class is the base for all compiler listeners.
		The base class provides a mechanism for overriding the script importing
		behavior.
	*/
	class _OgreExport ScriptCompilerListener
	{
	public:
		virtual ScriptNodeListPtr importFile(const String &name);
		virtual void preParse(WordIDMap &ids);
	};

	enum
	{
		CE_UNKNOWNTOKEN,
		CE_OPENBRACEEXPECTED,
		CE_VARIABLEEXPECTED,
		CE_VARIABLEVALUEEXPECTED,
		CE_UNDEFINEDVARIABLE,
		CE_OBJECTNAMEEXPECTED,
		CE_OBJECTTYPEEXPECTED,
		CE_STRINGEXPECTED,
		CE_NUMBEREXPECTED,
		CE_VALUEEXPECTED,
		CE_INVALIDPROPERTY,
		CE_INVALIDPROPERTYVALUE,
		CE_OBJECTALLOCATIONERROR,
		CE_TRUTHVALUEEXPECTED,
		CE_BASE_ERRORS_END
	};

	/** This struct stores semantic error information. It is information
		about errors which may occur during the script compilation phase.
		This is different than a script parsing error (syntax error).
	*/
	struct ScriptCompilerError
	{
		uint32 error;
		String file;
		int line, column;
	};
	typedef SharedPtr<ScriptCompilerError> ScriptCompilerErrorPtr;
	typedef std::list<ScriptCompilerErrorPtr> ScriptCompilerErrorList;

	/** This class acts as a base class for all compilers of Ogre scripts.
		Each script target type (e.g. Materials, Particle Systems, etc.) has
		its own derived compiler.

		The base class defines a few convenient entry points for the compilation
		process. It then pre-processes the input before handing the final
		abstract syntax tree over to the derived class for translation into
		the target form.
	*/
	class _OgreExport ScriptCompiler
	{
	protected: // Variables and scope types
		// This type stores information about the specific variable
		typedef std::map<String,String> ScopedVariableMap;

		// This is the structure storing a single lexical scoping of variables
		struct LexicalScope
		{
			ScopedVariableMap vars;
		};
		typedef Ogre::SharedPtr<LexicalScope> LexicalScopePtr;
		typedef std::list<LexicalScopePtr> ScopeStack;
	protected: // Common word id values
		enum
		{
			ID_ON,
			ID_OFF,
			ID_TRUE,
			ID_FALSE,
			ID_YES,
			ID_NO
		};
	public:
		ScriptCompiler();

		/** This function accepts the script text and the source of the script
			and then delegates the compilation to the derived compiler.

			@param text This is the source code of the script being compiled
			@param source This is the source of the script (usually a file or resource name)
			@param group The resource group the final resources belong to
		*/
		bool compile(const String &text, const String &group, const String &source);
		/** This function takes in a list of ScriptNodePtr objects. Essentially, this
			is an unprocessed abstract syntax tree, as it would be returned from the
			parse function. It then processes the nodes and delegates final compilation
			to the derived compiler.

			@param nodes The abstract syntax tree representing the source code to compile
			@param group The resource group the final resources belong to
		*/
		bool compile(ScriptNodeListPtr nodes, const String &group);
		/** Accepts a DataStreamPtr which represents the script being compiled.
		    This stream is delegated to the text-based compilation function.

			@param stream The data stream of the script
			@param group This is the resource group to compile the script within
		*/
		bool compile(DataStreamPtr &stream, const String &group);
		/** Returns the resource group that the currently compiling script belongs to
		*/
		const String &getGroup() const;
		/** Returns the list of errors from the last script compilation compilation
		*/
		const ScriptCompilerErrorList &getErrors() const;
		/** Returns a constant reference to the word id map used by this compiler.
		@remarks The word id map identifies key words and maps them to integer keys.
		Some compilers prefer to deal with tokens as integers intead of using strings.
		This word id map is the mechanism that links string token to integer identifier.
		*/
		const WordIDMap &getWordIDs() const;
	protected: // Operations
		/// This is the overridable function for base classes to compile the AST
		virtual bool compileImpl(ScriptNodeListPtr nodes) = 0;
		/** This function descends into the tree and does a replacement of the variables.
			Variables are replaced with their AST representations, which are fully processed.
			This means a variable can expand to any valid construct, since inheritance and
			variable processing occurs on the replacement
		*/
		void processVariables(ScriptNodeList &nodes, const ScriptNodeListPtr &top);
		/** This function handles inheritance expansions.
		*/
		void processObjects(ScriptNodeList &nodes, const ScriptNodeListPtr &top);
		/// This built-in function processes import nodes
		void processImports(ScriptNodeListPtr &nodes);
		/** This should be overridden by the given compiler. It is meant to load the given
			script and turn it into an AST.

			This base version uses the ResourceGroupManager to load the script
			and uses the parse function to turn it into an un-processed AST.
			The calling compiler should further process this AST if it uses
			this base version.
		*/
		virtual ScriptNodeListPtr loadImportPath(const String &name);
		/** This should be overridden by the given compiler. It located the import
			target within the AST and returns only the tree for that target object.
		*/
		ScriptNodeListPtr locateTarget(ScriptNodeList &nodes, const String &target);
		/** This function is intended to do a search within the given level of the tree
			for an object of the same name. If the object exists, it should return true, otherwise
			it returns false.
		*/
		bool containsObject(const ScriptNodeList &nodes, const String &name);
		// Retrieves the node at the index away from the current iterator, or a null node
		ScriptNodePtr getNodeAt(ScriptNodeList::const_iterator &from, ScriptNodeList::const_iterator &end, int index) const;
		// Retrieves an iterator to the next node of the given type
		ScriptNodeList::iterator findNode(ScriptNodeList::iterator &from, ScriptNodeList::iterator &end, uint32 type) const;
		// Retrieves an iterator to the next node of the given type
		ScriptNodeList::const_iterator findNode(ScriptNodeList::const_iterator &from, ScriptNodeList::const_iterator &end, uint32 type) const;
		// Verifies that the next node is the given type and that it exists
		bool verifyNextNodeType(ScriptNodeList::const_iterator &i, ScriptNodeList::const_iterator &end, uint32 type) const;
		/// Retrieves the truth value from the node and returns it in val. Returns true if successful, false if not.
		bool getTruthValue(const ScriptNodePtr &node, bool &val) const;
		/// This registers a new error
		void addError(uint32 error, const String &file, int line, int col);
		/// This pushes a new scope onto the stack, copying variables from the higher stack into it
		void pushScope();
		/// This pops the top scope off the stack. If there are no scopes on the stack, it does nothing
		void popScope();
		/// This searches the current scope for the given variable and returns an iterator to it and true if successful
		std::pair<bool,String> findVariable(const String &name);
		/// Sets the value of the variable in the current scope, does nothing if there is no scope
		void setVariable(const String &name, const String &value);
	protected:
		// Compiler context data
		String mGroup; // The resource group of the resultant resources

		// This controls whether objects in the script can have just names and not types
		bool mAllowNontypedObjects;
		
		typedef std::map<String,ScriptNodeListPtr> ImportCacheMap;
		ImportCacheMap mImports; // The set of imported scripts to avoid circular dependencies
		typedef std::multimap<String,String> ImportRequestMap;
		ImportRequestMap mImportRequests; // This holds the target objects for each script to be imported

		// This stores the imports of the scripts, so they are separated and can be treated specially
		ScriptNodeList mImportTable;

		// The stack used to process our variables
		ScopeStack mStack;

		// Error information
		ScriptCompilerErrorList mErrors;

		// This is the wordID map sent to the parser
		WordIDMap mWordIDs;
	};

}

#endif
