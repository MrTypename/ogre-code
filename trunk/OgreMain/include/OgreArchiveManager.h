/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef _ArchiveManager_H__
#define _ArchiveManager_H__

#include "OgrePrerequisites.h"

#include "OgreResourceManager.h"
#include "OgreSingleton.h"

namespace Ogre {

    /** ResourceManager specialization to handle Archive plug-ins.
        @see
            ResourceManager
    */
    class _OgreExport ArchiveManager
        : public ResourceManager, public Singleton<ArchiveManager>
    {
    protected:
        typedef std::map<String, ArchiveFactory*> ArchiveFactoryMap;
        /// Factories available to create archives, indexed by archive type (String identifier e.g. 'Zip')
        ArchiveFactoryMap mArchFactories;

    public:
        /** Default constructor - should never get called by a client app.
        */
        ArchiveManager() {}
        /** Default destructor.
        */
        virtual ~ArchiveManager();

        /** Opens an archive for file reading.
            @remarks
                The archives are created using class factories within
                extension libraries.
            @param filename
                The filename that will be opened
            @param refLibrary
                The library that contains the data-handling code
            @param priority
                The priority of the archive (not yet used)
            @returns
                If the function succeeds, a valid pointer to an ArchiveEx
                object is returened.
            @par
                If the function fails, an exception is thrown.
        */
        ArchiveEx* load( const String& filename, const String& archiveType, int priority = 1 );

        /** Overloaded from ResourceManager.
            @see
                ResourceManager::create
        */
        Resource* create( const String& name );

        /** Adds a new ArchiveFactory to the list of available factories.
            @remarks
                Plugin developers who add new archive codecs need to call
                this after defining their ArchiveFactory subclass and
                ArchiveEx subclasses for their archive type.
        */
        void addArchiveFactory(ArchiveFactory* factory);
    };

}

#endif
