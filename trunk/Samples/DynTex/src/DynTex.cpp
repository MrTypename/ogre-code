// Dynamic texturing demo
//
// Feel free to use this for anything you want (Wumpus, OGRE Team 2004). This file is public domain.
// Uses the Grey-Scott diffusion reaction process, a nice overview of the parameter space can be seen at
//   http://www.cacr.caltech.edu/ismap/image.html
//
// Some very cool effects can be reached with some combinations of parameters, varying the parameters 
// during growth gives even more curious things.
//
// Use 1-8 to change parameters, and 0 to reset to initial conditions

#include "ExampleApplication.h"
#include <OgreTexture.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreTextureManager.h>
#include <OgreLogManager.h>
#include <sstream>
TexturePtr ptex;
HardwarePixelBufferSharedPtr buffer;
Overlay* overlay;
static const int reactorExtent = 258;

// Nano fixed point library
#define FROMFLOAT(X) ((int)((X)*((float)(1<<16))))
#define TOFLOAT(X) ((float)((X)/((float)(1<<16))))
#define MULT(X,Y) (((X)*(Y))>>16)

class DynTexFrameListener : public ExampleFrameListener
{
private:
    static float fDefDim;
    static float fDefVel;
	float tim;
	
	int *chemical[2];
	int *delta[2];
	size_t mSize;
	int dt,hdiv0,hdiv1; // diffusion parameters
	int F,k; // reaction parameters

	bool rpressed;
public:
    DynTexFrameListener(RenderWindow* win, Camera* cam) : ExampleFrameListener( win, cam )
    {
		tim = 0;
		rpressed = false;
		
		// Setupr
		LogManager::getSingleton().logMessage("Creating chemical containment");
		mSize = reactorExtent*reactorExtent;
		chemical[0] = new int [mSize];
		chemical[1] = new int [mSize];
		delta[0] = new int [mSize];
		delta[1] = new int [mSize];
		
		dt = FROMFLOAT(2.0f);
		hdiv0 = FROMFLOAT(2.0E-5f/(2.0f*0.01f*0.01f)); // a / (2.0f*h*h); -- really diffusion rate 
		hdiv1 = FROMFLOAT(1.0E-5f/(2.0f*0.01f*0.01f)); // a / (2.0f*h*h); -- really diffusion rate 
		k = FROMFLOAT(0.056f);
		F = FROMFLOAT(0.020f);
		
		resetReactor();
		fireUpReactor();
		updateInfoParamF();
		updateInfoParamK();
		updateInfoParamA0();
		updateInfoParamA1();
		
		LogManager::getSingleton().logMessage("Cthulhu dawn");
    }
	void resetReactor()
	{
		LogManager::getSingleton().logMessage("Facilitating neutral start up conditions");
		for(unsigned int x=0; x<mSize; x++) 
		{
			chemical[0][x] = FROMFLOAT(1.0f);
			chemical[1][x] = FROMFLOAT(0.0f);
		}
	}
	void fireUpReactor()
	{
		LogManager::getSingleton().logMessage("Warning: reactor is being fired up");
		int center = reactorExtent/2;
		for(unsigned int x=center-10; x<center+10; x++) 
		{
			for(unsigned int y=center-10; y<center+10; y++) 
			{
				chemical[0][y*reactorExtent+x] = FROMFLOAT(0.5f) + rand()%FROMFLOAT(0.1);
				chemical[1][y*reactorExtent+x] = FROMFLOAT(0.25f) + rand()%FROMFLOAT(0.1);
			}
		}
		LogManager::getSingleton().logMessage("Warning: reaction has begun");
	}
	
	void runStep()
	{
		for(unsigned int x=0; x<mSize; x++) 
		{
			delta[0][x] = 0;
			delta[1][x] = 0;
		}
		// Boundary conditions
		unsigned int idx;
		idx = 0;
		for(unsigned int y=0; y<reactorExtent; y++) 
		{
			chemical[0][idx] = chemical[0][idx+reactorExtent-2];
			chemical[0][idx+reactorExtent-1] = chemical[0][idx+1];
			chemical[1][idx] = chemical[1][idx+reactorExtent-2];
			chemical[1][idx+reactorExtent-1] = chemical[1][idx+1];
			idx += reactorExtent;
		}
		unsigned int skip = reactorExtent*(reactorExtent-1);
		for(unsigned int y=0; y<reactorExtent; y++) 
		{
			chemical[0][y] = chemical[0][y + skip - reactorExtent];
			chemical[0][y + skip] = chemical[0][y + reactorExtent];
			chemical[1][y] = chemical[1][y + skip - reactorExtent];
			chemical[1][y + skip] = chemical[1][y + reactorExtent];
		}
		// Diffusion
		idx = reactorExtent+1;
		for(unsigned int y=0; y<reactorExtent-2; y++) 
		{
			for(unsigned int x=0; x<reactorExtent-2; x++) 
			{
				delta[0][idx] += MULT(chemical[0][idx-reactorExtent] + chemical[0][idx-1]
								-4*chemical[0][idx] + chemical[0][idx+1]
								+chemical[0][idx+reactorExtent], hdiv0);
				delta[1][idx] += MULT(chemical[1][idx-reactorExtent] + chemical[1][idx-1]
								-4*chemical[1][idx] + chemical[1][idx+1]
								+chemical[1][idx+reactorExtent], hdiv1);
				idx++;
			}
			idx += 2;
		}
		// Reaction (Grey-Scott)
		idx = reactorExtent+1;
		int U,V;

		for(unsigned int y=0; y<reactorExtent-2; y++) 
		{
			for(unsigned int x=0; x<reactorExtent-2; x++) 
			{                        
				U = chemical[0][idx]; V = chemical[1][idx];
				int UVV = MULT(MULT(U,V),V);
				delta[0][idx] += -UVV + MULT(F,(1<<16)-U);
				delta[1][idx] += UVV - MULT(F+k,V);
				idx++;
			}
			idx += 2;
		}
		// Update concentrations
		for(unsigned int x=0; x<mSize; x++) 
		{
			chemical[0][x] += MULT(delta[0][x], dt);
			chemical[1][x] += MULT(delta[1][x], dt);
		}
	}
	
	void buildTexture()
	{
		buffer->lock(HardwareBuffer::HBL_NORMAL);
		const PixelBox &pb = buffer->getCurrentLock();
		unsigned int idx = reactorExtent+1;
		for(unsigned int y=0; y<256; y++) {
			unsigned int fun = abs(((int)y)-128)*2;
			for(unsigned int x=0; x<256; x++) {
				reinterpret_cast<uint32*>(static_cast<uint8*>(pb.data)+y*pb.rowPitch)[x] = 
					0xFF003400 | (fun<<16) | ((chemical[0][idx]>>8)&0xFF);
				idx++;
			}
			idx += 2;
		}
		buffer->unlock();
	}
	// GUI updaters
	void updateInfoParamK()
	{
		GuiManager::getSingleton().getOverlayElement("Example/DynTex/Param_K") \
			->setCaption("[1/2]k: "+StringConverter::toString(TOFLOAT(k)));		
	}
	void updateInfoParamF()
	{
		GuiManager::getSingleton().getOverlayElement("Example/DynTex/Param_F") \
			->setCaption("[3/4]F: "+StringConverter::toString(TOFLOAT(F)));		
	}
	void updateInfoParamA0()
	{
		// Diffusion rate for chemical 1
		GuiManager::getSingleton().getOverlayElement("Example/DynTex/Param_A0") \
			->setCaption("[5/6]Diffusion 1: "+StringConverter::toString(TOFLOAT(hdiv0)));		
	}
	void updateInfoParamA1()
	{
		// Diffusion rate for chemical 2
		GuiManager::getSingleton().getOverlayElement("Example/DynTex/Param_A1") \
			->setCaption("[7/8]Diffusion 2: "+StringConverter::toString(TOFLOAT(hdiv1)));		
	}

    bool frameStarted( const FrameEvent& evt )
    {
        bool bOK = ExampleFrameListener::frameStarted( evt );
		
        if( mInputDevice->isKeyDown( KC_1 ) ) {
        	k -= FROMFLOAT(0.005f*evt.timeSinceLastFrame);
			updateInfoParamK();
		}
        if( mInputDevice->isKeyDown( KC_2 ) ) {
        	k += FROMFLOAT(0.005f*evt.timeSinceLastFrame);
			updateInfoParamK();
		}
        if( mInputDevice->isKeyDown( KC_3 ) ) {
        	F -= FROMFLOAT(0.005f*evt.timeSinceLastFrame);
			updateInfoParamF();
		}
        if( mInputDevice->isKeyDown( KC_4 ) ) {
        	F += FROMFLOAT(0.005f*evt.timeSinceLastFrame);
			updateInfoParamF();
		}
        if( mInputDevice->isKeyDown( KC_5 ) ) {
        	hdiv0 -= FROMFLOAT(0.005f*evt.timeSinceLastFrame);
			updateInfoParamA0();
		}
        if( mInputDevice->isKeyDown( KC_6 ) ) {
        	hdiv0 += FROMFLOAT(0.005f*evt.timeSinceLastFrame);
			updateInfoParamA0();
		}
        if( mInputDevice->isKeyDown( KC_7 ) ) {
        	hdiv1 -= FROMFLOAT(0.005f*evt.timeSinceLastFrame);
			updateInfoParamA1();
		}
        if( mInputDevice->isKeyDown( KC_8 ) ) {
        	hdiv1 += FROMFLOAT(0.005f*evt.timeSinceLastFrame);
			updateInfoParamA1();
		}

		if( mInputDevice->isKeyDown( KC_0 ) && !rpressed ) {
			// Reset 0
			resetReactor();
			fireUpReactor();
			rpressed = true;
		} else {
			rpressed = false;
		}

		tim += evt.timeSinceLastFrame;
		float rate = 100.0f;
		while(tim > (1.0f/rate)) {
			// 50 steps per second at most
			runStep();
			tim -= (1.0f/rate);
		}
		buildTexture();

        return bOK;
    }

};

float DynTexFrameListener::fDefDim = 25.0f;
float DynTexFrameListener::fDefVel = 50.0f;

class DynTexApplication : public ExampleApplication
{
public:
    DynTexApplication() {}

protected:
	
	
    virtual void createFrameListener(void)
    {
        mFrameListener= new DynTexFrameListener(mWindow, mCamera);
        mFrameListener->showDebugOverlay(true);
        mRoot->addFrameListener(mFrameListener);
    }


	virtual void createViewports(void)
    {
		// Create one viewport, entire window
        Viewport* vp = mWindow->addViewport(mCamera);
        vp->setBackgroundColour(ColourValue(0,0,0));

        // Alter the camera aspect ratio to match the viewport
        mCamera->setAspectRatio(
            Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
    }

    // Just override the mandatory create scene method
    void createScene(void)
    {
        // Create dynamic texture
		ptex = TextureManager::getSingleton().createManual(
			"DynaTex","General", TEX_TYPE_2D, 256, 256, 1, PF_A8R8G8B8);
		buffer = ptex->getBuffer(0, 0);

		// Set ambient light to low
        mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));
		mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox", 50 );

        //mRoot->getRenderSystem()->clearFrameBuffer(FBT_COLOUR, ColourValue(255,255,255,0));

        // Create a light
        Light* l = mSceneMgr->createLight("MainLight");
        l->setDiffuseColour(0.75, 0.75, 0.80);
		l->setSpecularColour(0.9, 0.9, 1);
        l->setPosition(-100,80,50);
		mSceneMgr->getRootSceneNode()->attachObject(l);

			
        Entity *planeEnt = mSceneMgr->createEntity("TexPlane1", Ogre::SceneManager::PT_PLANE);
        // Give the plane a texture
        planeEnt->setMaterialName("Examples/DynaTest");

        SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-100,-40,-100));
        node->attachObject(planeEnt);
		node->setScale(3.0f, 3.0f, 3.0f);

        // Create objects
        SceneNode *blaNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-200,0,50));
        Entity *ent2 = mSceneMgr->createEntity( "knot", "knot.mesh" );
		ent2->setMaterialName("Examples/DynaTest4");
        blaNode->attachObject( ent2 );
		
		blaNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(200,-90,50));
        ent2 = mSceneMgr->createEntity( "knot2", "knot.mesh" );
		ent2->setMaterialName("Examples/DynaTest2");
        blaNode->attachObject( ent2 );
        blaNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-110,200,50));
        ent2 = mSceneMgr->createEntity( "knot3", "fish.mesh" );
		ent2->setMaterialName("Examples/DynaTest3");
        blaNode->attachObject( ent2 );
		blaNode->setScale(50.0f, 50.0f, 50.0f);
		
		
		//TextureManager::getSingleton().getByName("RustySteel.jpg");
		
		
		std::stringstream d;
		d << "HardwarePixelBuffer " << buffer->getWidth() << " " << buffer->getHeight() << " " << buffer->getDepth();
		LogManager::getSingleton().logMessage(d.str());
		
		buffer->lock(HardwareBuffer::HBL_NORMAL);
		const PixelBox &pb = buffer->getCurrentLock();
		d.str("");
		d << "PixelBox " << pb.width << " " << pb.height << " " << pb.depth << " " << pb.rowPitch << " " << pb.slicePitch << " " << pb.data << " " << PixelUtil::getFormatName(pb.format);
		LogManager::getSingleton().logMessage(d.str());
		for(unsigned int y=0; y<256; y++) {
			for(unsigned int x=0; x<256; x++) {
				reinterpret_cast<uint32*>(static_cast<uint8*>(pb.data)+y*pb.rowPitch)[x] = 
					0xFF340000 | (y<<8) | x;
			}
		}
		buffer->unlock();
		
		// show GUI
		overlay = OverlayManager::getSingleton().getByName("Example/DynTexOverlay");    
		overlay->show();
    }

};

#if OGRE_PLATFORM == PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char *argv[])
#endif
{
    // Create application object
    DynTexApplication app;

    SET_TERM_HANDLER;
    
    try {
        app.go();
    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " <<
            e.getFullDescription().c_str() << std::endl;
#endif
    }

    return 0;
}
