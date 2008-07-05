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
#include "OgreStableHeaders.h"

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

#include "OgreGLRenderSystem.h"

#include "OgreGLXRenderTexture.h"
#include "OgreGLXContext.h"


#include <iostream>

// Replace by arb ASAP
#ifndef GLX_ATI_pixel_format_float
#define GLX_ATI_pixel_format_float  1
#define GLX_RGBA_FLOAT_ATI_BIT                          0x00000100
#endif


namespace Ogre
{
  
    GLXRenderTexture::GLXRenderTexture( const String & name, uint width, uint height, TextureType texType,  PixelFormat format):
        GLRenderTexture(name, width, height, texType, format),
        _hPBuffer(0),
        mContext(0)
    {
        createPBuffer();
        // Create context
        mContext = new GLXContext(_pDpy, _hPBuffer, _hGLContext);
        // Register the context with the rendersystem
        GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
        rs->_registerContext(this, mContext);
    }

    void GLXRenderTexture::createPBuffer() {        
        LogManager::getSingleton().logMessage(
        "GLXRenderTexture::Creating PBuffer"
        );
           
        _pDpy = glXGetCurrentDisplay();
        ::GLXContext context = glXGetCurrentContext();
        int screen = DefaultScreen(_pDpy);
        XVisualInfo * visInfo = 0;
        int iFormat = 0;
        int iNumFormats;
        int attribs[50];
        int attrib = 0;
        // Attribs for glXChooseFBConfig
        // Get R,G,B,A depths
        int depths[4];
        Image::formatGetDepths(mInternalFormat, depths);

        attribs[attrib++] = GLX_RENDER_TYPE;
        if(Image::formatIsFloat(mInternalFormat))
            attribs[attrib++] = GLX_RGBA_FLOAT_ATI_BIT; // GLX_RGBA_FLOAT_BIT
        else
            attribs[attrib++] = GLX_RGBA_BIT;
        
        attribs[attrib++] = GLX_RED_SIZE;
        attribs[attrib++] = depths[0];
        attribs[attrib++] = GLX_GREEN_SIZE;
        attribs[attrib++] = depths[1];
        attribs[attrib++] = GLX_BLUE_SIZE;
        attribs[attrib++] = depths[2];
        attribs[attrib++] = GLX_ALPHA_SIZE;
        attribs[attrib++] = depths[3];

        attribs[attrib++] = GLX_DRAWABLE_TYPE;
        attribs[attrib++] = GLX_PBUFFER_BIT;
        attribs[attrib++] = GLX_STENCIL_SIZE;
        attribs[attrib++] = 8;
        attribs[attrib++] = GLX_DEPTH_SIZE;
        attribs[attrib++] = 16;             // at least 16
        attribs[attrib++] = GLX_DOUBLEBUFFER;
        attribs[attrib++] = 0;
        attribs[attrib++] = None;
        GLXFBConfig * fbConfigs;
        int nConfigs;
        fbConfigs =
            glXChooseFBConfig(_pDpy, screen, attribs, &nConfigs);
        if (nConfigs == 0 || !fbConfigs) 
            Except(Exception::UNIMPLEMENTED_FEATURE, "glXChooseFBConfig() failed: Couldn't find a suitable pixel format", "GLRenderTexture::createPBuffer");
        // Attribs for CreatePbuffer
        attrib = 0;
        attribs[attrib++] = GLX_PBUFFER_WIDTH;
        attribs[attrib++] = mWidth;
        attribs[attrib++] = GLX_PBUFFER_HEIGHT;
        attribs[attrib++] = mHeight;
        attribs[attrib++] = GLX_PRESERVED_CONTENTS;
        attribs[attrib++] = 1;
        attribs[attrib++] = None;
        // Pick the first returned format that will return a pbuffer
        for (int i = 0; i < nConfigs; i++) {
            _hPBuffer =
                glXCreatePbuffer(_pDpy, fbConfigs[i], attribs);
    
            if (_hPBuffer) {
                visInfo = glXGetVisualFromFBConfig(_pDpy, fbConfigs[0]);
                if (!visInfo) 
                   Except(Exception::UNIMPLEMENTED_FEATURE, "glXGetVisualFromFBConfig() failed: couldn't get an RGBA, double-buffered visual", "GLRenderTexture::createPBuffer");
            
                _hGLContext =
                    glXCreateContext(_pDpy, visInfo, context, True);
                break;
            }
    
        }
        if (!_hPBuffer) 
            Except(Exception::UNIMPLEMENTED_FEATURE, "glXCreatePbuffer() failed", "GLRenderTexture::createPBuffer");
        if (!_hGLContext) 
            Except(Exception::UNIMPLEMENTED_FEATURE, "glXCreateContext() failed", "GLRenderTexture::createPBuffer");        

        if(fbConfigs)
            XFree(fbConfigs);
        if(visInfo)
            XFree(visInfo);
      
        // Query real width and height
        GLuint iWidth, iHeight;
        glXQueryDrawable(_pDpy, _hPBuffer, GLX_WIDTH, &iWidth);
        glXQueryDrawable(_pDpy, _hPBuffer, GLX_HEIGHT, &iHeight);

        LogManager::getSingleton().logMessage(
             LML_NORMAL,
                "GLXRenderTexture::PBuffer created -- Real dimensions %ix%i",iWidth,iHeight
        );
        mWidth = iWidth;  
        mHeight = iHeight;
    }

    GLXRenderTexture::~GLXRenderTexture()
    {
        // Unregister and destroy mContext
        GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
        rs->_unregisterContext(this);
        delete mContext;
        // Destroy GL context
        glXDestroyContext(_pDpy, _hGLContext);
        _hGLContext = 0;
        glXDestroyPbuffer(_pDpy, _hPBuffer);
        _hPBuffer = 0;
    }

/*
    void GLXRenderTexture::_copyToTexture()
    {
        // Should do nothing
    }
 */  
    
  
}