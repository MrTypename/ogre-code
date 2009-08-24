#ifndef __SdkSample_H__
#define __SdkSample_H__

#include "Sample.h"
#include "SdkTrays.h"
#include "SdkCameraMan.h"

namespace OgreBites
{
	/*=============================================================================
	// Base SDK sample class. Includes default player camera and SDK trays.
	=============================================================================*/
	class SdkSample : public Sample, public SdkTrayListener
    {
    public:

		SdkSample()
		{
			// so we don't have to worry about checking if these keys exist later
			mInfo["Title"] = "Untitled";
			mInfo["Description"] = "";
			mInfo["Category"] = "Unsorted";
			mInfo["Thumbnail"] = "";
			mInfo["Help"] = "";

			mTrayMgr = 0;
			mCameraMan = 0;
		}

		virtual ~SdkSample() {}

		/*-----------------------------------------------------------------------------
		| Manually update the cursor position after being unpaused.
		-----------------------------------------------------------------------------*/
		virtual void unpaused()
		{
			mTrayMgr->refreshCursor();
		}

		/*-----------------------------------------------------------------------------
		| Automatically saves position and orientation for free-look cameras.
		-----------------------------------------------------------------------------*/
		virtual void saveState(Ogre::NameValuePairList& state)
		{
			if (mCameraMan->getStyle() == CS_FREELOOK)
			{
				state["CameraPosition"] = Ogre::StringConverter::toString(mCamera->getPosition());
				state["CameraOrientation"] = Ogre::StringConverter::toString(mCamera->getOrientation());
			}
		}

		/*-----------------------------------------------------------------------------
		| Automatically restores position and orientation for free-look cameras.
		-----------------------------------------------------------------------------*/
		virtual void restoreState(Ogre::NameValuePairList& state)
		{
			if (state.find("CameraPosition") != state.end() && state.find("CameraOrientation") != state.end())
			{
				mCameraMan->setStyle(CS_FREELOOK);
				mCamera->setPosition(Ogre::StringConverter::parseVector3(state["CameraPosition"]));
				mCamera->setOrientation(Ogre::StringConverter::parseQuaternion(state["CameraOrientation"]));
			}
		}

		virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt)
		{
			mTrayMgr->frameRenderingQueued(evt);

			if (!mTrayMgr->isDialogVisible())
			{
				mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera

				if (mDetailsPanel->isVisible())   // if details panel is visible, then update its contents
				{
					mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
					mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
					mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
					mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
					mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
					mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
					mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));
				}
			}

			return true;
		}

		virtual bool keyPressed(const OIS::KeyEvent& evt)
		{
			if (evt.key == OIS::KC_H || evt.key == OIS::KC_F1)   // toggle visibility of help dialog
			{
				if (!mTrayMgr->isDialogVisible() && mInfo["Help"] != "") mTrayMgr->showOkDialog("Help", mInfo["Help"]);
				else mTrayMgr->closeDialog();
			}

			if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

			if (evt.key == OIS::KC_F)   // toggle visibility of advanced frame stats
			{
				mTrayMgr->toggleAdvancedFrameStats();
			}
			else if (evt.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
			{
				if (mDetailsPanel->getTrayLocation() == TL_NONE)
				{
					mTrayMgr->moveWidgetToTray(mDetailsPanel, TL_TOPRIGHT, 0);
					mDetailsPanel->show();
				}
				else
				{
					mTrayMgr->removeWidgetFromTray(mDetailsPanel);
					mDetailsPanel->hide();
				}
			}
			else if (evt.key == OIS::KC_T)   // cycle polygon rendering mode
			{
				Ogre::String newVal;
				Ogre::TextureFilterOptions tfo;
				unsigned int aniso;

				switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
				{
				case 'B':
					newVal = "Trilinear";
					tfo = Ogre::TFO_TRILINEAR;
					aniso = 1;
					break;
				case 'T':
					newVal = "Anisotropic";
					tfo = Ogre::TFO_ANISOTROPIC;
					aniso = 8;
					break;
				case 'A':
					newVal = "None";
					tfo = Ogre::TFO_NONE;
					aniso = 1;
					break;
				default:
					newVal = "Bilinear";
					tfo = Ogre::TFO_BILINEAR;
					aniso = 1;
				}

				Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
				Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
				mDetailsPanel->setParamValue(9, newVal);
			}
			else if (evt.key == OIS::KC_R)   // cycle polygon rendering mode
			{
				Ogre::String newVal;
				Ogre::PolygonMode pm;

				switch (mCamera->getPolygonMode())
				{
				case Ogre::PM_SOLID:
					newVal = "Wireframe";
					pm = Ogre::PM_WIREFRAME;
					break;
				case Ogre::PM_WIREFRAME:
					newVal = "Points";
					pm = Ogre::PM_POINTS;
					break;
				default:
					newVal = "Solid";
					pm = Ogre::PM_SOLID;
				}

				mCamera->setPolygonMode(pm);
				mDetailsPanel->setParamValue(10, newVal);
			}
			else if(evt.key == OIS::KC_F5)   // refresh all textures
			{
				Ogre::TextureManager::getSingleton().reloadAll();
			}
			else if (evt.key == OIS::KC_F9)   // take a screenshot
			{
				Ogre::String path = "screenshots/screenshot_";
				#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
				path = "screenshots\\screenshot_";
				#endif
				mWindow->writeContentsToFile(path + Ogre::StringConverter::toString(mRoot->getNextFrameNumber()) + ".jpg");
			}

			mCameraMan->injectKeyDown(evt);

			return true;
		}

		virtual bool keyReleased(const OIS::KeyEvent& evt)
		{
			mCameraMan->injectKeyUp(evt);

			return true;
		}

		/* IMPORTANT: When overriding these following handlers, remember to allow the tray manager
		to filter out any interface-related mouse events before processing them in your scene.
		If the tray manager handler returns true, the event was meant for the trays, not you. */

		virtual bool mouseMoved(const OIS::MouseEvent& evt)
		{
			if (mTrayMgr->injectMouseMove(evt)) return true;

			mCameraMan->injectMouseMove(evt);

			return true;
		}

		virtual bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
		{
			if (mTrayMgr->injectMouseDown(evt, id)) return true;

			mCameraMan->injectMouseDown(evt, id);

			return true;
		}

		virtual bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
		{
			if (mTrayMgr->injectMouseUp(evt, id)) return true;

			mCameraMan->injectMouseUp(evt, id);

			return true;
		}

		/*-----------------------------------------------------------------------------
		| Extendeded to setup a default tray interface and camera controller.
		-----------------------------------------------------------------------------*/
		virtual void _setup(Ogre::RenderWindow* window, OIS::Keyboard* keyboard, OIS::Mouse* mouse)
		{
			mWindow = window;
			mKeyboard = keyboard;
			mMouse = mouse;

			locateResources();
			createSceneManager();
			loadResources();
			mResourcesLoaded = true;
			setupView();
			
			mTrayMgr = new SdkTrayManager("SampleControls", window, mouse, this);  // create a tray interface

			// show stats and logo and hide the cursor
			mTrayMgr->showFrameStats(TL_BOTTOMLEFT);
			mTrayMgr->showLogo(TL_BOTTOMRIGHT);
			mTrayMgr->hideCursor();

			// create a params panel for displaying sample details
			Ogre::StringVector items;
			items.push_back("cam.pX");
			items.push_back("cam.pY");
			items.push_back("cam.pZ");
			items.push_back("");
			items.push_back("cam.oW");
			items.push_back("cam.oX");
			items.push_back("cam.oY");
			items.push_back("cam.oZ");
			items.push_back("");
			items.push_back("Filtering");
			items.push_back("Poly Mode");
			mDetailsPanel = mTrayMgr->createParamsPanel(TL_NONE, "DetailsPanel", 180, items);
			mDetailsPanel->setParamValue(9, "Bilinear");
			mDetailsPanel->setParamValue(10, "Solid");
			mDetailsPanel->hide();

			setupScene();
			mSceneCreated = true;

			mDone = false;
		}

		virtual void _shutdown()
		{
			if (mTrayMgr) delete mTrayMgr;
			if (mCameraMan) delete mCameraMan;

			// restore settings we may have changed, so as not to affect other samples
			Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_BILINEAR);
			Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(1);

			Sample::_shutdown();
		}

    protected:

		virtual void setupView()
		{
			// setup default viewport layout and camera
			mCamera = mSceneMgr->createCamera("MainCamera");
			mViewport = mWindow->addViewport(mCamera);
			mCamera->setAspectRatio((Ogre::Real)mViewport->getActualWidth() / (Ogre::Real)mViewport->getActualHeight());
			mCamera->setAutoAspectRatio(true);
			mCamera->setNearClipDistance(5);

			mCameraMan = new SdkCameraMan(mCamera);   // create a default camera controller
		}

		Ogre::Viewport* mViewport;    // main viewport
		Ogre::Camera* mCamera;        // main camera
		SdkTrayManager* mTrayMgr;     // tray interface manager
		SdkCameraMan* mCameraMan;     // basic camera controller
		ParamsPanel* mDetailsPanel;   // sample details panel
		bool mCursorWasVisible;       // was cursor visible before dialog appeared
    };
}

#endif
