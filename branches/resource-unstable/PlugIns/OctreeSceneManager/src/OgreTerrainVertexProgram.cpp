
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2004 The OGRE Team
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

#include "OgreTerrainVertexProgram.h"

namespace Ogre {

    String TerrainVertexProgram::mNoFogArbvp1 = 
        "!!ARBvp1.0\n"
        "PARAM c5 = { 1, 1, 1, 1 };\n"
        "#var float4x4 worldViewProj :  : c[0], 4 : 8 : 1\n"
        "#var float morphFactor :  : c[4] : 9 : 1\n"
        "TEMP R0;\n"
        "ATTRIB v17 = vertex.weight;\n"
        "ATTRIB v25 = vertex.texcoord[1];\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "	MOV result.texcoord[0].xy, v24;\n"
        "	MOV result.texcoord[1].xy, v25;\n"
        "	MOV R0, v16;\n"
        "	MAD R0.y, v17.x, c4.x, R0.y;\n"
        "	DP4 result.position.x, c0[0], R0;\n"
        "	DP4 result.position.y, c0[1], R0;\n"
        "	DP4 result.position.z, c0[2], R0;\n"
        "	DP4 result.position.w, c0[3], R0;\n"
        "	MOV result.color.front.primary, c5.x;\n"
        "END\n";

    String TerrainVertexProgram::mLinearFogArbvp1 = 
        "!!ARBvp1.0\n"
        "PARAM c5 = { 1, 1, 1, 1 };\n"
        "#var float4x4 worldViewProj :  : c[0], 4 : 9 : 1\n"
        "#var float morphFactor :  : c[4] : 10 : 1\n"
        "TEMP R0, R1;\n"
        "ATTRIB v17 = vertex.weight;\n"
        "ATTRIB v25 = vertex.texcoord[1];\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "	MOV result.texcoord[0].xy, v24;\n"
        "	MOV result.texcoord[1].xy, v25;\n"
        "	MOV R1, v16;\n"
        "	MAD R1.y, v17.x, c4.x, R1.y;\n"
        "	DP4 R0.x, c0[0], R1;\n"
        "	DP4 R0.y, c0[1], R1;\n"
        "	DP4 R0.z, c0[2], R1;\n"
        "	DP4 R0.w, c0[3], R1;\n"
        "	MOV result.fogcoord.x, R0.z;\n"
        "	MOV result.position, R0;\n"
        "	MOV result.color.front.primary, c5.x;\n"
        "END\n";
    String TerrainVertexProgram::mExpFogArbvp1 = 
        "!!ARBvp1.0\n"
        "PARAM c6 = { 1, 1, 1, 1 };\n"
        "PARAM c7 = { 2.71828, 0, 0, 0 };\n"
        "#var float4x4 worldViewProj :  : c[0], 4 : 9 : 1\n"
        "#var float morphFactor :  : c[4] : 10 : 1\n"
        "#var float fogDensity :  : c[5] : 11 : 1\n"
        "TEMP R0, R1;\n"
        "ATTRIB v17 = vertex.weight;\n"
        "ATTRIB v25 = vertex.texcoord[1];\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c5 = program.local[5];\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "	MOV result.texcoord[0].xy, v24;\n"
        "	MOV result.texcoord[1].xy, v25;\n"
        "	MOV R1, v16;\n"
        "	MAD R1.y, v17.x, c4.x, R1.y;\n"
        "	DP4 R0.x, c0[0], R1;\n"
        "	DP4 R0.y, c0[1], R1;\n"
        "	DP4 R0.z, c0[2], R1;\n"
        "	DP4 R0.w, c0[3], R1;\n"
        "	MOV result.position, R0;\n"
        "	MOV result.color.front.primary, c6.x;\n"
        "	MUL R0.zw, R0.z, c5.x;\n"
        "	MOV R0.xy, c7.x;\n"
        "	LIT R0.z, R0;\n"
        "	RCP result.fogcoord.x, R0.z;\n"
        "END\n";
    String TerrainVertexProgram::mExp2FogArbvp1 = 
        "!!ARBvp1.0\n"
        "PARAM c6 = { 1, 1, 1, 1 };\n"
        "PARAM c7 = { 0.002, 2.71828, 0, 0 };\n"
        "#var float4x4 worldViewProj :  : c[0], 4 : 9 : 1\n"
        "#var float morphFactor :  : c[4] : 10 : 1\n"
        "#var float fogDensity :  : c[5] : 11 : 1\n"
        "TEMP R0, R1;\n"
        "ATTRIB v17 = vertex.weight;\n"
        "ATTRIB v25 = vertex.texcoord[1];\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "	MOV result.texcoord[0].xy, v24;\n"
        "	MOV result.texcoord[1].xy, v25;\n"
        "	MOV R1, v16;\n"
        "	MAD R1.y, v17.x, c4.x, R1.y;\n"
        "	DP4 R0.x, c0[0], R1;\n"
        "	DP4 R0.y, c0[1], R1;\n"
        "	DP4 R0.z, c0[2], R1;\n"
        "	DP4 R0.w, c0[3], R1;\n"
        "	MOV result.position, R0;\n"
        "	MOV result.color.front.primary, c6.x;\n"
        "	MUL R0.x, R0.z, c7.x;\n"
        "	MUL R0.zw, R0.x, R0.x;\n"
        "	MOV R0.xy, c7.y;\n"
        "	LIT R0.z, R0;\n"
        "	RCP result.fogcoord.x, R0.z;\n"
        "END\n";

    String TerrainVertexProgram::mNoFogVs_1_1 = 
        "vs_1_1\n"
        "def c5, 1, 1, 1, 1\n"
        "//var float4x4 worldViewProj :  : c[0], 4 : 8 : 1\n"
        "//var float morphFactor :  : c[4] : 9 : 1\n"
        "dcl_blendweight v1\n"
        "dcl_texcoord1 v8\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "	mov oT0.xy, v7\n"
        "	mov oT1.xy, v8\n"
        "	mov r0, v0\n"
        "	mad r0.y, v1.x, c4.x, r0.y\n"
        "	dp4 oPos.x, c0, r0\n"
        "	dp4 oPos.y, c1, r0\n"
        "	dp4 oPos.z, c2, r0\n"
        "	dp4 oPos.w, c3, r0\n"
        "	mov oD0, c5.x\n";
    String TerrainVertexProgram::mLinearFogVs_1_1 = 
        "vs_1_1\n"
        "def c5, 1, 1, 1, 1\n"
        "//var float4x4 worldViewProj :  : c[0], 4 : 9 : 1\n"
        "//var float morphFactor :  : c[4] : 10 : 1\n"
        "dcl_blendweight v1\n"
        "dcl_texcoord1 v8\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "	mov oT0.xy, v7\n"
        "	mov oT1.xy, v8\n"
        "	mov r1, v0\n"
        "	mad r1.y, v1.x, c4.x, r1.y\n"
        "	dp4 r0.x, c0, r1\n"
        "	dp4 r0.y, c1, r1\n"
        "	dp4 r0.z, c2, r1\n"
        "	dp4 r0.w, c3, r1\n"
        "	mov oFog, r0.z\n"
        "	mov oPos, r0\n"
        "	mov oD0, c5.x\n";
    String TerrainVertexProgram::mExpFogVs_1_1 = 
        "vs_1_1\n"
        "def c6, 1, 1, 1, 1\n"
        "def c7, 2.71828, 0, 0, 0\n"
        "//var float4x4 worldViewProj :  : c[0], 4 : 9 : 1\n"
        "//var float morphFactor :  : c[4] : 10 : 1\n"
        "//var float fogDensity :  : c[5] : 11 : 1\n"
        "dcl_blendweight v1\n"
        "dcl_texcoord1 v8\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "	mov oT0.xy, v7\n"
        "	mov oT1.xy, v8\n"
        "	mov r1, v0\n"
        "	mad r1.y, v1.x, c4.x, r1.y\n"
        "	dp4 r0.x, c0, r1\n"
        "	dp4 r0.y, c1, r1\n"
        "	dp4 r0.z, c2, r1\n"
        "	dp4 r0.w, c3, r1\n"
        "	mov oPos, r0\n"
        "	mov oD0, c6.x\n"
        "	mul r0.zw, r0.z, c5.x\n"
        "	mov r0.xy, c7.x\n"
        "	lit r0.z, r0\n"
        "	rcp oFog, r0.z\n";
    String TerrainVertexProgram::mExp2FogVs_1_1 = 
        "vs_1_1\n"
        "def c6, 1, 1, 1, 1\n"
        "def c7, 0.002, 2.71828, 0, 0\n"
        "//var float4x4 worldViewProj :  : c[0], 4 : 9 : 1\n"
        "//var float morphFactor :  : c[4] : 10 : 1\n"
        "//var float fogDensity :  : c[5] : 11 : 1\n"
        "dcl_blendweight v1\n"
        "dcl_texcoord1 v8\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "	mov oT0.xy, v7\n"
        "	mov oT1.xy, v8\n"
        "	mov r1, v0\n"
        "	mad r1.y, v1.x, c4.x, r1.y\n"
        "	dp4 r0.x, c0, r1\n"
        "	dp4 r0.y, c1, r1\n"
        "	dp4 r0.z, c2, r1\n"
        "	dp4 r0.w, c3, r1\n"
        "	mov oPos, r0\n"
        "	mov oD0, c6.x\n"
        "	mul r0.x, r0.z, c7.x\n"
        "	mul r0.zw, r0.x, r0.x\n"
        "	mov r0.xy, c7.y\n"
        "	lit r0.z, r0\n"
        "	rcp oFog, r0.z\n";

    const String& TerrainVertexProgram::getProgramSource(
        FogMode fogMode, const String syntax)
    {
        switch(fogMode)
        {
        case FOG_NONE:
            if (syntax == "arbvp1")
            {
                return mNoFogArbvp1;
            }
            else
            {
                return mNoFogVs_1_1;
            }
            break;
        case FOG_LINEAR:
            if (syntax == "arbvp1")
            {
                return mLinearFogArbvp1;
            }
            else
            {
                return mLinearFogVs_1_1;
            }
            break;
        case FOG_EXP:
            if (syntax == "arbvp1")
            {
                return mExpFogArbvp1;
            }
            else
            {
                return mExpFogVs_1_1;
            }
            break;
        case FOG_EXP2:
            if (syntax == "arbvp1")
            {
                return mExp2FogArbvp1;
            }
            else
            {
                return mExp2FogVs_1_1;
            }
            break;
        };
        // default
        return StringUtil::BLANK;

    }
}