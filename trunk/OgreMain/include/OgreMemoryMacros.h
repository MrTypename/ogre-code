/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2002 The OGRE Team
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
-----------------------------------------------------------------------------
*/

//-----------------------------------------------------------------------------
// How does this work?
// Remember that before the compiler starts to process a source file, it runs
// a neat tool called a preprocessor on it. What this preprocessor does in
// this case is replace all the instances of *alloc/free with the expanded
// macros - this way we cleverly replace all the calls to the standard C
// memory (de)allocation functions. The same is done for new/delete
//
// Of course, we have the drawback that we can't name a member function of
// a class *alloc or free and we can't overload the new/delete operators without
// first undefining these macros - ah, a C++ preprocessor with RE replacement,
// that would be a dream come true :)
//
#ifndef OGRE_MEMORY_MACROS
#define OGRE_MEMORY_MACROS

#if OGRE_DEBUG_MEMORY_MANAGER && OGRE_DEBUG_MODE
#   define new    (::Ogre::MemoryManager::sMemManager.setOwner(__FILE__,__LINE__,__FUNCTION__),false) ? NULL                                                 : new
#   define delete (::Ogre::MemoryManager::sMemManager.setOwner(__FILE__,__LINE__,__FUNCTION__),false) ? ::Ogre::MemoryManager::sMemManager.setOwner("",0,"") : delete
#   define malloc(sz)      ::Ogre::MemoryManager::sMemManager.allocMem(__FILE__,__LINE__,__FUNCTION__, m_alloc_malloc, sz,      gProcessID)
#   define calloc(sz)      ::Ogre::MemoryManager::sMemManager.allocMem(__FILE__,__LINE__,__FUNCTION__, m_alloc_calloc, sz,      gProcessID)
#   define realloc(ptr,sz) ::Ogre::MemoryManager::sMemManager.rllocMem(__FILE__,__LINE__,__FUNCTION__, m_alloc_realloc,sz, ptr, gProcessID)
#   define free(ptr)       ::Ogre::MemoryManager::sMemManager.dllocMem(__FILE__,__LINE__,__FUNCTION__, m_alloc_free,       ptr, gProcessID)
#else
#   define new new
#   define delete delete
#   define malloc( sz ) ::Ogre::MemoryManager::sMemManager.allocMem( __FILE__, __LINE__, sz )
#   define free( ptr ) ::Ogre::MemoryManager::sMemManager.dllocMem( __FILE__, __LINE__, ptr )
#   define realloc( ptr, sz ) ::Ogre::MemoryManager::sMemManager.rllocMem( __FILE__, __LINE__, ptr, sz )
#   define calloc( cnt, sz ) ::Ogre::MemoryManager::sMemManager.cllocMem( __FILE__, __LINE__, cnt, sz )
#endif // OGRE_DEBUG_MEMORY_MANAGER

#endif // OGRE_MEMORY_MACROS
//-----------------------------------------------------------------------------
