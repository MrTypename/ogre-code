/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2003 The OGRE Team
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
#include "OgreD3D9GpuProgramManager.h"
#include "OgreD3D9GpuProgram.h"

namespace Ogre {
    //-----------------------------------------------------------------------------
    D3D9GpuProgramManager::D3D9GpuProgramManager(LPDIRECT3DDEVICE9 device)
        :mpDevice(device)
    {
    }
    //-----------------------------------------------------------------------------
    GpuProgramParametersSharedPtr D3D9GpuProgramManager::createParameters(void)
    {
        return GpuProgramParametersSharedPtr(new GpuProgramParameters());
    }
    //-----------------------------------------------------------------------------
    GpuProgram* D3D9GpuProgramManager::create(const String& name, GpuProgramType gptype, const String& syntaxCode)
    {
        if (gptype == GPT_VERTEX_PROGRAM)
        {
            return new D3D9GpuVertexProgram(name, syntaxCode, mpDevice);
        }
        else
        {
            return new D3D9GpuFragmentProgram(name, syntaxCode, mpDevice);
        }
    }

}
