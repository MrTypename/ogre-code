#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#include "OgreSDLGLSupport.h"

#include "OgreSDLWindow.h"

using namespace Ogre;

SDLGLSupport::SDLGLSupport()
{

    SDL_Init(SDL_INIT_VIDEO);
}

SDLGLSupport::~SDLGLSupport()
{
}

void SDLGLSupport::addConfig(void)
{
    mVideoModes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_OPENGL);
    
    if (mVideoModes == (SDL_Rect **)0)
    {
        Except(999, "Unable to load video modes",
                "SDLRenderSystem::initConfigOptions");
    }

    ConfigOption optFullScreen;
    ConfigOption optVideoMode;
    ConfigOption optFSAA;

    // FS setting possiblities
    optFullScreen.name = "Full Screen";
    optFullScreen.possibleValues.push_back("Yes");
    optFullScreen.possibleValues.push_back("No");
    optFullScreen.currentValue = "Yes";
    optFullScreen.immutable = false;

    // Video mode possiblities
    optVideoMode.name = "Video Mode";
    optVideoMode.immutable = false;
    for (size_t i = 0; mVideoModes[i]; i++)
    {
        char szBuf[16];
		snprintf(szBuf, 16, "%d x %d", mVideoModes[i]->w, mVideoModes[i]->h);
        optVideoMode.possibleValues.push_back(szBuf);
        // Make the first one default
        if (i == 0)
        {
            optVideoMode.currentValue = szBuf;
        }
    }
    
    //FSAA possibilities
    optFSAA.name = "FSAA";
    optFSAA.possibleValues.push_back("0");
    optFSAA.possibleValues.push_back("2");
    optFSAA.possibleValues.push_back("4");
    optFSAA.possibleValues.push_back("6");
    optFSAA.currentValue = "0";
    optFSAA.immutable = false;
    
    mOptions[optFullScreen.name] = optFullScreen;
    mOptions[optVideoMode.name] = optVideoMode;
    mOptions[optFSAA.name] = optFSAA;
}

String SDLGLSupport::validateConfig(void)
{
    return String("");
}

RenderWindow* SDLGLSupport::createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle)
{
	if (autoCreateWindow)
    {
        ConfigOptionMap::iterator opt = mOptions.find("Full Screen");
        if (opt == mOptions.end())
            Except(999, "Can't find full screen options!", "SDLGLSupport::createWindow");
        bool fullscreen = (opt->second.currentValue == "Yes");

        opt = mOptions.find("Video Mode");
        if (opt == mOptions.end())
            Except(999, "Can't find video mode options!", "SDLGLSupport::createWindow");
        String val = opt->second.currentValue;
        String::size_type pos = val.find('x');
        if (pos == String::npos)
            Except(999, "Invalid Video Mode provided", "SDLGLSupport::createWindow");

        int fsaa_x_samples = 0;
        opt = mOptions.find("FSAA");
        if(opt != mOptions.end()) //check for FSAA parameter, if not ignore it...
        {
            fsaa_x_samples = StringConverter::parseInt(opt->second.currentValue);
            if(fsaa_x_samples>1) {
                // If FSAA is enabled in the parameters, enable the MULTISAMPLEBUFFERS
                // and set the number of samples before the render window is created.
                SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,1);
                SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,fsaa_x_samples);
            }
        }

        unsigned int w = StringConverter::parseUnsignedInt(val.substr(0, pos));
        unsigned int h = StringConverter::parseUnsignedInt(val.substr(pos + 1));

        const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();
        return renderSystem->createRenderWindow(windowTitle, w, h, videoInfo->vfmt->BitsPerPixel, fullscreen);
    }
    else
    {
        // XXX What is the else?
		return NULL;
    }
}

RenderWindow* SDLGLSupport::newWindow(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth,
        bool fullScreen, int left, int top, bool depthBuffer, RenderWindow* parentWindowHandle,
		bool vsync)
{
	SDLWindow* window = new SDLWindow();
	window->create(name, width, height, colourDepth, fullScreen, left, top, depthBuffer,
		parentWindowHandle);
	return window;
}

void SDLGLSupport::start()
{
    LogManager::getSingleton().logMessage(
        "******************************\n"
        "*** Starting SDL Subsystem ***\n"
        "******************************");

    SDL_Init(SDL_INIT_VIDEO);
}

void SDLGLSupport::stop()
{
    LogManager::getSingleton().logMessage(
        "******************************\n"
        "*** Stopping SDL Subsystem ***\n"
        "******************************");

    SDL_Quit();
}

void* SDLGLSupport::getProcAddress(const String& procname)
{
    return SDL_GL_GetProcAddress(procname.c_str());
}