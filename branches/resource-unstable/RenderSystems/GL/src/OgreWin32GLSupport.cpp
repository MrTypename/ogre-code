#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#include <algorithm>

#include "OgreWin32GLSupport.h"
#include "OgreGLTexture.h"
#include "OgreWin32Window.h"

#define HW_RTT

#ifdef HW_RTT
#include "OgreWin32RenderTexture.h"
#endif

using namespace Ogre;

namespace Ogre {
    Win32GLSupport::Win32GLSupport():
        mExternalWindowHandle(0),
		mCurrentContext(0,0)
    {
    } 

	template<class C> void remove_duplicates(C& c)
	{
		std::sort(c.begin(), c.end());
		typename C::iterator p = std::unique(c.begin(), c.end());
		c.erase(p, c.end());
	}

	void Win32GLSupport::addConfig()
	{
		//TODO: EnumDisplayDevices http://msdn.microsoft.com/library/en-us/gdi/devcons_2303.asp
		/*vector<string> DisplayDevices;
		DISPLAY_DEVICE DisplayDevice;
		DisplayDevice.cb = sizeof(DISPLAY_DEVICE);
		DWORD i=0;
		while (EnumDisplayDevices(NULL, i++, &DisplayDevice, 0) {
			DisplayDevices.push_back(DisplayDevice.DeviceName);
		}*/
		  
		ConfigOption optFullScreen;
		ConfigOption optVideoMode;
		ConfigOption optColourDepth;
		ConfigOption optDisplayFrequency;
		ConfigOption optVSync;

		// FS setting possiblities
		optFullScreen.name = "Full Screen";
		optFullScreen.possibleValues.push_back("Yes");
		optFullScreen.possibleValues.push_back("No");
		optFullScreen.currentValue = "Yes";
		optFullScreen.immutable = false;

		// Video mode possiblities
		DEVMODE DevMode;
		DevMode.dmSize = sizeof(DEVMODE);
		optVideoMode.name = "Video Mode";
		optVideoMode.immutable = false;
		for (DWORD i = 0; EnumDisplaySettings(NULL, i, &DevMode); ++i)
		{
			if (DevMode.dmBitsPerPel < 16 || DevMode.dmPelsHeight < 480)
				continue;
			mDevModes.push_back(DevMode);
			char szBuf[16];
			snprintf(szBuf, 16, "%d x %d", DevMode.dmPelsWidth, DevMode.dmPelsHeight);
			optVideoMode.possibleValues.push_back(szBuf);
		}
		remove_duplicates(optVideoMode.possibleValues);
		optVideoMode.currentValue = optVideoMode.possibleValues.front();

		optColourDepth.name = "Colour Depth";
		optColourDepth.immutable = false;
		optColourDepth.currentValue = "";

		optDisplayFrequency.name = "Display Frequency";
		optDisplayFrequency.immutable = false;
		optDisplayFrequency.currentValue = "";

		optVSync.name = "VSync";
		optVSync.immutable = false;
		optVSync.possibleValues.push_back("No");
		optVSync.possibleValues.push_back("Yes");
		optVSync.currentValue = "No";

		mOptions[optFullScreen.name] = optFullScreen;
		mOptions[optVideoMode.name] = optVideoMode;
		mOptions[optColourDepth.name] = optColourDepth;
		mOptions[optDisplayFrequency.name] = optDisplayFrequency;
		mOptions[optVSync.name] = optVSync;

		refreshConfig();
	}

	void Win32GLSupport::refreshConfig()
	{
		ConfigOptionMap::iterator optVideoMode = mOptions.find("Video Mode");
		ConfigOptionMap::iterator moptColourDepth = mOptions.find("Colour Depth");
		ConfigOptionMap::iterator moptDisplayFrequency = mOptions.find("Display Frequency");
		if(optVideoMode == mOptions.end() || moptColourDepth == mOptions.end() || moptDisplayFrequency == mOptions.end())
			Except(999, "Can't find mOptions!", "Win32GLSupport::refreshConfig");
		ConfigOption* optColourDepth = &moptColourDepth->second;
		ConfigOption* optDisplayFrequency = &moptDisplayFrequency->second;

		String val = optVideoMode->second.currentValue;
		String::size_type pos = val.find('x');
		if (pos == String::npos)
			Except(999, "Invalid Video Mode provided", "Win32GLSupport::refreshConfig");
		int width = atoi(val.substr(0, pos).c_str());

		for(std::vector<DEVMODE>::const_iterator i = mDevModes.begin(); i != mDevModes.end(); ++i)
		{
			if (int(i->dmPelsWidth) != width)
				continue;
			char buf[128];
			sprintf(buf, "%d", i->dmBitsPerPel);
			optColourDepth->possibleValues.push_back(buf);
			sprintf(buf, "%d", i->dmDisplayFrequency);
			optDisplayFrequency->possibleValues.push_back(buf);
		}
		remove_duplicates(optColourDepth->possibleValues);
		remove_duplicates(optDisplayFrequency->possibleValues);
		optColourDepth->currentValue = optColourDepth->possibleValues.back();
		optDisplayFrequency->currentValue = optDisplayFrequency->possibleValues.front();
	}

	void Win32GLSupport::setConfigOption(const String &name, const String &value)
	{
		ConfigOptionMap::iterator it = mOptions.find(name);

		// Update
		if(it != mOptions.end())
			it->second.currentValue = value;
		else
		{
            StringUtil::StrStreamType str;
            str << "Option named '" << name << "' does not exist.";
			Except( Exception::ERR_INVALIDPARAMS, str.str(), "Win32GLSupport::setConfigOption" );
		}

		if( name == "Video Mode" )
			refreshConfig();

		if( name == "Full Screen" )
		{
			it = mOptions.find( "Display Frequency" );
			if( value == "No" )
			{
				it->second.currentValue = "N/A";
				it->second.immutable = true;
			}
			else
			{
				it->second.currentValue = it->second.possibleValues.front();
				it->second.immutable = false;
			}
		}
	}

	String Win32GLSupport::validateConfig()
	{
		// TODO, DX9
		return String("");
	}

	RenderWindow* Win32GLSupport::createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle)
	{
		if (autoCreateWindow)
        {
            ConfigOptionMap::iterator opt = mOptions.find("Full Screen");
            if (opt == mOptions.end())
                Except(999, "Can't find full screen options!", "Win32GLSupport::createWindow");
            bool fullscreen = (opt->second.currentValue == "Yes");

            opt = mOptions.find("Video Mode");
            if (opt == mOptions.end())
                Except(999, "Can't find video mode options!", "Win32GLSupport::createWindow");
            String val = opt->second.currentValue;
            String::size_type pos = val.find('x');
            if (pos == String::npos)
                Except(999, "Invalid Video Mode provided", "Win32GLSupport::createWindow");

			unsigned int w = StringConverter::parseUnsignedInt(val.substr(0, pos));
            unsigned int h = StringConverter::parseUnsignedInt(val.substr(pos + 1));

			opt = mOptions.find("Colour Depth");
			if (opt == mOptions.end())
				Except(999, "Can't find Colour Depth options!", "Win32GLSupport::createWindow");
			unsigned int colourDepth = StringConverter::parseUnsignedInt(opt->second.currentValue);

			opt = mOptions.find("VSync");
			if (opt == mOptions.end())
				Except(999, "Can't find VSync options!", "Win32GLSupport::createWindow");
			bool vsync = (opt->second.currentValue == "Yes");
			renderSystem->setWaitForVerticalBlank(vsync);

            return renderSystem->createRenderWindow(windowTitle, w, h, colourDepth, fullscreen);
        }
        else
        {
            // XXX What is the else?
			return NULL;
        }
	}

	RenderWindow* Win32GLSupport::newWindow(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth,
            bool fullScreen, int left, int top, bool depthBuffer, RenderWindow* parentWindowHandle,
			bool vsync)
	{
		ConfigOptionMap::iterator opt = mOptions.find("Display Frequency");
		if (opt == mOptions.end())
			Except(999, "Can't find Colour Depth options!", "Win32GLSupport::newWindow");
		unsigned int displayFrequency = StringConverter::parseUnsignedInt(opt->second.currentValue);

		Win32Window* window = new Win32Window(*this);
		if (!fullScreen && mExternalWindowHandle) // ADD CONTROL IF WE HAVE A WINDOW)
		{
			Win32Window *pWin32Window = (Win32Window *)window;
	 		pWin32Window->setExternalWindowHandle(mExternalWindowHandle);
		}
		window->create(name, width, height, colourDepth, fullScreen, left, top, depthBuffer,
			parentWindowHandle, vsync, displayFrequency);
		return window;
	}

	void Win32GLSupport::start()
	{
		LogManager::getSingleton().logMessage("*** Starting Win32GL Subsystem ***");
	}

	void Win32GLSupport::stop()
	{
		LogManager::getSingleton().logMessage("*** Stopping Win32GL Subsystem ***");
	}

	void Win32GLSupport::initialiseExtensions() {
		// First, initialise the normal extensions
		GLSupport::initialiseExtensions();
		// Welcome to the crazy world of W32 extension handling!
		if(	!checkExtension("WGL_EXT_extensions_string") )
			return;
		// Check for W32 specific extensions probe function
		PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = 
			(PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
		if(!_wglGetExtensionsStringEXT)
			return;
		const char *wgl_extensions = _wglGetExtensionsStringEXT();
        StringUtil::StrStreamType str;
        str << "Supported WGL extensions: " << wgl_extensions;
		LogManager::getSingleton().logMessage(
			LML_NORMAL, str.str());
		// Parse the, and add them to the main list
		std::stringstream ext;
        String instr;
		ext << wgl_extensions;
        while(ext >> instr)
        {
            extensionList.insert(instr);
        }
	}

	void Win32GLSupport::initialiseCapabilities(RenderSystemCapabilities &caps) {
		

		if(	checkExtension("WGL_ARB_pixel_format") &&
			checkExtension("WGL_ARB_render_texture")) {
			// If yes, add rendersystem flag RSC_HWRENDER_TO_TEXTURE	
			caps.setCapability(RSC_HWRENDER_TO_TEXTURE);
		}
	}

	void* Win32GLSupport::getProcAddress(const String& procname)
	{
        return wglGetProcAddress( procname.c_str() );
	}

	void Win32GLSupport::resizeReposition(void* renderTarget)
	{
		Win32Window  *pWin32Window = (Win32Window *)renderTarget;
		if (pWin32Window->getWindowHandle()== m_windowToResize){
					pWin32Window->windowMovedOrResized();
		}

	}

	RenderTexture * Win32GLSupport::createRenderTexture( const String & name, unsigned int width, unsigned int height, TextureType texType,  PixelFormat format ) 
	{
#ifdef HW_RTT
		if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_HWRENDER_TO_TEXTURE))
			return new Win32RenderTexture(*this, name, width, height, texType, format);
		else
#endif
			return new GLRenderTexture(name, width, height, texType, format);
	}

	void Win32GLSupport::pushContext(HDC hdc, HGLRC hglrc) 
	{
		W32Context newCtx = W32Context(hdc, hglrc);
		// We don't care what the outer context is
		if(mContextStack.empty()) {
			mContextStack.push_front(W32Context(0,0));
		} else {
			// Push current ctx to stack
			mContextStack.push_front(mCurrentContext);
		}
		// Set new context as current
		if(mCurrentContext != newCtx) {
			// Optimalisation to prevent superfluous wglMakeCurrent, as those are expensive
			// especially on ATI hardware.
			wglMakeCurrent(hdc, hglrc);
			mCurrentContext = newCtx;
		}
	}

	void Win32GLSupport::popContext() 
	{
		// Push current ctx to stack
		W32Context oldCtx = mContextStack.front();
		mContextStack.pop_front();
		// Check if mCurrentContext is not already equal to the old context, in which case
		// do nothing. Also, see that we don't change the context to (0,0) when going to the most
		// outer context (so that it's still possible to change textures and hardware buffers)
		if(mCurrentContext != oldCtx && oldCtx.first!=0) {
			wglMakeCurrent(oldCtx.first, oldCtx.second);
			mCurrentContext = oldCtx;
		}
	}
}