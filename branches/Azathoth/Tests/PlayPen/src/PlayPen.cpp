/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
/*
-----------------------------------------------------------------------------
Filename:    PlayPen.cpp
Description: Somewhere to play in the sand...
-----------------------------------------------------------------------------
*/

#include "ExampleApplication.h"
#include "OgreProgressiveMesh.h"
#include "OgreEdgeListBuilder.h"

/*
#include "OgreNoMemoryMacros.h"
#include <ode/odecpp.h>
#include <ode/odecpp_collision.h>
#include "OgreMemoryMacros.h"
*/

/*
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include "OgreNoMemoryMacros.h"
#include <crtdbg.h>
#endi*/

#define NUM_TEST_NODES 5
SceneNode* mTestNode[NUM_TEST_NODES] = {0,0,0,0,0};
SceneNode* mLightNode = 0;
SceneNode* mRootNode;
SceneNode* camNode;
Entity* mEntity;
Real animTime = 0;
Animation* mAnim = 0;
std::vector<AnimationState*> mAnimStateList;
AnimationState* mAnimState = 0;
Overlay* mpOverlay;
Entity* pPlaneEnt;
Camera* testCam = 0;
Camera* reflectCam = 0;
SceneNode* camPlaneNode[6];
Light* mLight;
IntersectionSceneQuery* intersectionQuery = 0;
RaySceneQuery* rayQuery = 0;
Entity* ball = 0;
Vector3 ballVector;
bool testreload = false;
Plane plane;


// Hacky globals
GpuProgramParametersSharedPtr fragParams;
GpuProgramParametersSharedPtr vertParams;
MaterialPtr skin;
Frustum* frustum = 0;
Camera* theCam;

class RefractionTextureListener : public RenderTargetListener
{
public:
    void preRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        pPlaneEnt->setVisible(false);

    }
    void postRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        pPlaneEnt->setVisible(true);
    }

};
class ReflectionTextureListener : public RenderTargetListener
{
public:
    void preRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        static Plane reflectPlane(Vector3::UNIT_Y, -100);
        pPlaneEnt->setVisible(false);
        theCam->enableReflection(reflectPlane);

    }
    void postRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        pPlaneEnt->setVisible(true);
        theCam->disableReflection();
    }

};

class UberSimpleFrameListener : public FrameListener
{
protected:
	InputReader* mInputDevice;

public:
	UberSimpleFrameListener(RenderWindow* win, Camera* cam)
	{
		mInputDevice = PlatformManager::getSingleton().createInputReader();
		mInputDevice->initialise(win,true, true);
	}
	~UberSimpleFrameListener()
	{
		PlatformManager::getSingleton().destroyInputReader( mInputDevice );
	}

	bool frameStarted(const FrameEvent& evt)
	{
		mInputDevice->capture();
		if (mInputDevice->isKeyDown(KC_ESCAPE))
		{
			return false;
		}
		return true;

	}
};



class PlayPenListener : public ExampleFrameListener
{
protected:
	SceneManager* mSceneMgr;
public:
    PlayPenListener(SceneManager* mgr, RenderWindow* win, Camera* cam)
        : ExampleFrameListener(win, cam),mSceneMgr(mgr)
    {
    }


    bool frameStarted(const FrameEvent& evt)
    {

        if (!vertParams.isNull())
        {
            Matrix4 scaleMat = Matrix4::IDENTITY;
            scaleMat[0][0] = 0.5f;
            scaleMat[1][1] = -0.5f;
            scaleMat[2][2] = 0.5f;
            scaleMat[0][3] = 0.5f;
            scaleMat[1][3] = 0.5f;
            scaleMat[2][3] = 0.5f;
            Matrix4 mat = frustum->getProjectionMatrix() * 
                frustum->getViewMatrix();
            
            
            mat = scaleMat * mat;

            vertParams->setNamedConstant("texViewProjMatrix", mat);



        }


		static float reloadtime = 10.0f;
		if (testreload)
		{
			reloadtime -= evt.timeSinceLastFrame;
			if (reloadtime <= 0)
			{
				Entity* e = mSceneMgr->getEntity("1");
				e->getParentSceneNode()->detachObject("1");
				e = mSceneMgr->getEntity("2");
				e->getParentSceneNode()->detachObject("2");
				mSceneMgr->removeAllEntities();
				ResourceGroupManager::getSingleton().unloadResourceGroup("Sinbad");
				ResourceGroupManager::getSingleton().loadResourceGroup("Sinbad");

				testreload = false;

			}
		}



        bool ret = ExampleFrameListener::frameStarted(evt);

		// Sync reflection cam
		if (reflectCam)
		{
			reflectCam->setOrientation(mCamera->getOrientation());
			reflectCam->setPosition(mCamera->getPosition());
		}

		return ret;

    }

    
    bool frameEnded(const FrameEvent& evt)
    {



        // local just to stop toggles flipping too fast
        static Real timeUntilNextToggle = 0;
        static bool animate = true;
        static bool rotate = false;

        static bool firstTime = true;

        if (!firstTime)
        {
            //mCamera->yaw(20);
        }
        firstTime = false;

        if (timeUntilNextToggle >= 0) 
            timeUntilNextToggle -= evt.timeSinceLastFrame;

		static bool mWireframe = false;
		if (mInputDevice->isKeyDown(KC_G) && timeUntilNextToggle <= 0)
        {
			mWireframe = !mWireframe;
			if (mWireframe)
			{
				mCamera->setDetailLevel(SDL_WIREFRAME);
			}
			else
			{
				mCamera->setDetailLevel(SDL_SOLID);
			}
			timeUntilNextToggle = 0.5;

		}

        MaterialPtr mat = MaterialManager::getSingleton().getByName("Core/StatsBlockBorder/Up");
        mat->setDepthCheckEnabled(true);
        mat->setDepthWriteEnabled(true);

        for (int i = 0; i < NUM_TEST_NODES; ++i)
        {
            if (mTestNode[i] && rotate)
            mTestNode[i]->yaw(Degree(evt.timeSinceLastFrame * 15));
        }
        
        if (mAnimState && animate)
            mAnimState->addTime(evt.timeSinceLastFrame);
		std::vector<AnimationState*>::iterator animi;
		for (animi = mAnimStateList.begin(); animi != mAnimStateList.end(); ++animi)
		{
			(*animi)->addTime(evt.timeSinceLastFrame);
		}

        if (mInputDevice->isKeyDown(KC_R) && timeUntilNextToggle <= 0)
        {
            rotate = !rotate;
            timeUntilNextToggle = 0.5;
        }
        if (mInputDevice->isKeyDown(KC_1) && timeUntilNextToggle <= 0)
        {
            animate = !animate;
            timeUntilNextToggle = 0.5;
        }


        if (rayQuery)
        {
		    static Ray camRay;
		    static std::set<Entity*> lastEnts;
		    camRay.setOrigin(mCamera->getPosition());
		    camRay.setDirection(mCamera->getDirection());
		    rayQuery->setRay(camRay);

		    // Reset last set
		    for (std::set<Entity*>::iterator lasti = lastEnts.begin();
				    lasti != lastEnts.end(); ++lasti)
		    {
			    (*lasti)->setMaterialName("Examples/OgreLogo");
		    }
		    lastEnts.clear();
    		
    			
		    RaySceneQueryResult& results = rayQuery->execute();
		    for (RaySceneQueryResult::iterator mov = results.begin();
				    mov != results.end(); ++mov)
		    {
                if (mov->movable)
                {
			        if (mov->movable->getMovableType() == "Entity")
			        {
				        Entity* ent = static_cast<Entity*>(mov->movable);
				        lastEnts.insert(ent);
				        ent->setMaterialName("Examples/TextureEffect2");
        						
			        }
                }
		    }
        }

        if (intersectionQuery)
        {
            static std::set<Entity*> lastEnts;

            // Reset last set
            for (std::set<Entity*>::iterator lasti = lastEnts.begin();
                lasti != lastEnts.end(); ++lasti)
            {
                (*lasti)->setMaterialName("Examples/OgreLogo");
            }
            lastEnts.clear();


            IntersectionSceneQueryResult& results = intersectionQuery->execute();
            for (SceneQueryMovableIntersectionList::iterator mov = results.movables2movables.begin();
                mov != results.movables2movables.end(); ++mov)
            {
                SceneQueryMovableObjectPair& thepair = *mov;
                if (thepair.first->getMovableType() == "Entity")
                {
                    Entity* ent = static_cast<Entity*>(thepair.first);
                    lastEnts.insert(ent);
                    ent->setMaterialName("Examples/TextureEffect2");

                }
                if (thepair.second->getMovableType() == "Entity")
                {
                    Entity* ent = static_cast<Entity*>(thepair.second);
                    lastEnts.insert(ent);
                    ent->setMaterialName("Examples/TextureEffect2");

                }
            }
        }

        /*
		if (mInputDevice->isKeyDown(KC_V) && timeUntilNextToggle <= 0)
        {
            static bool isVP = false;
            if (!isVP)
            {
                skin->getTechnique(0)->getPass(0)->setVertexProgram("SimpleVP");
                skin->getTechnique(0)->getPass(0)->setVertexProgramParameters(vertParams);
                isVP = true;
            }
            else
            {
                skin->getTechnique(0)->getPass(0)->setVertexProgram("");
                isVP = false;
            }
			timeUntilNextToggle = 0.5;
        }
        */

		if (mInputDevice->isKeyDown(KC_P))
        {
            mTestNode[0]->yaw(Degree(-evt.timeSinceLastFrame * 30));
        }
		if (mInputDevice->isKeyDown(KC_O))
        {
            mTestNode[0]->yaw(Degree(evt.timeSinceLastFrame * 30));
        }
		if (mInputDevice->isKeyDown(KC_K))
        {
            mTestNode[0]->roll(Degree(-evt.timeSinceLastFrame * 30));
        }
		if (mInputDevice->isKeyDown(KC_L))
        {
            mTestNode[0]->roll(Degree(evt.timeSinceLastFrame * 30));
        }
		if (mInputDevice->isKeyDown(KC_U))
        {
            mTestNode[0]->translate(0,0,-evt.timeSinceLastFrame * 30);
        }
		if (mInputDevice->isKeyDown(KC_J))
        {
            mTestNode[0]->translate(0,0,evt.timeSinceLastFrame * 30);
        }
		if (mInputDevice->isKeyDown(KC_M))
        {
            mTestNode[0]->translate(0,evt.timeSinceLastFrame * 30, 0);
        }
		if (mInputDevice->isKeyDown(KC_N))
        {
            mTestNode[0]->translate(0,-evt.timeSinceLastFrame * 30, 0);
        }

        if (mInputDevice->isKeyDown(KC_0) && timeUntilNextToggle <= 0)
        {
            mAnimState->setEnabled(!mAnimState->getEnabled());
            timeUntilNextToggle = 0.5;
        }

        
        /** Hack to test frustum vols
        if (testCam)
        {
            // reposition the camera planes
            PlaneBoundedVolumeList volList = mLight->_getFrustumClipVolumes(testCam);

            PlaneBoundedVolume& vol = volList[1];
            for (int p = 0; p < 6; ++p)
            {
                Plane& pl = vol.planes[p];
                camPlaneNode[p]->setOrientation(Vector3::UNIT_Z.getRotationTo(pl.normal));
                camPlaneNode[p]->setPosition(0,0,0);
                camPlaneNode[p]->translate(0,0, pl.d, Node::TS_LOCAL);
            }

            vol.intersects(mEntity->getWorldBoundingBox());
        }
        */

        // Print camera details
        mWindow->setDebugText("P: " + StringConverter::toString(mCamera->getDerivedPosition()) + " " + 
            "O: " + StringConverter::toString(mCamera->getDerivedOrientation()));
        return ExampleFrameListener::frameStarted(evt) && ExampleFrameListener::frameEnded(evt);      


    }


};

class PlayPenApplication : public ExampleApplication
{
protected:
    RefractionTextureListener mRefractionListener;
    ReflectionTextureListener mReflectionListener;
public:
    PlayPenApplication() {
    
    
    }

    ~PlayPenApplication() 
    {
        if (frustum)
            delete frustum;
    }
protected:
    
    void chooseSceneManager(void)
    {
        mSceneMgr = mRoot->getSceneManager( ST_GENERIC );
        //mSceneMgr = mRoot->getSceneManager(ST_INTERIOR);

    }


    void createTestBugPlaneMesh3Streams(const String& testMeshName)
    {
        Plane plane(Vector3::UNIT_Z, 0);
        Real width = 258;
        Real height = 171;
        int xsegments = 50;
        int ysegments = 50;
        Real xTile = 1.0f;
        Real yTile = 1.0f;
        const Vector3& upVector = Vector3::UNIT_Y;
        MeshPtr pMesh = MeshManager::getSingleton().createManual(testMeshName, 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        SubMesh *pSub = pMesh->createSubMesh();

        // Set up vertex data
        // Use a single shared buffer
        pMesh->sharedVertexData = new VertexData();
        VertexData* vertexData = pMesh->sharedVertexData;
        // Set up Vertex Declaration
        VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
        // We always need positions
        vertexDecl->addElement(0, 0, VET_FLOAT3, VES_POSITION);
        vertexDecl->addElement(1, 0, VET_FLOAT3, VES_NORMAL);
        vertexDecl->addElement(2, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);

        vertexData->vertexCount = (xsegments + 1) * (ysegments + 1);

        // Allocate vertex buffers
        HardwareVertexBufferSharedPtr vbufpos = 
            HardwareBufferManager::getSingleton().
            createVertexBuffer(vertexDecl->getVertexSize(0), vertexData->vertexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
        HardwareVertexBufferSharedPtr vbufnorm = 
            HardwareBufferManager::getSingleton().
            createVertexBuffer(vertexDecl->getVertexSize(1), vertexData->vertexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
        HardwareVertexBufferSharedPtr vbuftex = 
            HardwareBufferManager::getSingleton().
            createVertexBuffer(vertexDecl->getVertexSize(2), vertexData->vertexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);

        // Set up the binding (one source only)
        VertexBufferBinding* binding = vertexData->vertexBufferBinding;
        binding->setBinding(0, vbufpos);
        binding->setBinding(1, vbufnorm);
        binding->setBinding(2, vbuftex);

        // Work out the transform required
        // Default orientation of plane is normal along +z, distance 0
        Matrix4 xlate, xform, rot;
        Matrix3 rot3;
        xlate = rot = Matrix4::IDENTITY;
        // Determine axes
        Vector3 zAxis, yAxis, xAxis;
        zAxis = plane.normal;
        zAxis.normalise();
        yAxis = upVector;
        yAxis.normalise();
        xAxis = yAxis.crossProduct(zAxis);

        rot3.FromAxes(xAxis, yAxis, zAxis);
        rot = rot3;

        // Set up standard xform from origin
        xlate.setTrans(plane.normal * -plane.d);

        // concatenate
        xform = xlate * rot;

        // Generate vertex data
        // Lock the whole buffer
        float* pPos = static_cast<float*>(
            vbufpos->lock(HardwareBuffer::HBL_DISCARD) );
        float* pNorm = static_cast<float*>(
            vbufnorm->lock(HardwareBuffer::HBL_DISCARD) );
        float* pTex = static_cast<float*>(
            vbuftex->lock(HardwareBuffer::HBL_DISCARD) );
        Real xSpace = width / xsegments;
        Real ySpace = height / ysegments;
        Real halfWidth = width / 2;
        Real halfHeight = height / 2;
        Real xTex = (1.0f * xTile) / xsegments;
        Real yTex = (1.0f * yTile) / ysegments;
        Vector3 vec;
        Vector3 min, max;
        Real maxSquaredLength;
        bool firstTime = true;

        for (int y = 0; y < ysegments + 1; ++y)
        {
            for (int x = 0; x < xsegments + 1; ++x)
            {
                // Work out centered on origin
                vec.x = (x * xSpace) - halfWidth;
                vec.y = (y * ySpace) - halfHeight;
                vec.z = 0.0f;
                // Transform by orientation and distance
                vec = xform * vec;
                // Assign to geometry
                *pPos++ = vec.x;
                *pPos++ = vec.y;
                *pPos++ = vec.z;

                // Build bounds as we go
                if (firstTime)
                {
                    min = vec;
                    max = vec;
                    maxSquaredLength = vec.squaredLength();
                    firstTime = false;
                }
                else
                {
                    min.makeFloor(vec);
                    max.makeCeil(vec);
                    maxSquaredLength = std::max(maxSquaredLength, vec.squaredLength());
                }

                // Default normal is along unit Z
                vec = Vector3::UNIT_Z;
                // Rotate
                vec = rot * vec;

                *pNorm++ = vec.x;
                *pNorm++ = vec.y;
                *pNorm++ = vec.z;

                *pTex++ = x * xTex;
                *pTex++ = 1 - (y * yTex);


            } // x
        } // y

        // Unlock
        vbufpos->unlock();
        vbufnorm->unlock();
        vbuftex->unlock();
        // Generate face list
        pSub->useSharedVertices = true;
        //tesselate2DMesh(pSub, xsegments + 1, ysegments + 1, false, indexBufferUsage, indexShadowBuffer);
        SubMesh* sm = pSub;
        int meshWidth = xsegments + 1;
        int meshHeight = ysegments + 1; 
        bool doubleSided = false;
        HardwareBuffer::Usage indexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY;
        bool indexShadowBuffer = true;
        // The mesh is built, just make a list of indexes to spit out the triangles
        int vInc, uInc, v, u, iterations;
        int vCount, uCount;

        if (doubleSided)
        {
            iterations = 2;
            vInc = 1;
            v = 0; // Start with front
        }
        else
        {
            iterations = 1;
            vInc = 1;
            v = 0;
        }

        // Allocate memory for faces
        // Num faces, width*height*2 (2 tris per square), index count is * 3 on top
        sm->indexData->indexCount = (meshWidth-1) * (meshHeight-1) * 2 * iterations * 3;
        sm->indexData->indexBuffer = HardwareBufferManager::getSingleton().
            createIndexBuffer(HardwareIndexBuffer::IT_16BIT,
            sm->indexData->indexCount, indexBufferUsage, indexShadowBuffer);

        int v1, v2, v3;
        //bool firstTri = true;
        HardwareIndexBufferSharedPtr ibuf = sm->indexData->indexBuffer;
        // Lock the whole buffer
        unsigned short* pIndexes = static_cast<unsigned short*>(
            ibuf->lock(HardwareBuffer::HBL_DISCARD) );

        while (iterations--)
        {
            // Make tris in a zigzag pattern (compatible with strips)
            u = 0;
            uInc = 1; // Start with moving +u

            vCount = meshHeight - 1;
            while (vCount--)
            {
                uCount = meshWidth - 1;
                while (uCount--)
                {
                    // First Tri in cell
                    // -----------------
                    v1 = ((v + vInc) * meshWidth) + u;
                    v2 = (v * meshWidth) + u;
                    v3 = ((v + vInc) * meshWidth) + (u + uInc);
                    // Output indexes
                    *pIndexes++ = v1;
                    *pIndexes++ = v2;
                    *pIndexes++ = v3;
                    // Second Tri in cell
                    // ------------------
                    v1 = ((v + vInc) * meshWidth) + (u + uInc);
                    v2 = (v * meshWidth) + u;
                    v3 = (v * meshWidth) + (u + uInc);
                    // Output indexes
                    *pIndexes++ = v1;
                    *pIndexes++ = v2;
                    *pIndexes++ = v3;

                    // Next column
                    u += uInc;
                }
                // Next row
                v += vInc;
                u = 0;


            }

            // Reverse vInc for double sided
            v = meshHeight - 1;
            vInc = -vInc;

        }
        // Unlock
        ibuf->unlock();

        //pMesh->_updateBounds();
        pMesh->_setBounds(AxisAlignedBox(min, max));
        pMesh->_setBoundingSphereRadius(Math::Sqrt(maxSquaredLength));
        // load
        pMesh->load();
        pMesh->touch();
    }

    void stressTestStaticGeometry(void)
    {

		// Set ambient light
		mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

		// Create a point light
		Light* l = mSceneMgr->createLight("MainLight");
		l->setType(Light::LT_DIRECTIONAL);
		Vector3 dir(1, -1, -1.5);
		dir.normalise();
		l->setDirection(dir);
		l->setDiffuseColour(1.0, 0.7, 0.0);


		Plane plane;
		plane.normal = Vector3::UNIT_Y;
		plane.d = 0;
		MeshManager::getSingleton().createPlane("Myplane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
			4500,4500,10,10,true,1,5,5,Vector3::UNIT_Z);
		Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
		pPlaneEnt->setMaterialName("Examples/GrassFloor");
		pPlaneEnt->setCastShadows(false);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

		Vector3 min(-2000,0,-2000);
		Vector3 max(2000,0,2000);


		Entity* e = mSceneMgr->createEntity("1", "ogrehead.mesh");
		StaticGeometry* s = 0;

		unsigned int count = 10;
		while(count--)
		{
			if(s) mSceneMgr->removeStaticGeometry(s);
			s = mSceneMgr->createStaticGeometry("bing");

			s->addEntity(e, Vector3(100, 100, 100));

			s->build();
		}


		//s->setRenderingDistance(1000);
		//s->dump("static.txt");
		//mSceneMgr->showBoundingBoxes(true);
		mCamera->setLodBias(0.5);
        

    }

	void testReflectedBillboards()
	{
		// Set ambient light
		mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

		// Create a light
		Light* l = mSceneMgr->createLight("MainLight");
		l->setType(Light::LT_DIRECTIONAL);
		Vector3 dir(0.5, -1, 0);
		dir.normalise();
		l->setDirection(dir);
		l->setDiffuseColour(1.0f, 1.0f, 0.8f);
		l->setSpecularColour(1.0f, 1.0f, 1.0f);


		// Create a prefab plane
		plane.d = 0;
		plane.normal = Vector3::UNIT_Y;
		MeshManager::getSingleton().createPlane("ReflectionPlane", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
			plane, 2000, 2000, 
			1, 1, true, 1, 1, 1, Vector3::UNIT_Z);
		Entity* planeEnt = mSceneMgr->createEntity( "Plane", "ReflectionPlane" );

		// Attach the rtt entity to the root of the scene
		SceneNode* rootNode = mSceneMgr->getRootSceneNode();
		SceneNode* planeNode = rootNode->createChildSceneNode();

		// Attach both the plane entity, and the plane definition
		planeNode->attachObject(planeEnt);

		RenderTexture* rttTex = mRoot->getRenderSystem()->createRenderTexture( "RttTex", 512, 512, TEX_TYPE_2D, PF_R8G8B8 );
		{
			reflectCam = mSceneMgr->createCamera("ReflectCam");
			reflectCam->setNearClipDistance(mCamera->getNearClipDistance());
			reflectCam->setFarClipDistance(mCamera->getFarClipDistance());
			reflectCam->setAspectRatio(
				(Real)mWindow->getViewport(0)->getActualWidth() / 
				(Real)mWindow->getViewport(0)->getActualHeight());

			Viewport *v = rttTex->addViewport( reflectCam );
			v->setClearEveryFrame( true );
			v->setBackgroundColour( ColourValue::Black );

			MaterialPtr mat = MaterialManager::getSingleton().create("RttMat",
				ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			TextureUnitState* t = mat->getTechnique(0)->getPass(0)->createTextureUnitState("RustedMetal.jpg");
			t = mat->getTechnique(0)->getPass(0)->createTextureUnitState("RttTex");
			// Blend with base texture
			t->setColourOperationEx(LBX_BLEND_MANUAL, LBS_TEXTURE, LBS_CURRENT, ColourValue::White, 
				ColourValue::White, 0.25);
			t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
			t->setProjectiveTexturing(true, reflectCam);

			// set up linked reflection
			reflectCam->enableReflection(plane);
			// Also clip
			reflectCam->enableCustomNearClipPlane(plane);
		}

		// Give the plane a texture
		planeEnt->setMaterialName("RttMat");


		// point billboards
		ParticleSystem* pSys2 = ParticleSystemManager::getSingleton().createSystem("fountain1", 
			"Examples/Smoke");
		// Point the fountain at an angle
		SceneNode* fNode = static_cast<SceneNode*>(rootNode->createChild());
		fNode->attachObject(pSys2);

		// oriented_self billboards
		ParticleSystem* pSys3 = ParticleSystemManager::getSingleton().createSystem("fountain2", 
			"Examples/PurpleFountain");
		// Point the fountain at an angle
		fNode = rootNode->createChildSceneNode();
		fNode->translate(-200,-100,0);
		fNode->rotate(Vector3::UNIT_Z, Degree(-20));
		fNode->attachObject(pSys3);


		
		// oriented_common billboards
		ParticleSystem* pSys4 = ParticleSystemManager::getSingleton().createSystem("rain", 
			"Examples/Rain");
		SceneNode* rNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		rNode->translate(0,1000,0);
		rNode->attachObject(pSys4);
		// Fast-forward the rain so it looks more natural
		pSys4->fastForward(5);


		mCamera->setPosition(-50, 100, 500);
		mCamera->lookAt(0,0,0);
	}

	void testBug()
	{
		SceneNode* n = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		SceneNode* n2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		n->setAutoTracking(true, n2);

		mSceneMgr->destroySceneNode(n->getName());
		mSceneMgr->destroySceneNode(n2->getName());


	}

	void testTransparencyMipMaps()
	{
		MaterialPtr mat = MaterialManager::getSingleton().create("test", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		// known png with alpha
		Pass* pass = mat->getTechnique(0)->getPass(0);
		pass->createTextureUnitState("ogretext.png");
		pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
		// alpha blend
		pass->setDepthWriteEnabled(false);

		// alpha reject
		//pass->setDepthWriteEnabled(true);
		//pass->setAlphaRejectSettings(CMPF_LESS, 128);

		// Define a floor plane mesh
		Plane p;
		p.normal = Vector3::UNIT_Y;
		p.d = 200;
		MeshManager::getSingleton().createPlane("FloorPlane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			p,2000,2000,1,1,true,1,5,5,Vector3::UNIT_Z);

		// Create an entity (the floor)
		Entity* ent = mSceneMgr->createEntity("floor", "FloorPlane");
		ent->setMaterialName("test");
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);

		mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);
		mSceneMgr->setAmbientLight(ColourValue::White);


		{
		
		Real alphaLevel = 0.5f;
		MaterialPtr alphamat = MaterialManager::getSingleton().create("testy", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Pass* pass = alphamat->getTechnique(0)->getPass(0);
		pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
		pass->setDepthWriteEnabled(false);
		TextureUnitState* t = pass->createTextureUnitState();
		t->setAlphaOperation(LBX_SOURCE1, LBS_MANUAL, LBS_CURRENT, alphaLevel);

		ent = mSceneMgr->createEntity("asd", "ogrehead.mesh");
		ent->setMaterialName("testy");
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);

		}
		
	}

    void testCthNewBlending(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-Vector3::UNIT_Y);

        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        //l->setPosition(20,80,50);


        Entity *ent = mSceneMgr->createEntity("head", "ogrehead_2.mesh");

        // Add entity to the root scene node
        mRootNode = mSceneMgr->getRootSceneNode();
        static_cast<SceneNode*>(mRootNode->createChild())->attachObject(ent);

        mTestNode[0] = static_cast<SceneNode*>(
            mRootNode->createChild("TestNode", Vector3(100,0,0)));
        ent = mSceneMgr->createEntity("head2", "ogrehead.mesh");
        mTestNode[0]->attachObject(ent);

        mTestNode[0]->attachObject(l);
        

        // Create a skydome
        mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

        // Define a floor plane mesh
        Plane p;
        p.normal = Vector3::UNIT_Y;
        p.d = 200;
        MeshManager::getSingleton().createPlane("FloorPlane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			p,2000,2000,1,1,true,1,5,5,Vector3::UNIT_Z);

        // Create an entity (the floor)
        ent = mSceneMgr->createEntity("floor", "FloorPlane");
        ent->setMaterialName("Block/Material1");

        mRootNode->attachObject(ent);

        mCamera->setPosition(0,0,200);
        mCamera->setDirection(0,0,-1);

        //mSceneMgr->setDisplaySceneNodes(true);

    }

    void testMatrices(void)
    {
        mCamera->setPosition(0,0,500);
        mCamera->lookAt(0,0,0);
        const Matrix4& view = mCamera->getViewMatrix();
        const Matrix4& proj = mCamera->getProjectionMatrix();

        Matrix4 viewproj = proj * view;

        Vector3 point3d(100,100,0);

        Vector3 projPoint = viewproj * point3d;

        point3d = Vector3(100,100,400);
        projPoint = viewproj * point3d;
    }
    void testBasicPlane()
    {
        /*
        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);
        */

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-Vector3::UNIT_Y);
        Entity *ent;

        // Define a floor plane mesh
        Plane p;
        p.normal = Vector3::UNIT_Y;
        p.d = 200;
        MeshManager::getSingleton().createPlane("FloorPlane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			p,2000,2000,1,1,true,1,5,5,Vector3::UNIT_Z);

        // Create an entity (the floor)
        ent = mSceneMgr->createEntity("floor", "FloorPlane");
        ent->setMaterialName("Examples/RustySteel");

        mSceneMgr->getRootSceneNode()->attachObject(ent);

        Entity* sphereEnt = mSceneMgr->createEntity("ogre", "ogrehead.mesh");
		
		mRootNode = mSceneMgr->getRootSceneNode();
        SceneNode* node = mSceneMgr->createSceneNode();
        node->attachObject(sphereEnt);
        mRootNode->addChild(node);

    }

    void testAlpha()
    {
        /*
        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);
        */

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-Vector3::UNIT_Y);
        Entity *ent;

        // Define a floor plane mesh
        Plane p;
        p.normal = Vector3::UNIT_Y;
        p.d = 200;
        MeshManager::getSingleton().createPlane("FloorPlane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			p,2000,2000,1,1,true,1,5,5,Vector3::UNIT_Z);

        p.normal = Vector3::UNIT_Z;
        p.d = 200;
        MeshManager::getSingleton().createPlane("WallPlane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			p,2000,2000,1,1,true,1,5,5,Vector3::UNIT_Y);

        // Create an entity (the floor)
        ent = mSceneMgr->createEntity("floor", "FloorPlane");
        ent->setMaterialName("Core/StatsBlockBorder/Up");

        mSceneMgr->getRootSceneNode()->attachObject(ent);

        ent = mSceneMgr->createEntity("wall", "WallPlane");
        ent->setMaterialName("Core/StatsBlockBorder/Up");

        mSceneMgr->getRootSceneNode()->attachObject(ent);


        Entity* sphereEnt = mSceneMgr->createEntity("ogre", "ogrehead.mesh");
		
		mRootNode = mSceneMgr->getRootSceneNode();
        SceneNode* node = mSceneMgr->createSceneNode();
        node->attachObject(sphereEnt);
        mRootNode->addChild(node);

        mSceneMgr->showBoundingBoxes(true);

    }
    void testBsp()
    {
        // Load Quake3 locations from a file
        ConfigFile cf;

        cf.load("quake3settings.cfg");

        String quakePk3 = cf.getSetting("Pak0Location");
        String quakeLevel = cf.getSetting("Map");

		ResourceGroupManager::getSingleton().addResourceLocation(quakePk3, "Zip");


        // Load world geometry
        mSceneMgr->setWorldGeometry(quakeLevel);

        // modify camera for close work
        mCamera->setNearClipDistance(4);
        mCamera->setFarClipDistance(4000);

        // Also change position, and set Quake-type orientation
        // Get random player start point
        ViewPoint vp = mSceneMgr->getSuggestedViewpoint(true);
        mCamera->setPosition(vp.position);
        mCamera->pitch(Degree(90)); // Quake uses X/Y horizon, Z up
        mCamera->rotate(vp.orientation);
        // Don't yaw along variable axis, causes leaning
        mCamera->setFixedYawAxis(true, Vector3::UNIT_Z);

    }

    void testAnimation()
    {
        Light* l = mSceneMgr->createLight("MainLight");
        l->setPosition(200,110,0);
        l->setType(Light::LT_POINT);
        Entity *ent;

        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.4));

        // Create an entity (the plant)
        ent = mSceneMgr->createEntity("1", "limo.mesh");

        SceneNode* node = static_cast<SceneNode*>(mSceneMgr->getRootSceneNode()->createChild(Vector3(-50,0,0)));
        node->attachObject(ent);
        node->scale(2,2,2);

        mAnimState = ent->getAnimationState("SteerLeftOn");
        mAnimState->setEnabled(true);

        mWindow->getViewport(0)->setBackgroundColour(ColourValue(1,0,0));


    }

    void testGpuPrograms(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-Vector3::UNIT_Y);

        Entity* pEnt = mSceneMgr->createEntity( "1", "robot.mesh" );
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pEnt);

        mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
            
        pEnt = mSceneMgr->createEntity( "2", "ogrehead.mesh" );
        mTestNode[0]->attachObject( pEnt );
        mTestNode[0]->translate(100,0,0);

        // Rejig the ogre skin material
        // Load the programs first

        pEnt->getSubEntity(1)->setMaterialName("SimpleTest");

    }
    void testProjection(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-Vector3::UNIT_Y);

        Entity* pEnt;
        //pEnt = mSceneMgr->createEntity( "1", "knot.mesh" );
        //mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-30,0,-50))->attachObject(pEnt);
        //pEnt->setMaterialName("Examples/OgreLogo");

        // Define a floor plane mesh
        Plane p;
        p.normal = Vector3::UNIT_Z;
        p.d = 200;
        MeshManager::getSingleton().createPlane("WallPlane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			p,1500,1500,1,1,true,1,5,5,Vector3::UNIT_Y);
        pEnt = mSceneMgr->createEntity( "5", "WallPlane" );
        pEnt->setMaterialName("Examples/OgreLogo");
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pEnt);


        mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
            
        //pEnt = mSceneMgr->createEntity( "2", "ogrehead.mesh" );
        //mTestNode[0]->attachObject( pEnt );
        mTestNode[0]->translate(0, 0, 750);

        frustum = new Frustum();
        //frustum->setVisible(true);
        frustum->setFarClipDistance(5000);
        frustum->setNearClipDistance(200);
        frustum->setAspectRatio(1);
        frustum->setProjectionType(PT_ORTHOGRAPHIC);
        mTestNode[0]->attachObject(frustum);

        // Hook the frustum up to the material
        MaterialPtr mat = MaterialManager::getSingleton().getByName("Examples/OgreLogo");
        TextureUnitState *t = mat->getTechnique(0)->getPass(0)->getTextureUnitState(0);
        t->setProjectiveTexturing(true, frustum);
        //t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);

    }

    void testMultiViewports(void)
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-Vector3::UNIT_Y);

        Entity* pEnt = mSceneMgr->createEntity( "1", "knot.mesh" );
        mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-30,0,-50))->attachObject(pEnt);

        mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
            
        pEnt = mSceneMgr->createEntity( "2", "ogrehead.mesh" );
        mTestNode[0]->attachObject( pEnt );
        mTestNode[0]->translate(0, 0, 200);

        frustum = new Frustum();
        //frustum->setVisible(true);
        frustum->setFarClipDistance(5000);
        frustum->setNearClipDistance(100);
        mTestNode[0]->attachObject(frustum);

        Viewport* vp = mRoot->getAutoCreatedWindow()->addViewport(mCamera, 1, 0.5, 0.5, 0.5, 0.5);
        vp->setOverlaysEnabled(false);
        vp->setBackgroundColour(ColourValue(1,0,0));

    }


    // Just override the mandatory create scene method
    void testSceneNodeTracking(void)
    {

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

        // Create a skydome
        mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        Entity *ent;

        // Define a floor plane mesh
        Plane p;
        p.normal = Vector3::UNIT_Y;
        p.d = 200;
        MeshManager::getSingleton().createPlane("FloorPlane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			p,200000,200000,20,20,true,1,50,50,Vector3::UNIT_Z);

        // Create an entity (the floor)
        ent = mSceneMgr->createEntity("floor", "FloorPlane");
        ent->setMaterialName("Examples/RustySteel");
        // Attach to child of root node, better for culling (otherwise bounds are the combination of the 2)
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);

        // Add a head, give it it's own node
        SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        ent = mSceneMgr->createEntity("head", "ogrehead.mesh");
        headNode->attachObject(ent);

        // Add another head, give it it's own node
        SceneNode* headNode2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        ent = mSceneMgr->createEntity("head2", "ogrehead.mesh");
        headNode2->attachObject(ent);

        // Make sure the head node tracks the root
        headNode->setAutoTracking(true, headNode2, Vector3::UNIT_Z);
        //headNode->setFixedYawAxis(true);

        // Create the camera node & attach camera
        //SceneNode* camNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        //camNode->attachObject(mCamera);

        // set up spline animation of node
        Animation* anim = mSceneMgr->createAnimation("CameraTrack", 10);
        // Spline it for nice curves
        anim->setInterpolationMode(Animation::IM_SPLINE);
        // Create a track to animate the head's node
        AnimationTrack* track = anim->createTrack(0, headNode);
        // Setup keyframes
        KeyFrame* key = track->createKeyFrame(0); // startposition
        key = track->createKeyFrame(2.5);
        key->setTranslate(Vector3(500,500,-1000));
        key = track->createKeyFrame(5);
        key->setTranslate(Vector3(-1500,1000,-600));
        key = track->createKeyFrame(7.5);
        key->setTranslate(Vector3(0,-100,0));
        key = track->createKeyFrame(10);
        key->setTranslate(Vector3(0,0,0));
        // Create a track to animate the second head's node
        track = anim->createTrack(1, headNode2);
        // Setup keyframes
        key = track->createKeyFrame(0); // startposition
        key = track->createKeyFrame(2.5);
        key->setTranslate(Vector3(-500,600,-100));
        key = track->createKeyFrame(5);
        key->setTranslate(Vector3(800,200,-600));
        key = track->createKeyFrame(7.5);
        key->setTranslate(Vector3(200,-1000,0));
        key = track->createKeyFrame(10);
        key->setTranslate(Vector3(30,70,110));
        // Create a new animation state to track this
        mAnimState = mSceneMgr->createAnimationState("CameraTrack");
        mAnimState->setEnabled(true);

        // Put in a bit of fog for the hell of it
        mSceneMgr->setFog(FOG_EXP, ColourValue::White, 0.0002);

    }



    void testDistortion(void)
    {
        theCam = mCamera;
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-Vector3::UNIT_Y);

        Entity* pEnt;

        RenderTexture* rttTex = mRoot->getRenderSystem()->createRenderTexture( "Refraction", 512, 512 );
        {
            Viewport *v = rttTex->addViewport( mCamera );
            MaterialPtr mat = MaterialManager::getSingleton().getByName("Examples/FresnelReflectionRefraction");
            mat->getTechnique(0)->getPass(0)->getTextureUnitState(2)->setTextureName("Refraction");
            v->setOverlaysEnabled(false);
            rttTex->addListener(&mRefractionListener);
        }

        rttTex = mRoot->getRenderSystem()->createRenderTexture( "Reflection", 512, 512 );
        {
            Viewport *v = rttTex->addViewport( mCamera );
            MaterialPtr mat = MaterialManager::getSingleton().getByName("Examples/FresnelReflectionRefraction");
            mat->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setTextureName("Reflection");
            v->setOverlaysEnabled(false);
            rttTex->addListener(&mReflectionListener);
        }
        // Define a floor plane mesh
        Plane p;
        p.normal = Vector3::UNIT_Y;
        p.d = 100;
        MeshManager::getSingleton().createPlane("WallPlane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			p,1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
        pPlaneEnt = mSceneMgr->createEntity( "5", "WallPlane" );
        pPlaneEnt->setMaterialName("Examples/FresnelReflectionRefraction");
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

        
        mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");

        mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        int i;
        for (i = 0; i < 10; ++i)
        {
            pEnt = mSceneMgr->createEntity( "ogre" + StringConverter::toString(i), "ogrehead.mesh" );
            mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(i*100 - 500, -75, 0))->attachObject(pEnt);
            pEnt = mSceneMgr->createEntity( "knot" + StringConverter::toString(i), "knot.mesh" );
            mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(i*100 - 500, 140, 0))->attachObject(pEnt);
        }



    }


    void testEdgeBuilderSingleIndexBufSingleVertexBuf()
    {
        /* This tests the edge builders ability to find shared edges in the simple case
           of a single index buffer referencing a single vertex buffer
        */
        VertexData vd;
        IndexData id;
        // Test pyramid
        vd.vertexCount = 4;
        vd.vertexStart = 0;
        vd.vertexDeclaration = HardwareBufferManager::getSingleton().createVertexDeclaration();
        vd.vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
        HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float)*3, 4, HardwareBuffer::HBU_STATIC,true);
        vd.vertexBufferBinding->setBinding(0, vbuf);
        float* pFloat = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
        *pFloat++ = 0  ; *pFloat++ = 0  ; *pFloat++ = 0  ;
        *pFloat++ = 50 ; *pFloat++ = 0  ; *pFloat++ = 0  ;
        *pFloat++ = 0  ; *pFloat++ = 100; *pFloat++ = 0  ;
        *pFloat++ = 0  ; *pFloat++ = 0  ; *pFloat++ = -50;
        vbuf->unlock();
            
        id.indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, 12, HardwareBuffer::HBU_STATIC, true);
        id.indexCount = 12;
        id.indexStart = 0;
        unsigned short* pIdx = static_cast<unsigned short*>(id.indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
        *pIdx++ = 0; *pIdx++ = 1; *pIdx++ = 2;
        *pIdx++ = 0; *pIdx++ = 2; *pIdx++ = 3;
        *pIdx++ = 1; *pIdx++ = 3; *pIdx++ = 2;
        *pIdx++ = 0; *pIdx++ = 3; *pIdx++ = 1;
        id.indexBuffer->unlock();

        EdgeListBuilder edgeBuilder;
        edgeBuilder.addVertexData(&vd);
        edgeBuilder.addIndexData(&id);
        EdgeData* edgeData = edgeBuilder.build();

        edgeData->log(LogManager::getSingleton().getDefaultLog());

        delete edgeData;


    }

    void testEdgeBuilderMultiIndexBufSingleVertexBuf()
    {
        /* This tests the edge builders ability to find shared edges when there are
           multiple index sets (submeshes) using a single vertex buffer.
        */
        VertexData vd;
        IndexData id[4];
        // Test pyramid
        vd.vertexCount = 4;
        vd.vertexStart = 0;
        vd.vertexDeclaration = HardwareBufferManager::getSingleton().createVertexDeclaration();
        vd.vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
        HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float)*3, 4, HardwareBuffer::HBU_STATIC,true);
        vd.vertexBufferBinding->setBinding(0, vbuf);
        float* pFloat = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
        *pFloat++ = 0  ; *pFloat++ = 0  ; *pFloat++ = 0  ;
        *pFloat++ = 50 ; *pFloat++ = 0  ; *pFloat++ = 0  ;
        *pFloat++ = 0  ; *pFloat++ = 100; *pFloat++ = 0  ;
        *pFloat++ = 0  ; *pFloat++ = 0  ; *pFloat++ = -50;
        vbuf->unlock();
            
        id[0].indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, 3, HardwareBuffer::HBU_STATIC, true);
        id[0].indexCount = 3;
        id[0].indexStart = 0;
        unsigned short* pIdx = static_cast<unsigned short*>(id[0].indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
        *pIdx++ = 0; *pIdx++ = 1; *pIdx++ = 2;
        id[0].indexBuffer->unlock();

        id[1].indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, 3, HardwareBuffer::HBU_STATIC, true);
        id[1].indexCount = 3;
        id[1].indexStart = 0;
        pIdx = static_cast<unsigned short*>(id[1].indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
        *pIdx++ = 0; *pIdx++ = 2; *pIdx++ = 3;
        id[1].indexBuffer->unlock();

        id[2].indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, 3, HardwareBuffer::HBU_STATIC, true);
        id[2].indexCount = 3;
        id[2].indexStart = 0;
        pIdx = static_cast<unsigned short*>(id[2].indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
        *pIdx++ = 1; *pIdx++ = 3; *pIdx++ = 2;
        id[2].indexBuffer->unlock();

        id[3].indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, 3, HardwareBuffer::HBU_STATIC, true);
        id[3].indexCount = 3;
        id[3].indexStart = 0;
        pIdx = static_cast<unsigned short*>(id[3].indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
        *pIdx++ = 0; *pIdx++ = 3; *pIdx++ = 1;
        id[3].indexBuffer->unlock();

        EdgeListBuilder edgeBuilder;
        edgeBuilder.addVertexData(&vd);
        edgeBuilder.addIndexData(&id[0]);
        edgeBuilder.addIndexData(&id[1]);
        edgeBuilder.addIndexData(&id[2]);
        edgeBuilder.addIndexData(&id[3]);
        EdgeData* edgeData = edgeBuilder.build();

        edgeData->log(LogManager::getSingleton().getDefaultLog());

        delete edgeData;


    }


    void testEdgeBuilderMultiIndexBufMultiVertexBuf()
    {
        /* This tests the edge builders ability to find shared edges when there are
           both multiple index sets (submeshes) each using a different vertex buffer
           (not using shared geoemtry). 
        */

        VertexData vd[4];
        IndexData id[4];
        // Test pyramid
        vd[0].vertexCount = 3;
        vd[0].vertexStart = 0;
        vd[0].vertexDeclaration = HardwareBufferManager::getSingleton().createVertexDeclaration();
        vd[0].vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
        HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float)*3, 3, HardwareBuffer::HBU_STATIC,true);
        vd[0].vertexBufferBinding->setBinding(0, vbuf);
        float* pFloat = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
        *pFloat++ = 0  ; *pFloat++ = 0  ; *pFloat++ = 0  ;
        *pFloat++ = 50 ; *pFloat++ = 0  ; *pFloat++ = 0  ;
        *pFloat++ = 0  ; *pFloat++ = 100; *pFloat++ = 0  ;
        vbuf->unlock();

        vd[1].vertexCount = 3;
        vd[1].vertexStart = 0;
        vd[1].vertexDeclaration = HardwareBufferManager::getSingleton().createVertexDeclaration();
        vd[1].vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float)*3, 3, HardwareBuffer::HBU_STATIC,true);
        vd[1].vertexBufferBinding->setBinding(0, vbuf);
        pFloat = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
        *pFloat++ = 0  ; *pFloat++ = 0  ; *pFloat++ = 0  ;
        *pFloat++ = 0  ; *pFloat++ = 100; *pFloat++ = 0  ;
        *pFloat++ = 0  ; *pFloat++ = 0  ; *pFloat++ = -50;
        vbuf->unlock();

        vd[2].vertexCount = 3;
        vd[2].vertexStart = 0;
        vd[2].vertexDeclaration = HardwareBufferManager::getSingleton().createVertexDeclaration();
        vd[2].vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float)*3, 3, HardwareBuffer::HBU_STATIC,true);
        vd[2].vertexBufferBinding->setBinding(0, vbuf);
        pFloat = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
        *pFloat++ = 50 ; *pFloat++ = 0  ; *pFloat++ = 0  ;
        *pFloat++ = 0  ; *pFloat++ = 100; *pFloat++ = 0  ;
        *pFloat++ = 0  ; *pFloat++ = 0  ; *pFloat++ = -50;
        vbuf->unlock();
            
        vd[3].vertexCount = 3;
        vd[3].vertexStart = 0;
        vd[3].vertexDeclaration = HardwareBufferManager::getSingleton().createVertexDeclaration();
        vd[3].vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float)*3, 3, HardwareBuffer::HBU_STATIC,true);
        vd[3].vertexBufferBinding->setBinding(0, vbuf);
        pFloat = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
        *pFloat++ = 0  ; *pFloat++ = 0  ; *pFloat++ = 0  ;
        *pFloat++ = 50 ; *pFloat++ = 0  ; *pFloat++ = 0  ;
        *pFloat++ = 0  ; *pFloat++ = 0  ; *pFloat++ = -50;
        vbuf->unlock();

        id[0].indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, 3, HardwareBuffer::HBU_STATIC, true);
        id[0].indexCount = 3;
        id[0].indexStart = 0;
        unsigned short* pIdx = static_cast<unsigned short*>(id[0].indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
        *pIdx++ = 0; *pIdx++ = 1; *pIdx++ = 2;
        id[0].indexBuffer->unlock();

        id[1].indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, 3, HardwareBuffer::HBU_STATIC, true);
        id[1].indexCount = 3;
        id[1].indexStart = 0;
        pIdx = static_cast<unsigned short*>(id[1].indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
        *pIdx++ = 0; *pIdx++ = 1; *pIdx++ = 2;
        id[1].indexBuffer->unlock();

        id[2].indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, 3, HardwareBuffer::HBU_STATIC, true);
        id[2].indexCount = 3;
        id[2].indexStart = 0;
        pIdx = static_cast<unsigned short*>(id[2].indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
        *pIdx++ = 0; *pIdx++ = 2; *pIdx++ = 1;
        id[2].indexBuffer->unlock();

        id[3].indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, 3, HardwareBuffer::HBU_STATIC, true);
        id[3].indexCount = 3;
        id[3].indexStart = 0;
        pIdx = static_cast<unsigned short*>(id[3].indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
        *pIdx++ = 0; *pIdx++ = 2; *pIdx++ = 1;
        id[3].indexBuffer->unlock();

        EdgeListBuilder edgeBuilder;
        edgeBuilder.addVertexData(&vd[0]);
        edgeBuilder.addVertexData(&vd[1]);
        edgeBuilder.addVertexData(&vd[2]);
        edgeBuilder.addVertexData(&vd[3]);
        edgeBuilder.addIndexData(&id[0], 0);
        edgeBuilder.addIndexData(&id[1], 1);
        edgeBuilder.addIndexData(&id[2], 2);
        edgeBuilder.addIndexData(&id[3], 3);
        EdgeData* edgeData = edgeBuilder.build();

        edgeData->log(LogManager::getSingleton().getDefaultLog());

        delete edgeData;


    }

    void testSkeletalAnimation()
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
        //mWindow->getViewport(0)->setBackgroundColour(ColourValue::White);



		//Entity *ent = mSceneMgr->createEntity("robot", "scuttlepod.mesh");
		//mAnimState = ent->getAnimationState("Walk");
        //Entity *ent = mSceneMgr->createEntity("robot", "jaiqua.mesh");
		//mAnimState = ent->getAnimationState("Jaiqua_walk_Preset");
		Entity *ent = mSceneMgr->createEntity("robot", "Golden.mesh");
		mAnimState = ent->getAnimationState("TestAnimationThatDoesntWork");

		mAnimState->setEnabled(true);

        // Uncomment the below to test software skinning
        //ent->setMaterialName("Examples/Rocky");
        // Add entity to the scene node
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);

        // Give it a little ambience with lights
        Light* l;
        l = mSceneMgr->createLight("Light");
		Vector3 dir(-1,-1, 0);
		dir.normalise();
		l->setType(Light::LT_DIRECTIONAL);
		l->setDirection(dir);

        l = mSceneMgr->createLight("GreenLight");
        l->setPosition(0,0,-100);
        l->setDiffuseColour(0.5, 1.0, 0.5);

        // Position the camera
        mCamera->setPosition(100,50,100);
        mCamera->lookAt(-50,50,0);

        // Report whether hardware skinning is enabled or not
        Technique* t = ent->getSubEntity(0)->getMaterial()->getBestTechnique();
        Pass* p = t->getPass(0);
        if (p->hasVertexProgram() && 
            p->getVertexProgram()->isSkeletalAnimationIncluded())
        {
            mWindow->setDebugText("Hardware skinning is enabled");
        }
        else
        {
            mWindow->setDebugText("Software skinning is enabled");
        }


    }


    void testPrepareShadowVolume(void)
    {

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-Vector3::UNIT_Y);

        mTestNode[0] = (SceneNode*)mSceneMgr->getRootSceneNode()->createChild();
            
        Entity* pEnt = mSceneMgr->createEntity( "1", "ogrehead.mesh" );
        mTestNode[0]->attachObject( pEnt );

        pEnt->getMesh()->prepareForShadowVolume();

    }

    void testWindowedViewportMode()
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-Vector3::UNIT_Y);

        mTestNode[0] = (SceneNode*)mSceneMgr->getRootSceneNode()->createChild();

        Entity* pEnt = mSceneMgr->createEntity( "1", "ogrehead.mesh" );
        mTestNode[0]->attachObject( pEnt );

        mCamera->setWindow(0,0,0.5,0.5);

    }
    void testSubEntityVisibility()
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-Vector3::UNIT_Y);

        mTestNode[0] = (SceneNode*)mSceneMgr->getRootSceneNode()->createChild();

        Entity* pEnt = mSceneMgr->createEntity( "1", "ogrehead.mesh" );
        mTestNode[0]->attachObject( pEnt );

        pEnt->getSubEntity(1)->setVisible(false);


    }

    void testAttachObjectsToBones()
    {
        Entity *ent;
        for (int i = 0; i < 5; ++i)
        {
            ent = mSceneMgr->createEntity("robot" + StringConverter::toString(i), "robot.mesh");
            Entity* ent2 = mSceneMgr->createEntity("plane" + StringConverter::toString(i), "razor.mesh");
            ent->attachObjectToBone("Joint8", ent2);
            // Add entity to the scene node
            mSceneMgr->getRootSceneNode()->createChildSceneNode(
                Vector3(0,0,(i*50)-(5*50/2)))->attachObject(ent);
            if (i==4)
            {
                ent->getParentNode()->yaw(Degree(45));
            }

			ent->getParentNode()->scale(Vector3(0.5, 0.5, 0.5));
        }
        mAnimState = ent->getAnimationState("Walk");
        mAnimState->setEnabled(true);



        // Give it a little ambience with lights
        Light* l;
        l = mSceneMgr->createLight("BlueLight");
        l->setPosition(-200,-80,-100);
        l->setDiffuseColour(0.5, 0.5, 1.0);

        l = mSceneMgr->createLight("GreenLight");
        l->setPosition(0,0,-100);
        l->setDiffuseColour(0.5, 1.0, 0.5);

        // Position the camera
        mCamera->setPosition(100,50,100);
        mCamera->lookAt(-50,50,0);

        mSceneMgr->setAmbientLight(ColourValue(1,1,1,1));
        mSceneMgr->showBoundingBoxes(true);

    }
    void testOrtho()
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.0));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setPosition(800,600,0);

        mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mLightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

        Entity* pEnt = mSceneMgr->createEntity( "3", "knot.mesh" );
        mTestNode[1] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-200, 0, -200));
        mTestNode[1]->attachObject( pEnt );

        pEnt = mSceneMgr->createEntity( "4", "knot.mesh" );
        mTestNode[2] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100, 0, 200));
        mTestNode[2]->attachObject( pEnt );


        mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");


        Plane plane;
        plane.normal = Vector3::UNIT_Y;
        plane.d = 100;
        MeshManager::getSingleton().createPlane("Myplane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
            1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
        Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
        pPlaneEnt->setMaterialName("2 - Default");
        pPlaneEnt->setCastShadows(false);
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

        mCamera->setFixedYawAxis(false);
        mCamera->setProjectionType(PT_ORTHOGRAPHIC);
        mCamera->setPosition(0,10000,0);
        mCamera->lookAt(0,0,0);
        mCamera->setNearClipDistance(1000);

    }

	void testManualLOD()
	{
		MeshPtr msh1 = (MeshPtr)MeshManager::getSingleton().load("robot.mesh", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		msh1->createManualLodLevel(200, "razor.mesh");
		msh1->createManualLodLevel(500, "sphere.mesh");

		Entity *ent;
		for (int i = 0; i < 5; ++i)
		{
			ent = mSceneMgr->createEntity("robot" + StringConverter::toString(i), "robot.mesh");
			// Add entity to the scene node
			mSceneMgr->getRootSceneNode()->createChildSceneNode(
				Vector3(0,0,(i*50)-(5*50/2)))->attachObject(ent);
		}
		mAnimState = ent->getAnimationState("Walk");
		mAnimState->setEnabled(true);



		// Give it a little ambience with lights
		Light* l;
		l = mSceneMgr->createLight("BlueLight");
		l->setPosition(-200,-80,-100);
		l->setDiffuseColour(0.5, 0.5, 1.0);

		l = mSceneMgr->createLight("GreenLight");
		l->setPosition(0,0,-100);
		l->setDiffuseColour(0.5, 1.0, 0.5);

		// Position the camera
		mCamera->setPosition(100,50,100);
		mCamera->lookAt(-50,50,0);

		mSceneMgr->setAmbientLight(ColourValue::White);

	}

    void clearSceneSetup()
    {
        bool showOctree = true;
        mSceneMgr->setOption("ShowOctree", &showOctree);

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

        // Create a skydome
        mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        // Accept default settings: point light, white diffuse, just set position
        // NB I could attach the light to a SceneNode if I wanted it to move automatically with
        //  other objects, but I don't
        l->setPosition(20,80,50);

        Entity *ent;

        // Create an entity (the floor)
        ent = mSceneMgr->createEntity("floor", "FloorPlane");
        ent->setMaterialName("Examples/RustySteel");
        // Attach to child of root node, better for culling (otherwise bounds are the combination of the 2)
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);

        // Add a head, give it it's own node
        SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        ent = mSceneMgr->createEntity("head", "ogrehead.mesh");
        headNode->attachObject(ent);

        // Add another head, give it it's own node
        SceneNode* headNode2 = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        ent = mSceneMgr->createEntity("head2", "ogrehead.mesh");
        headNode2->attachObject(ent);

        // Make sure the head node tracks the root
        headNode->setAutoTracking(true, headNode2, Vector3::UNIT_Z);
        //headNode->setFixedYawAxis(true);

        // Create the camera node & attach camera
        //SceneNode* camNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        //camNode->attachObject(mCamera);

        // set up spline animation of node
        Animation* anim = mSceneMgr->createAnimation("CameraTrack", 10);
        // Spline it for nice curves
        anim->setInterpolationMode(Animation::IM_SPLINE);
        // Create a track to animate the head's node
        AnimationTrack* track = anim->createTrack(0, headNode);
        // Setup keyframes
        KeyFrame* key = track->createKeyFrame(0); // startposition
        key = track->createKeyFrame(2.5);
        key->setTranslate(Vector3(500,500,-1000));
        key = track->createKeyFrame(5);
        key->setTranslate(Vector3(-1500,1000,-600));
        key = track->createKeyFrame(7.5);
        key->setTranslate(Vector3(0,-100,0));
        key = track->createKeyFrame(10);
        key->setTranslate(Vector3(0,0,0));
        // Create a track to animate the second head's node
        track = anim->createTrack(1, headNode2);
        // Setup keyframes
        key = track->createKeyFrame(0); // startposition
        key = track->createKeyFrame(2.5);
        key->setTranslate(Vector3(-500,600,-100));
        key = track->createKeyFrame(5);
        key->setTranslate(Vector3(800,200,-600));
        key = track->createKeyFrame(7.5);
        key->setTranslate(Vector3(200,-1000,0));
        key = track->createKeyFrame(10);
        key->setTranslate(Vector3(30,70,110));
        // Create a new animation state to track this
        mAnimState = mSceneMgr->createAnimationState("CameraTrack");
        mAnimState->setEnabled(true);
    }
    class ClearSceneListener : public FrameListener
    {
    protected:
        SceneManager* mSceneMgr;
        PlayPenApplication* ppApp;

    public:
        ClearSceneListener(SceneManager* sm, PlayPenApplication* target)
        {
            mSceneMgr = sm;
            ppApp = target;
        }
        bool frameStarted(const FrameEvent& evt)
        {
            static Real timeElapsed = 0;

            timeElapsed += evt.timeSinceLastFrame;
            if (timeElapsed > 15)
            {
                mSceneMgr->clearScene();
                ppApp->clearSceneSetup();
                timeElapsed = 0;
            }
            return true;
        }
    };
    ClearSceneListener* csListener;
    void testClearScene()
    {
        // Define a floor plane mesh
        Plane p;
        p.normal = Vector3::UNIT_Y;
        p.d = 200;
        MeshManager::getSingleton().createPlane("FloorPlane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			p,200000,200000,20,20,true,1,50,50,Vector3::UNIT_Z);


        // leak here I know
        csListener = new ClearSceneListener(mSceneMgr, this);
        Root::getSingleton().addFrameListener(csListener);
        clearSceneSetup();
    }

    void testStencilShadows(ShadowTechnique tech, bool pointLight, bool directionalLight)
    {
        mSceneMgr->setShadowTechnique(tech);
        //mSceneMgr->setShowDebugShadows(true);
        mSceneMgr->setShadowDirectionalLightExtrusionDistance(1000);
        //mSceneMgr->setShadowColour(ColourValue(0.4, 0.25, 0.25));

        //mSceneMgr->setShadowFarDistance(800);
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.0));
        
        // Point light
        if(pointLight)
        {
            mLight = mSceneMgr->createLight("MainLight");
            mLight->setPosition(-400,400,-300);
            mLight->setDiffuseColour(0.9, 0.9, 1);
            mLight->setSpecularColour(0.9, 0.9, 1);
            mLight->setAttenuation(6000,1,0.001,0);
        }
        // Directional light
        if (directionalLight)
        {
            mLight = mSceneMgr->createLight("Light2");
            Vector3 dir(-1,-1,0);
            dir.normalise();
            mLight->setType(Light::LT_DIRECTIONAL);
            mLight->setDirection(dir);
            mLight->setDiffuseColour(1, 1, 0.8);
            mLight->setSpecularColour(1, 1, 1);
        }

        mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();

		// Hardware skin
        Entity* pEnt;
        pEnt = mSceneMgr->createEntity( "1", "robot.mesh" );
        AnimationState* anim = pEnt->getAnimationState("Walk");
        anim->setEnabled(true);
		mAnimStateList.push_back(anim);
		mTestNode[0]->attachObject( pEnt );

		// Software skin
		pEnt = mSceneMgr->createEntity( "12", "robot.mesh" );
		anim = pEnt->getAnimationState("Walk");
		anim->setEnabled(true);
		mAnimStateList.push_back(anim);
		mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100, 0, 0))->attachObject(pEnt);
		pEnt->setMaterialName("Examples/Rocky");


        // Does not receive shadows
        pEnt = mSceneMgr->createEntity( "3", "knot.mesh" );
        pEnt->setMaterialName("Examples/EnvMappedRustySteel");
        MaterialPtr mat2 = MaterialManager::getSingleton().getByName("Examples/EnvMappedRustySteel");
        mat2->setReceiveShadows(false);
        mTestNode[2] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-200, 0, -200));
        mTestNode[2]->attachObject( pEnt );

        // Transparent object 
        pEnt = mSceneMgr->createEntity( "3.5", "knot.mesh" );
        pEnt->setMaterialName("Examples/TransparentTest");
        MaterialPtr mat3 = MaterialManager::getSingleton().getByName("Examples/TransparentTest");
		pEnt->setCastShadows(false);
        mTestNode[3] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(350, 0, -200));
        mTestNode[3]->attachObject( pEnt );

		// User test
		/*
		pEnt = mSceneMgr->createEntity( "3.6", "ogre_male_endCaps.mesh" );
		mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 0, 100))->attachObject( pEnt );
		*/

        MeshPtr msh = MeshManager::getSingleton().load("knot.mesh", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        unsigned short src, dest;
        if (!msh->suggestTangentVectorBuildParams(src, dest))
        {
            msh->buildTangentVectors(src, dest);
        }
        pEnt = mSceneMgr->createEntity( "4", "knot.mesh" );
        pEnt->setMaterialName("Examples/BumpMapping/MultiLightSpecular");
        mTestNode[2] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100, 0, 200));
        mTestNode[2]->attachObject( pEnt );


        ParticleSystem* pSys2 = ParticleSystemManager::getSingleton().createSystem("smoke", 
            "Examples/Smoke");
        mTestNode[4] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-300, -100, 200));
        mTestNode[4]->attachObject(pSys2);


        mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");


        Plane plane;
        plane.normal = Vector3::UNIT_Y;
        plane.d = 100;
        MeshManager::getSingleton().createPlane("Myplane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
            1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
        Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
        pPlaneEnt->setMaterialName("2 - Default");
        pPlaneEnt->setCastShadows(false);
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

        mCamera->setPosition(180, 34, 223);
        mCamera->setOrientation(Quaternion(0.7265, -0.2064, 0.6304, 0.1791));

    }

    void test2Spotlights()
    {
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));

        mLight = mSceneMgr->createLight("MainLight");
        // Spotlight test
        mLight->setType(Light::LT_SPOTLIGHT);
        mLight->setDiffuseColour(1.0, 0.0, 0.8);
        mLight->setSpotlightRange(Degree(30), Degree(40));
        mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mTestNode[0]->setPosition(800,600,0);
        mTestNode[0]->lookAt(Vector3(800,0,0), Node::TS_WORLD, Vector3::UNIT_Z);
        mTestNode[0]->attachObject(mLight);

        mTestNode[1] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mLight = mSceneMgr->createLight("AnotherLight");
        // Spotlight test
        mLight->setType(Light::LT_SPOTLIGHT);
        mLight->setDiffuseColour(0, 1.0, 0.8);
        mLight->setSpotlightRange(Degree(30), Degree(40));
        mTestNode[1] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mTestNode[1]->setPosition(0,600,800);
        mTestNode[1]->lookAt(Vector3(0,0,800), Node::TS_WORLD, Vector3::UNIT_Z);
        mTestNode[1]->attachObject(mLight);


        Plane plane;
        plane.normal = Vector3::UNIT_Y;
        plane.d = 100;
        MeshManager::getSingleton().createPlane("Myplane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
            3500,3500,100,100,true,1,5,5,Vector3::UNIT_Z);
        Entity* pPlaneEnt;
        pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
        pPlaneEnt->setMaterialName("2 - Default");
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

    }
    void testTextureShadows(bool directional)
    {
        mSceneMgr->setShadowTextureSize(512);
        mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
        mSceneMgr->setShadowFarDistance(1500);
        mSceneMgr->setShadowColour(ColourValue(0.35, 0.35, 0.35));
        //mSceneMgr->setShadowFarDistance(800);
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));

        mLight = mSceneMgr->createLight("MainLight");

        
        if (directional)
		{
			// Directional test
			mLight->setType(Light::LT_DIRECTIONAL);
			Vector3 vec(-1,-1,0);
			vec.normalise();
			mLight->setDirection(vec);
		}
		else
		{

			// Spotlight test
			mLight->setType(Light::LT_SPOTLIGHT);
			mLight->setDiffuseColour(1.0, 1.0, 0.8);
			mTestNode[0] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			mTestNode[0]->setPosition(800,600,0);
			mTestNode[0]->lookAt(Vector3(0,0,0), Node::TS_WORLD, Vector3::UNIT_Z);
			mTestNode[0]->attachObject(mLight);
		}

        mTestNode[1] = mSceneMgr->getRootSceneNode()->createChildSceneNode();


        Entity* pEnt;
        pEnt = mSceneMgr->createEntity( "1", "robot.mesh" );
        mAnimState = pEnt->getAnimationState("Walk");
        mAnimState->setEnabled(true);
        //pEnt->setMaterialName("2 - Default");
        mTestNode[1]->attachObject( pEnt );
        mTestNode[1]->translate(0,-100,0);

        pEnt = mSceneMgr->createEntity( "3", "knot.mesh" );
        mTestNode[2] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-200, 0, -200));
        mTestNode[2]->attachObject( pEnt );

		// Transparent object (can force cast shadows)
		pEnt = mSceneMgr->createEntity( "3.5", "knot.mesh" );
		MaterialPtr tmat = MaterialManager::getSingleton().create("TestAlphaTransparency", 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		tmat->setTransparencyCastsShadows(true);
		Pass* tpass = tmat->getTechnique(0)->getPass(0);
		tpass->setAlphaRejectSettings(CMPF_GREATER, 150);
		tpass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
		tpass->createTextureUnitState("gras_02.png");
		tpass->setCullingMode(CULL_NONE);

		pEnt->setMaterialName("TestAlphaTransparency");
        mTestNode[3] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(350, 0, -200));
        mTestNode[3]->attachObject( pEnt );

        MeshPtr msh = MeshManager::getSingleton().load("knot.mesh",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        msh->buildTangentVectors();
        pEnt = mSceneMgr->createEntity( "4", "knot.mesh" );
        //pEnt->setMaterialName("Examples/BumpMapping/MultiLightSpecular");
        mTestNode[2] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100, 0, 200));
        mTestNode[2]->attachObject( pEnt );

        mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");


        Plane plane;
        plane.normal = Vector3::UNIT_Y;
        plane.d = 100;
        MeshManager::getSingleton().createPlane("Myplane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
            1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
        Entity* pPlaneEnt;
        pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
        pPlaneEnt->setMaterialName("2 - Default");
        pPlaneEnt->setCastShadows(false);
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

        // Set up a debug panel to display the shadow
        MaterialPtr debugMat = MaterialManager::getSingleton().create(
            "Ogre/DebugShadowMap", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        debugMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
        TextureUnitState *t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState("Ogre/ShadowTexture0");
        t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
        //t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState("spot_shadow_fade.png");
        //t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
        //t->setColourOperation(LBO_ADD);

        OverlayContainer* debugPanel = (OverlayContainer*)
            (OverlayManager::getSingleton().createOverlayElement("Panel", "Ogre/DebugShadowPanel"));
        debugPanel->_setPosition(0.8, 0);
        debugPanel->_setDimensions(0.2, 0.3);
        debugPanel->setMaterialName("Ogre/DebugShadowMap");
        Overlay* debugOverlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
        debugOverlay->add2D(debugPanel);



        ParticleSystem* pSys2 = ParticleSystemManager::getSingleton().createSystem("smoke", 
            "Examples/Smoke");
        mTestNode[4] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-300, -100, 200));
        mTestNode[4]->attachObject(pSys2);


    }

	void testRenderToFloatTexture()
	{
		//mSceneMgr->setShadowFarDistance(800);
		// Set ambient light
		mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));

		mLight = mSceneMgr->createLight("MainLight");

		// Directional test
		mLight->setType(Light::LT_DIRECTIONAL);
		Vector3 vec(-1,-1,0);
		vec.normalise();
		mLight->setDirection(vec);


		Entity* pEnt;
		pEnt = mSceneMgr->createEntity( "1", "robot.mesh" );
		mAnimState = pEnt->getAnimationState("Walk");
		mAnimState->setEnabled(true);
		//pEnt->setMaterialName("2 - Default");
		mTestNode[1] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mTestNode[1]->attachObject( pEnt );
		mTestNode[1]->translate(0,-100,0);

		pEnt = mSceneMgr->createEntity( "3", "knot.mesh" );
		mTestNode[2] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-200, 0, -200));
		mTestNode[2]->attachObject( pEnt );

		// Transparent object (can force cast shadows)
		pEnt = mSceneMgr->createEntity( "3.5", "knot.mesh" );
		pEnt->setMaterialName("Examples/TransparentTest");
		MaterialPtr mat3 = MaterialManager::getSingleton().getByName("Examples/TransparentTest");
		mat3->setTransparencyCastsShadows(true);
		mTestNode[3] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(350, 0, -200));
		mTestNode[3]->attachObject( pEnt );

		MeshPtr msh = MeshManager::getSingleton().load("knot.mesh",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		msh->buildTangentVectors();
		pEnt = mSceneMgr->createEntity( "4", "knot.mesh" );
		//pEnt->setMaterialName("Examples/BumpMapping/MultiLightSpecular");
		mTestNode[2] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100, 0, 200));
		mTestNode[2]->attachObject( pEnt );

		mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");


		Plane plane;
		plane.normal = Vector3::UNIT_Y;
		plane.d = 100;
		MeshManager::getSingleton().createPlane("Myplane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
			1500,1500,10,10,true,1,5,5,Vector3::UNIT_Z);
		Entity* pPlaneEnt;
		pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
		pPlaneEnt->setMaterialName("2 - Default");
		pPlaneEnt->setCastShadows(false);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

		RenderTexture* rttTex = 
			mRoot->getRenderSystem()->createRenderTexture( "RttTex", 512, 512, TEX_TYPE_2D, PF_FLOAT32_RGB );

		Viewport *v = rttTex->addViewport( mCamera );
		v->setClearEveryFrame( true );
		v->setBackgroundColour( ColourValue::Black );



		// Set up a debug panel to display the RTT
		MaterialPtr debugMat = MaterialManager::getSingleton().create(
			"Ogre/DebugShadowMap", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		debugMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
		TextureUnitState *t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState("RttTex");
		t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
		//t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState("spot_shadow_fade.png");
		//t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
		//t->setColourOperation(LBO_ADD);

		OverlayContainer* debugPanel = (OverlayContainer*)
			(OverlayManager::getSingleton().createOverlayElement("Panel", "Ogre/DebugShadowPanel"));
		debugPanel->_setPosition(0.8, 0);
		debugPanel->_setDimensions(0.2, 0.3);
		debugPanel->setMaterialName("Ogre/DebugShadowMap");
		Overlay* debugOverlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
		debugOverlay->add2D(debugPanel);



		ParticleSystem* pSys2 = ParticleSystemManager::getSingleton().createSystem("smoke", 
			"Examples/Smoke");
		mTestNode[4] = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-300, -100, 200));
		mTestNode[4]->attachObject(pSys2);


	}

	void testOverlayZOrder(void)
    {
        Overlay* o = OverlayManager::getSingleton().getByName("Test/Overlay3");
        o->show();
        o = OverlayManager::getSingleton().getByName("Test/Overlay2");
        o->show();
        o = OverlayManager::getSingleton().getByName("Test/Overlay1");
        o->show();
    }

    void createRandomEntityClones(Entity* ent, size_t cloneCount, 
        const Vector3& min, const Vector3& max)
    {
        Entity *cloneEnt;
        for (int n = 0; n < cloneCount; ++n)
        {
            // Create a new node under the root
            SceneNode* node = mSceneMgr->createSceneNode();
            // Random translate
            Vector3 nodePos;
            nodePos.x = Math::RangeRandom(min.x, max.x);
            nodePos.y = Math::RangeRandom(min.y, max.y);
            nodePos.z = Math::RangeRandom(min.z, max.z);
            node->setPosition(nodePos);
            mSceneMgr->getRootSceneNode()->addChild(node);
            cloneEnt = ent->clone(ent->getName() + "_clone" + StringConverter::toString(n));
            // Attach to new node
            node->attachObject(cloneEnt);

        }
    }

    void testIntersectionSceneQuery()
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-Vector3::UNIT_Y);

        // Create a set of random balls
        Entity* ent = mSceneMgr->createEntity("Ball", "sphere.mesh");
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
        createRandomEntityClones(ent, 500, Vector3(-5000,-5000,-5000), Vector3(5000,5000,5000));

        //intersectionQuery = mSceneMgr->createIntersectionQuery();
    }

    void testRaySceneQuery()
    {
        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

        // Create a point light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setType(Light::LT_DIRECTIONAL);
        l->setDirection(-Vector3::UNIT_Y);

        // Create a set of random balls
        Entity* ent = mSceneMgr->createEntity("Ball", "sphere.mesh");
        mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
        createRandomEntityClones(ent, 100, Vector3(-1000,-1000,-1000), Vector3(1000,1000,1000));

        rayQuery = mSceneMgr->createRayQuery(
            Ray(mCamera->getPosition(), mCamera->getDirection()));
        rayQuery->setSortByDistance(true, 1);

        //bool val = true;
        //mSceneMgr->setOption("ShowOctree", &val);

    }

	void testLotsAndLotsOfEntities()
	{
		// Set ambient light
		mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

		// Create a point light
		Light* l = mSceneMgr->createLight("MainLight");
		l->setType(Light::LT_DIRECTIONAL);
		l->setDirection(-Vector3::UNIT_Y);

		// Create a set of random balls
		Entity* ent = mSceneMgr->createEntity("Ball", "ogrehead.mesh");
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
		createRandomEntityClones(ent, 400, Vector3(-2000,-2000,-2000), Vector3(2000,2000,2000));

		//bool val = true;
		//mSceneMgr->setOption("ShowOctree", &val);

	}

	void testSimpleMesh()
	{
		// Set ambient light
		mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

		// Create a point light
		Light* l = mSceneMgr->createLight("MainLight");
		l->setType(Light::LT_DIRECTIONAL);
		Vector3 dir(1, -1, 0);
		dir.normalise();
		l->setDirection(dir);

		// Create a set of random balls
		Entity* ent = mSceneMgr->createEntity("test", "xsicylinder.mesh");
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);

	}

	void test2Windows(void)
	{
		// Set ambient light
		mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

		// Create a point light
		Light* l = mSceneMgr->createLight("MainLight");
		l->setType(Light::LT_DIRECTIONAL);
		Vector3 dir(1, -1, 0);
		dir.normalise();
		l->setDirection(dir);

		// Create a set of random balls
		Entity* ent = mSceneMgr->createEntity("test", "ogrehead.mesh");
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);

		NameValuePairList valuePair;
		valuePair["top"] = StringConverter::toString(0);
		valuePair["left"] = StringConverter::toString(0);

		RenderWindow* win2 = mRoot->createRenderWindow("window2", 200,200, false, &valuePair);
		win2->addViewport(mCamera);

	}

	void testStaticGeometry(void)
	{
		mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_MODULATIVE);
		//mSceneMgr->setShowDebugShadows(true);

		mSceneMgr->setSkyBox(true, "Examples/EveningSkyBox");
		// Set ambient light
		mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

		// Create a point light
		Light* l = mSceneMgr->createLight("MainLight");
		l->setType(Light::LT_DIRECTIONAL);
		Vector3 dir(1, -1, -1.5);
		dir.normalise();
		l->setDirection(dir);
		l->setDiffuseColour(1.0, 0.7, 0.0);


		Plane plane;
		plane.normal = Vector3::UNIT_Y;
		plane.d = 0;
		MeshManager::getSingleton().createPlane("Myplane",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
			4500,4500,10,10,true,1,5,5,Vector3::UNIT_Z);
		Entity* pPlaneEnt = mSceneMgr->createEntity( "plane", "Myplane" );
		pPlaneEnt->setMaterialName("Examples/GrassFloor");
		pPlaneEnt->setCastShadows(false);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pPlaneEnt);

		Vector3 min(-2000,0,-2000);
		Vector3 max(2000,0,2000);


		Entity* e = mSceneMgr->createEntity("1", "column.mesh");
		//createRandomEntityClones(e, 1000, min, max);
		
		StaticGeometry* s = mSceneMgr->createStaticGeometry("bing");
		s->setCastShadows(true);
		s->setRegionDimensions(Vector3(500,500,500));
		for (int i = 0; i < 300; ++i)
		{
			Vector3 pos;
			pos.x = Math::RangeRandom(min.x, max.x);
			pos.y = Math::RangeRandom(min.y, max.y);
			pos.z = Math::RangeRandom(min.z, max.z);

			s->addEntity(e, pos, Quaternion::IDENTITY, Vector3(5,5,5));

		}

		s->build();
		//s->setRenderingDistance(1000);
		//s->dump("static.txt");
		//mSceneMgr->showBoundingBoxes(true);
		mCamera->setLodBias(0.5);
		



	}

	void testReloadResources()
	{
		mSceneMgr->setAmbientLight(ColourValue::White);
		ResourceGroupManager::getSingleton().createResourceGroup("Sinbad");
		Root::getSingleton().addResourceLocation("../../../Media/models", "FileSystem", "Sinbad");
		MeshManager& mmgr = MeshManager::getSingleton();
		mmgr.load("robot.mesh", "Sinbad");
		mmgr.load("knot.mesh", "Sinbad");

		Entity* e = mSceneMgr->createEntity("1", "robot.mesh");
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(e);
		e = mSceneMgr->createEntity("2", "robot.mesh");
		mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100,0,0))->attachObject(e);
		e = mSceneMgr->createEntity("3", "knot.mesh");
		mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(100,300,0))->attachObject(e);

		testreload = true;

	}

	void testDepthBias()
	{
		SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode("Entity");
		const String meshName("cube.mesh");
		Entity* entity;

		entity = mSceneMgr->createEntity("Entity", meshName);
		entity->setMaterialName("Entity");
		node->attachObject(entity);

		for (size_t i = 0; i <= 6;++i)
		{
			String name("Decal");
			name += StringConverter::toString(i);

			MaterialPtr pMat = static_cast<MaterialPtr>(MaterialManager::getSingleton().create(
				name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME));

			pMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
			pMat->getTechnique(0)->getPass(0)->setAlphaRejectSettings(CMPF_GREATER_EQUAL, 128);
			pMat->getTechnique(0)->getPass(0)->setDepthBias(i);
			pMat->getTechnique(0)->getPass(0)->createTextureUnitState(name + ".png");

			entity = mSceneMgr->createEntity(name, meshName);
			entity->setMaterialName(name);
			node->attachObject(entity);
		}

		node = mSceneMgr->getRootSceneNode()->createChildSceneNode("Entity2",
			Vector3(-50.0, 0.0, 50.0), Quaternion(Degree(45.0), Vector3::UNIT_Z));
		entity = mSceneMgr->createEntity("Entity2", meshName);
		entity->setMaterialName("Examples/RustySteel");
		node->attachObject(entity);

		mCamera->setPosition(70.0, 90.0, 220.0);
		mCamera->lookAt(Vector3::ZERO); 
	}

	void testPerf()
	{
		int count = 0;
		Entity* ent;
		SceneNode *node;      
		for (int i = 0; i < 21; i++)
			for(int j = 0; j < 21; j++)
			{
				char tmp[64];
				sprintf(tmp, "Trial_%d", count);      
				count++;
				ent = mSceneMgr->createEntity(tmp,"cube.mesh");
				node =mSceneMgr->getRootSceneNode()->createChildSceneNode();
				node->attachObject( ent );                     
				node->setPosition(i*100, 0, j*100);
			}

			mCamera->setPosition(Vector3(3000,1000,3000));
			mCamera->lookAt(Vector3(0,0,0)); 
	}
	void testSphere(const std::string& strName, const float r, const int nRings = 16, const int nSegments = 16)
	{
		MeshPtr pSphere = MeshManager::getSingleton().createManual(strName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		SubMesh *pSphereVertex = pSphere->createSubMesh();

		pSphere->sharedVertexData = new VertexData();
		VertexData* vertexData = pSphere->sharedVertexData;

		// define the vertex format
		VertexDeclaration* vertexDecl = vertexData->vertexDeclaration;
		size_t currOffset = 0;
		// positions
		vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_POSITION);
		currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		// normals
		vertexDecl->addElement(0, currOffset, VET_FLOAT3, VES_NORMAL);
		currOffset += VertexElement::getTypeSize(VET_FLOAT3);
		// two dimensional texture coordinates
		vertexDecl->addElement(0, currOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
		currOffset += VertexElement::getTypeSize(VET_FLOAT2);

		// allocate the vertex buffer
		vertexData->vertexCount = (nRings + 1) * (nSegments+1);
		HardwareVertexBufferSharedPtr vBuf = HardwareBufferManager::getSingleton().createVertexBuffer(vertexDecl->getVertexSize(0), vertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		VertexBufferBinding* binding = vertexData->vertexBufferBinding;
		binding->setBinding(0, vBuf);
		float* pVertex = static_cast<float*>(vBuf->lock(HardwareBuffer::HBL_DISCARD));

		// allocate index buffer
		pSphereVertex->indexData->indexCount = 6 * nRings * (nSegments + 1);
		pSphereVertex->indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, pSphereVertex->indexData->indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		HardwareIndexBufferSharedPtr iBuf = pSphereVertex->indexData->indexBuffer;
		unsigned short* pIndices = static_cast<unsigned short*>(iBuf->lock(HardwareBuffer::HBL_DISCARD));

		float fDeltaRingAngle = (Math::PI / nRings);
		float fDeltaSegAngle = (2 * Math::PI / nSegments);
		unsigned short wVerticeIndex = 0 ;

		// Generate the group of rings for the sphere
		for( int ring = 0; ring <= nRings; ring++ ) {
			float r0 = r * sinf (ring * fDeltaRingAngle);
			float y0 = r * cosf (ring * fDeltaRingAngle);

			// Generate the group of segments for the current ring
			for(int seg = 0; seg <= nSegments; seg++) {
				float x0 = r0 * sinf(seg * fDeltaSegAngle);
				float z0 = r0 * cosf(seg * fDeltaSegAngle);

				// Add one vertex to the strip which makes up the sphere
				*pVertex++ = x0;
				*pVertex++ = y0;
				*pVertex++ = z0;

				Vector3 vNormal = Vector3(x0, y0, z0).normalisedCopy();
				*pVertex++ = vNormal.x;
				*pVertex++ = vNormal.y;
				*pVertex++ = vNormal.z;

				*pVertex++ = (float) seg / (float) nSegments;
				*pVertex++ = (float) ring / (float) nRings;

				if (ring != nRings) {
					// each vertex (except the last) has six indices pointing to it
					*pIndices++ = wVerticeIndex + nSegments + 1;
					*pIndices++ = wVerticeIndex;               
					*pIndices++ = wVerticeIndex + nSegments;
					*pIndices++ = wVerticeIndex + nSegments + 1;
					*pIndices++ = wVerticeIndex + 1;
					*pIndices++ = wVerticeIndex;
					wVerticeIndex ++;
				}
			}; // end for seg
		} // end for ring

		// Unlock
		vBuf->unlock();
		iBuf->unlock();
		// Generate face list
		pSphereVertex->useSharedVertices = true;

		// the original code was missing this line:
		pSphere->_setBounds( AxisAlignedBox( Vector3(-r, -r, -r), Vector3(r, r, r) ), false );
		pSphere->_setBoundingSphereRadius(r);
		// this line makes clear the mesh is loaded (avoids memory leaks)
		pSphere->load();

		pSphere->unload();
	}

	void testColourCube()
	{
	/// Create the mesh via the MeshManager
	Ogre::MeshPtr msh = MeshManager::getSingleton().createManual("ColourCube", "General");

	/// Create one submesh
	SubMesh* sub = msh->createSubMesh();

	const float sqrt13 = 0.577350269f; /* sqrt(1/3) */

	/// Define the vertices (8 vertices, each consisting of 3 groups of 3 floats
	const size_t nVertices = 8;
	const size_t vbufCount = 3*2*nVertices;
	float vertices[vbufCount] = {
		-100.0,100.0,-100.0,        //0 position
			-sqrt13,sqrt13,-sqrt13,     //0 normal
			100.0,100.0,-100.0,         //1 position
			sqrt13,sqrt13,-sqrt13,      //1 normal
			100.0,-100.0,-100.0,        //2 position
			sqrt13,-sqrt13,-sqrt13,     //2 normal
			-100.0,-100.0,-100.0,       //3 position
			-sqrt13,-sqrt13,-sqrt13,    //3 normal
			-100.0,100.0,100.0,         //4 position
			-sqrt13,sqrt13,sqrt13,      //4 normal
			100.0,100.0,100.0,          //5 position
			sqrt13,sqrt13,sqrt13,       //5 normal
			100.0,-100.0,100.0,         //6 position
			sqrt13,-sqrt13,sqrt13,      //6 normal
			-100.0,-100.0,100.0,        //7 position
			-sqrt13,-sqrt13,sqrt13,     //7 normal
	};

	RenderSystem* rs = Root::getSingleton().getRenderSystem();
	RGBA colours[nVertices];
	RGBA *pColour = colours;
	// Use render system to convert colour value since colour packing varies
	rs->convertColourValue(ColourValue(1.0,0.0,0.0), pColour++); //0 colour
	rs->convertColourValue(ColourValue(1.0,1.0,0.0), pColour++); //1 colour
	rs->convertColourValue(ColourValue(0.0,1.0,0.0), pColour++); //2 colour
	rs->convertColourValue(ColourValue(0.0,0.0,0.0), pColour++); //3 colour
	rs->convertColourValue(ColourValue(1.0,0.0,1.0), pColour++); //4 colour
	rs->convertColourValue(ColourValue(1.0,1.0,1.0), pColour++); //5 colour
	rs->convertColourValue(ColourValue(0.0,1.0,1.0), pColour++); //6 colour
	rs->convertColourValue(ColourValue(0.0,0.0,1.0), pColour++); //7 colour

	/// Define 12 triangles (two triangles per cube face)
	/// The values in this table refer to vertices in the above table
	const size_t ibufCount = 36;
	unsigned short faces[ibufCount] = {
		0,2,3,
			0,1,2,
			1,6,2,
			1,5,6,
			4,6,5,
			4,7,6,
			0,7,4,
			0,3,7,
			0,5,1,
			0,4,5,
			2,7,3,
			2,6,7
	};

	/// Create vertex data structure for 8 vertices shared between submeshes
	msh->sharedVertexData = new VertexData();
	msh->sharedVertexData->vertexCount = nVertices;

	/// Create declaration (memory format) of vertex data
	VertexDeclaration* decl = msh->sharedVertexData->vertexDeclaration;
	size_t offset = 0;
	// 1st buffer
	decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
	offset += VertexElement::getTypeSize(VET_FLOAT3);
	decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
	offset += VertexElement::getTypeSize(VET_FLOAT3);
	/// Allocate vertex buffer of the requested number of vertices (vertexCount) 
	/// and bytes per vertex (offset)
	HardwareVertexBufferSharedPtr vbuf = 
		HardwareBufferManager::getSingleton().createVertexBuffer(
		offset, msh->sharedVertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
	/// Upload the vertex data to the card
	vbuf->writeData(0, vbuf->getSizeInBytes(), vertices, true);

	/// Set vertex buffer binding so buffer 0 is bound to our vertex buffer
	VertexBufferBinding* bind = msh->sharedVertexData->vertexBufferBinding; 
	bind->setBinding(0, vbuf);

	// 2nd buffer
	offset = 0;
	decl->addElement(1, offset, VET_COLOUR, VES_DIFFUSE);
	offset += VertexElement::getTypeSize(VET_COLOUR);
	/// Allocate vertex buffer of the requested number of vertices (vertexCount) 
	/// and bytes per vertex (offset)
	vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
		offset, msh->sharedVertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
	/// Upload the vertex data to the card
	vbuf->writeData(0, vbuf->getSizeInBytes(), colours, true);

	/// Set vertex buffer binding so buffer 1 is bound to our colour buffer
	bind->setBinding(1, vbuf);

	/// Allocate index buffer of the requested number of vertices (ibufCount) 
	HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
		createIndexBuffer(
		HardwareIndexBuffer::IT_16BIT, 
		ibufCount, 
		HardwareBuffer::HBU_STATIC_WRITE_ONLY);

	/// Upload the index data to the card
	ibuf->writeData(0, ibuf->getSizeInBytes(), faces, true);

	/// Set parameters of the submesh
	sub->useSharedVertices = true;
	sub->indexData->indexBuffer = ibuf;
	sub->indexData->indexCount = ibufCount;
	sub->indexData->indexStart = 0;

	/// Set bounding information (for culling)
	msh->_setBounds(AxisAlignedBox(-100,-100,0,100,100,0));
	msh->_setBoundingSphereRadius(Math::Sqrt(100*100+100*100));

	/// Notify Mesh object that it has been loaded
	msh->load();


		MaterialPtr mat = MaterialManager::getSingleton().create(
			"Test/ColourTest", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		mat->getTechnique(0)->getPass(0)->setVertexColourTracking(TVC_AMBIENT);

		Entity* thisEntity = mSceneMgr->createEntity("cc", "ColourCube");
		thisEntity->setMaterialName("Test/ColourTest");
		SceneNode* thisSceneNode = static_cast<SceneNode*>(mSceneMgr->getRootSceneNode()->createChild());
		thisSceneNode->setPosition(-35, 0, 0);
		thisSceneNode->attachObject(thisEntity);

	}
	void testSplitPassesTooManyTexUnits()
	{
		MaterialPtr mat = MaterialManager::getSingleton().create(
			"Test", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		Pass* p = mat->getTechnique(0)->getPass(0);
		p->createTextureUnitState("gras_02.png");
		p->createTextureUnitState("gras_02.png");
		p->createTextureUnitState("gras_02.png");
		p->createTextureUnitState("gras_02.png");
		p->createTextureUnitState("gras_02.png");
		p->createTextureUnitState("gras_02.png");
		p->createTextureUnitState("gras_02.png");
		p->createTextureUnitState("gras_02.png");
		p->createTextureUnitState("gras_02.png");

		mat->compile();

	}
    // Just override the mandatory create scene method
    void createScene(void)
    {
        //testMatrices();
        //testBsp();
        //testAlpha();
        //testAnimation();

        //testGpuPrograms();
        //testMultiViewports();
        //testDistortion();
        //testEdgeBuilderSingleIndexBufSingleVertexBuf();
        //testEdgeBuilderMultiIndexBufSingleVertexBuf();
        //testEdgeBuilderMultiIndexBufMultiVertexBuf();
        //testPrepareShadowVolume();
        //testWindowedViewportMode();
        //testSubEntityVisibility();
        //testAttachObjectsToBones();
        //testSkeletalAnimation();
        //testOrtho();
        //testClearScene();

        //testProjection();
        //testStencilShadows(SHADOWTYPE_STENCIL_ADDITIVE, true, true);
        //testStencilShadows(SHADOWTYPE_STENCIL_MODULATIVE, false, true);
        //testTextureShadows(true);
		testSplitPassesTooManyTexUnits();
        //testOverlayZOrder();

        //testRaySceneQuery();
		//testSphere("MrSphere", 100);
        //testIntersectionSceneQuery();

        //test2Spotlights();

		//testManualLOD();
		//testLotsAndLotsOfEntities();
		//testSimpleMesh();
		//test2Windows();
		//testStaticGeometry();
		//testBug();
		//testReloadResources();
		//testTransparencyMipMaps();
		//testDepthBias();
		//testPerf();
		//testColourCube();
		//testReflectedBillboards();

    }
    // Create new frame listener
    void createFrameListener(void)
    {
        mFrameListener= new PlayPenListener(mSceneMgr, mWindow, mCamera);
        mFrameListener->showDebugOverlay(true);
		mRoot->addFrameListener(mFrameListener);
		//FrameListener* fl = new UberSimpleFrameListener(mWindow, mCamera);
        //mRoot->addFrameListener(fl);

    }
    

public:
    void go(void)
    {
        if (!setup())
            return;

        mRoot->startRendering();
    }


};



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class SwitchApplication : public ExampleApplication
{
public:
	SwitchApplication()                   { }
	~SwitchApplication()                  { }

	bool frameStarted(const FrameEvent &evt);
	void cleanup();
	void scene1();
	void scene2();
	Camera* getCamera()             { return mCamera; }

protected:
	AnimationState *mAnimationState;

	void createCamera()            { }
	void createViewports()         { }
	void chooseSceneManager(void)  { }
	void createScene(void)         { scene1(); }

	void createFrameListener(void);
};


class SwitchListener : public ExampleFrameListener
{
protected:
	SwitchApplication* mApp;
public:
	SwitchListener(SwitchApplication* app, RenderWindow* win, Camera* cam)
		: ExampleFrameListener(win, cam, false, false)
	{
		mApp = app;
	}

	bool frameStarted(const FrameEvent &evt);
};


bool SwitchListener::frameStarted(const FrameEvent &evt)
{
	if ( mInputDevice->isKeyDown( KC_1 ) && (mTimeUntilNextToggle < 0.0f))
	{
		mApp->cleanup();
		mApp->scene1();
		mCamera = mApp->getCamera();
		mTimeUntilNextToggle = 0.5f;
	}

	if ( mInputDevice->isKeyDown( KC_2 ) && (mTimeUntilNextToggle < 0.0f))
	{
		mApp->cleanup();
		mApp->scene2();
		mCamera = mApp->getCamera();
		mTimeUntilNextToggle = 0.5f;
	}

	mApp->frameStarted(evt);    // for processing animation
	return ExampleFrameListener::frameStarted(evt);
}

bool SwitchApplication::frameStarted(const FrameEvent &evt)
{
	if(mAnimationState)
		mAnimationState->addTime(evt.timeSinceLastFrame);
	return true;
}

void SwitchApplication::cleanup()
{
	mAnimationState = 0;
	mWindow->removeAllViewports();
	mSceneMgr->removeAllCameras();
	mSceneMgr->clearScene();
}

void SwitchApplication::scene1()
{
	mSceneMgr = Root::getSingleton().getSceneManager(ST_EXTERIOR_CLOSE);
	mCamera = mSceneMgr->createCamera("camera1");
	mCamera->setNearClipDistance(1);
	mCamera->setPosition(400, 100, 850);
	mCamera->lookAt(490, 100, 590);
	mWindow->addViewport(mCamera);

	mSceneMgr->setWorldGeometry( "Terrain.cfg" );
	mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox", 1000 );
	//mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_MODULATIVE);
	//mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);

	Light *light = mSceneMgr->createLight("Light1");
	light->setPosition(Vector3(-1000, 1000, 1000));
	light->setType(Light::LT_DIRECTIONAL);
	Vector3 direction = Vector3(1.5, -.6, -1.2);
	direction.normalise();
	light->setDirection(direction);
	light->setDiffuseColour(ColourValue(.95, .95, .5));
	light->setSpecularColour(ColourValue(.95, .95, .5));


	Entity *ent = mSceneMgr->createEntity( "Robot", "robot.mesh" );
	ent->setMaterialName( "Examples/Rocky" );
	SceneNode *sn;
	sn = mSceneMgr->getRootSceneNode()->createChildSceneNode("RobotNode");
	sn->attachObject(ent);
	sn->setPosition(490,100,590);

	mAnimationState = ent->getAnimationState( "Idle" );
	mAnimationState->setLoop( true );
	mAnimationState->setEnabled( true );
}

void SwitchApplication::scene2()
{
	mSceneMgr = Root::getSingleton().getSceneManager(ST_GENERIC);
	mCamera = mSceneMgr->createCamera("camera1");
	mCamera->setNearClipDistance(1);
	mCamera->setPosition(0, 10, 300);
	mCamera->lookAt(0, 0, 0);
	mWindow->addViewport(mCamera);

	mSceneMgr->setSkyBox(true,"Examples/EveningSkyBox", 1000);
	//mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_MODULATIVE);
	//mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);
	//mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);

	Light *light = mSceneMgr->createLight("Light2");
	light->setPosition(Vector3(-1000, 1000, 1000));
	light->setType(Light::LT_DIRECTIONAL);
	Vector3 direction = Vector3(1.5, -.6, -1.2);
	direction.normalise();
	light->setDirection(direction);
	light->setDiffuseColour(ColourValue(.95, .95, .5));
	light->setSpecularColour(ColourValue(.95, .95, .5));

	Plane plane(Vector3::UNIT_Y, 0);
	MeshManager::getSingleton().createPlane("ground",
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
		3000,1000,30,10,true,1,30,10,Vector3::UNIT_Z);
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	Entity *ent = mSceneMgr->createEntity("groundEnt", "ground");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
	ent->setMaterialName("Examples/Water5");
	ent->setCastShadows(false);

	SceneNode *sn;
	ent = mSceneMgr->createEntity ("ogreEnt", "ogrehead.mesh");
	ent->setMaterialName("Examples/TransparentTest");
	sn = mSceneMgr->getRootSceneNode()->createChildSceneNode("ogreNode");
	sn->attachObject(ent);
	sn->setPosition(0, 30, 0);
}


void SwitchApplication::createFrameListener(void)
{
	mFrameListener = new SwitchListener(this, mWindow, mCamera);
	mFrameListener->showDebugOverlay(true);
	mRoot->addFrameListener(mFrameListener);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool gReload;

// Listener class for frame updates
class MemoryTestFrameListener : public FrameListener, public KeyListener
{
protected:
	Real time;
	EventProcessor* mEventProcessor;
	InputReader* mInputDevice;
public:
	MemoryTestFrameListener(RenderWindow * win)
	{
		time = 0;
		mEventProcessor = new EventProcessor();
		mEventProcessor->initialise(win);
		mEventProcessor->startProcessingEvents();
		mEventProcessor->addKeyListener(this);
		mInputDevice = mEventProcessor->getInputReader();
	}
	virtual ~MemoryTestFrameListener()
	{
		time = 0;            
		delete mEventProcessor;
	}

	bool frameStarted(const FrameEvent& evt)
	{
		if( mInputDevice->isKeyDown( KC_ESCAPE) )
		{
			gReload = false;
			return false;
		}

		time += evt.timeSinceLastFrame;
		if(time>5)
		{
			LogManager::getSingleton().logMessage("Reloading scene after 5 seconds");
			gReload = true;
			time=0;
			return false;
		}
		else
		{
			gReload = false;
			return true;
		}
	}

	void keyClicked(KeyEvent* e) {};
	void keyPressed(KeyEvent* e) {};
	void keyReleased(KeyEvent* e) {};
	void keyFocusIn(KeyEvent* e) {}
	void keyFocusOut(KeyEvent* e) {}
};

/** Application class */
class MemoryTestApplication : public ExampleApplication
{
protected:
	MemoryTestFrameListener * mTestFrameListener;
public:

	void go(void)
	{
		mRoot = 0;
		if (!setup())
			return;

		mRoot->startRendering();

		while(gReload)
		{
			// clean up
			destroyScene();
			destroyResources();
			if (!setup())
				return;
			mRoot->startRendering();
		}
		// clean up
		destroyScene();
	}

	bool setup(void)
	{
		if(!gReload)
			mRoot = new Root();

		setupResources();

		if(!gReload)
		{
			bool carryOn = configure();
			if (!carryOn)
				return false;

			chooseSceneManager();
			createCamera();
			createViewports();

			// Set default mipmap level (NB some APIs ignore this)
			TextureManager::getSingleton().setDefaultNumMipmaps(5);

			// Create any resource listeners (for loading screens)
			createResourceListener();

			createFrameListener();
		}
		// Load resources
		loadResources();

		// Create the scene
		createScene();        

		return true;

	}

	/// Method which will define the source of resources (other than current folder)
	virtual void setupResources(void)
	{
		// Custom setup
		ResourceGroupManager::getSingleton().createResourceGroup("CustomResourceGroup");
		ResourceGroupManager::getSingleton().addResourceLocation(
			"../../../media/ogrehead.zip", "Zip", "CustomResourceGroup");
	}
	void loadResources(void)
	{
		// Initialise, parse scripts etc
		ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	}
	void destroyResources()
	{
		LogManager::getSingleton().logMessage("Destroying resources");
		ResourceGroupManager::getSingleton().removeResourceLocation(
			"../../../media/ogrehead.zip");
		ResourceGroupManager::getSingleton().destroyResourceGroup("CustomResourceGroup");
	}

	void createScene(void)
	{
		// Set a very low level of ambient lighting
		mSceneMgr->setAmbientLight(ColourValue(0.1, 0.1, 0.1));

		// Load ogre head
		MeshManager::getSingleton().load("ogrehead.mesh","CustomResourceGroup");
		Entity* head = mSceneMgr->createEntity("head", "ogrehead.mesh");

		// Attach the head to the scene
		mSceneMgr->getRootSceneNode()->attachObject(head);

	}

	void createFrameListener(void)
	{
		// This is where we instantiate our own frame listener
		mTestFrameListener= new MemoryTestFrameListener(mWindow);
		mRoot->addFrameListener(mTestFrameListener);
		/*if(!gReload)
		{
		ExampleApplication::createFrameListener();
		}*/
	}

	void destroyScene(void)
	{
		LogManager::getSingleton().logMessage("Clearing scene");
		mSceneMgr->clearScene();
	}
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------




#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"


INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    PlayPenApplication app;
	//SwitchApplication app;
	//MemoryTestApplication app;

    try {
        app.go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " << e.getFullDescription();
#endif
    }


    return 0;
}






