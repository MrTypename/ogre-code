/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as 
published by the Free Software Foundation; either version 2.1 of the 
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public 
License for more details.

You should have received a copy of the GNU Lesser General Public License 
along with this library; if not, write to the Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------*/
#ifndef __Prerequisites_H__
#define __Prerequisites_H__

// Platform-specific stuff
#include "OgrePlatform.h"

#if OGRE_COMPILER == COMPILER_MSVC
// Turn off warnings generated by long std templates
// This warns about truncation to 255 characters in debug/browse info
#   pragma warning (disable : 4786)

// disable: "conversion from 'double' to 'float', possible loss of data
#   pragma warning (disable : 4244)

// disable: "truncation from 'double' to 'float'
#   pragma warning (disable : 4305)

// disable: "<type> needs to have dll-interface to be used by clients'
// Happens on STL member variables which are not public therefore is ok
#   pragma warning (disable : 4251)

// disable: "non dll-interface class used as base for dll-interface class"
// Happens when deriving from Singleton because bug in compiler ignores
// template export
#   pragma warning (disable : 4275)

// disable: "C++ Exception Specification ignored"
// This is because MSVC 6 did not implement all the C++ exception 
// specifications in the ANSI C++ draft.
#   pragma warning( disable : 4290 )

// disable: "no suitable definition provided for explicit template 
// instantiation request" Occurs in VC7 for no justifiable reason on all 
// #includes of Singleton
#   pragma warning( disable: 4661)
#endif

/* Include all the standard header *after* all the configuration
   settings have been made.
*/
#include "OgreStdHeaders.h"


#include "OgreMemoryManager.h"

// define the real number values to be used
// default to use 'float' unless precompiler option set
namespace Ogre {

    #if OGRE_DOUBLE_PRECISION == 1
        typedef double Real;
    #else
        typedef float Real;
    #endif

    // define the Char type as either char or wchar_t
    #if OGRE_WCHAR_T_STRINGS == 1
    #   define OgreChar wchar_t
    #	define _T( x ) L##x
    #else
    #   define OgreChar char
    #	define _T( x ) x
    #endif

    #ifdef GCC_3_1
    #   define HashMap ::__gnu_cxx::hash_map
    #else
    #   define HashMap ::std::hash_map
    #endif

    /** In order to avoid finger-aches :)
    */
    typedef unsigned char uchar;
    typedef unsigned short ushort;
    typedef unsigned int uint;  
    typedef unsigned long ulong;

// Pre-declare classes
// Allows use of pointers in header files without including individual .h
// so decreases dependencies between files
    class Animation;
    class AnimationTrack;
    class AnimationState;
    class ArchiveEx;
    class ArchiveManager;
    class ArchiveFactory;
    class AxisAlignedBox;
    class Billboard;
    class BillboardSet;
    class Bone;
    class Camera;
    class Codec;
    class ColourValue;
    class ControllerManager;
    class Controller;
    class DataChunk;
    class DynLib;
    class DynLibManager;
    class Entity;
    class Factory;
    class Font;
    class FontManager;
    class FrameListener;
    class GuiElement;
    class GuiElementFactory;
    class GuiContainer;
    class GuiManager;
    class InputReader;
    class KeyFrame;
    class Light;
    class LogManager;
    class Material;
    class MaterialManager;
    class Math;
    class Matrix3;
    class Matrix4;
    class MemoryManager;
    class Mesh;
    class MeshManager;
    class MovableObject;
    class Node;
    class Overlay;
    class OverlayManager;
    class Particle;
    class ParticleAffector;
    class ParticleAffectorFactory;
    class ParticleEmitter;
    class ParticleEmitterFactory;
    class ParticleSystem;
    class ParticleSystemManager;
    class Plane;
    class Quaternion;
    class Renderable;
    class RenderPriorityGroup;
    class RenderQueue;
    class RenderQueueGroup;
    class RenderQueueListener;
    class RenderSystem;
    class RenderTarget;
    class RenderTargetListener;
    class RenderWindow;
    class Resource;
    class ResourceManager;
    class SceneManager;
    class SceneManagerEnumerator;
    class SceneNode;
    class SDDataChunk;
    class SimpleRenderable;
    class Skeleton;
    class SkeletonManager;
    class Sphere;
    class String;
    class StringInterface;
    class SubEntity;
    class SubMesh;
	class TextureFont;
    class Vector3;
    class Viewport;   

    struct GeometryData;
}

#endif // __OgrePrerequisites_H__

