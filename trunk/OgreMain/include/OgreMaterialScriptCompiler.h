/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/

#ifndef __MaterialScriptScompiler_H__
#define __MaterialScriptScompiler_H__

#include "OgreCompiler2Pass.h"

namespace Ogre {

    class MaterialScriptCompiler : public Compiler2Pass
    {
    private:
	    // Token ID enumeration
	    enum SymbolID {
		    // Terminal Tokens section
            mid_MATERIAL = 0, mid_CLONE, mid_TECHNIQUE, mid_PASS, mid_TEXTURE_UNIT,

    		// non-terminal tokens section
            mid_SCRIPT, mid_SCRIPT_OPTIONS, mid_SCRIPT_PROPERTIES, mid_MATERIAL_DEF, mid_VERTEX_PROGRAM_DEF,
            mid_FRAGMENT_PROGRAM_DEF, mid_MATERIAL_CLONE, mid_MATERIAL_OPTIONS, mid_MATERIAL_PROPERTIES,

            mid_TECHNIQUE_DEF, mid_SET_TEXTURE_ALIAS_DEF, mid_LOD_DISTANCES_DEF, mid_RECEIVE_SHADOWS_DEF,
            mid_TRANSPARENCY_CASTS_SHADOWS_DEF, mid_TECHNIQUE_OPTIONS, mid_TECHNIQUE_PROPERTIES,

            mid_PASS_DEF, mid_PASS_OPTIONS, mid_LOD_INDEX_DEF, mid_PASS_PROPERTIES, mid_AMBIENT_DEF, mid_DIFFUSE_DEF,
            mid_SPECULAR_DEF, mid_SCENE_BLEND_DEF, mid_DEPTH_CHECK_DEF, mid_DEPTH_WRITE_DEF,
            mid_DEPTH_FUNC_DEF, mid_ALPHA_REJECTION_DEF, mid_COLOUR_WRITE_DEF, mid_CULL_HARDWARE_DEF,
            mid_CULL_SOFTWARE_DEF, mid_LIGHTING_DEF, mid_FOG_OVERRIDE_DEF, mid_SHADING_DEF,
            mid_DEPTH_BIAS_DEF, mid_TEXTURE_UNIT_DEF, mid_TUS_OPTIONS, mid_TUS_PROPERTIES,


            mid_LABEL, mid_QUOTED_LABEL, mid_UNQUOTED_LABEL, mid_ALPHANUMERIC_SPACE,
            mid_ALPHANUMERIC, mid_CHARACTER, mid_NUMBER,

            // system token base
            mid_SYSTEM
        };

	    // static library database for tokens and BNF rules
	    static TokenRule materialScript_RulePath[];

    public:
    };
}

#endif