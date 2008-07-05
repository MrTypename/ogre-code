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

/**
    \file 
        Bezier.h
    \brief
        Specialisation of OGRE's framework application to show off
        the bezier patch support.
*/

#include "ExampleApplication.h"
#include "OgreStringConverter.h"

// Hack struct for test
PatchMesh* patch;
Entity* patchEntity;

// Event handler to add ability to alter subdivision
class BezierListener : public ExampleFrameListener
{
protected:
public:
    BezierListener(RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam)
    {
        
    }

    bool frameStarted(const FrameEvent& evt)
    {
        static Real timeLapse = 0.0f;
        static Real factor = 0.0;
        static bool wireframe = 0;


        timeLapse += evt.timeSinceLastFrame;

        // Prgressively grow the patch
        if (timeLapse > 1.0f)
        {
            factor += 0.2;

            if (factor > 1.0f) 
            {
                wireframe = !wireframe;
                //mCamera->setDetailLevel(wireframe ? SDL_WIREFRAME : SDL_SOLID);
                patchEntity->setRenderDetail(wireframe ? SDL_WIREFRAME : SDL_SOLID);
                factor = 0.0f;

            }

            patch->setSubdivision(factor);
            mWindow->setDebugText("Bezier subdivision factor: " + StringConverter::toString(factor));
            timeLapse = 0.0f;

        }

        // Call default
        return ExampleFrameListener::frameStarted(evt);
    }
};

       
class BezierApplication : public ExampleApplication
{
protected:
    VertexDeclaration* patchDecl;
    Real* patchCtlPoints;

public:
    BezierApplication() : patchDecl(NULL), patchCtlPoints(NULL) { }
    ~BezierApplication()
    {
        if (patchCtlPoints)
            delete [] patchCtlPoints;

        // patch vertex declaration will be deleted automatically
    }

protected:

#if OGRE_COMPILER == COMPILER_MSVC
    #pragma pack(push)
    #pragma pack(1)
#endif
    struct PatchVertex {
        Real x, y, z;
        Real nx, ny, nz;
        Real u, v;
    };
#if OGRE_COMPILER == COMPILER_MSVC
    #pragma pack(pop)
#endif

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-0.5, -0.5, 0);

        // Create patch
        patchDecl = HardwareBufferManager::getSingleton().createVertexDeclaration();
        patchDecl->addElement(0, 0, VET_FLOAT3, VES_POSITION);
        patchDecl->addElement(0, sizeof(Real)*3, VET_FLOAT3, VES_NORMAL);
        patchDecl->addElement(0, sizeof(Real)*6, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);

        // Make a 3x3 patch for test
        patchCtlPoints = (Real*)( new PatchVertex[9] );

        // Patch data
        PatchVertex *pVert = (PatchVertex*)patchCtlPoints;

        pVert->x = -500.0; pVert->y = 200.0; pVert->z = -500.0;
        pVert->nx = -0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.0; pVert->v = 0.0;
        pVert++;
        pVert->x = 0.0; pVert->y = 500.0; pVert->z = -750.0;
        pVert->nx = 0.0; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.5; pVert->v = 0.0;
        pVert++;
        pVert->x = 500.0; pVert->y = 1000.0; pVert->z = -500.0;
        pVert->nx = 0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 1.0; pVert->v = 0.0;
        pVert++;

        pVert->x = -500.0; pVert->y = 0.0; pVert->z = 0.0;
        pVert->nx = -0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.0; pVert->v = 0.5;
        pVert++;
        pVert->x = 0.0; pVert->y = 500.0; pVert->z = 0.0;
        pVert->nx = 0.0; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.5; pVert->v = 0.5;
        pVert++;
        pVert->x = 500.0; pVert->y = -50.0; pVert->z = 0.0;
        pVert->nx = 0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 1.0; pVert->v = 0.5;
        pVert++;

        pVert->x = -500.0; pVert->y = 0.0; pVert->z = 500.0;
        pVert->nx = -0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.0; pVert->v = 1.0;
        pVert++;
        pVert->x = 0.0; pVert->y = 500.0; pVert->z = 500.0;
        pVert->nx = 0.0; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 0.5; pVert->v = 1.0;
        pVert++;
        pVert->x = 500.0; pVert->y = 200.0; pVert->z = 800.0;
        pVert->nx = 0.5; pVert->ny = 0.5; pVert->nz = 0.0;
        pVert->u = 1.0; pVert->v = 1.0;
        pVert++;


        patch = MeshManager::getSingleton().createBezierPatch(
            "Bezier1", patchCtlPoints, patchDecl, 
            3, 3, 5, 5, PatchSurface::VS_BOTH);

        // Start patch at 0 detail
        patch->setSubdivision(0.0f);
        // Create entity based on patch
        patchEntity = mSceneMgr->createEntity("Entity1", "Bezier1");

        Material *pMat = (Material*)MaterialManager::getSingleton().create( "TextMat" );
        pMat->addTextureLayer( "BumpyMetal.jpg" );
        patchEntity->setMaterialName("TextMat");

        // Attach the entity to the root of the scene
        mSceneMgr->getRootSceneNode()->attachObject(patchEntity);

        mCamera->setPosition(500,500, 1500);
        mCamera->lookAt(0,200,-300);

    }
	void createFrameListener(void)
    {
		// This is where we instantiate our own frame listener
        mFrameListener= new BezierListener(mWindow, mCamera);
        mRoot->addFrameListener(mFrameListener);

    }

};