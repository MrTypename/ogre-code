/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright � 2000-2001 Steven J. Streeting
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
#include "OgreRenderTarget.h"

#include "OgreViewport.h"
#include "OgreException.h"
#include "OgreLogManager.h"

namespace Ogre {

    RenderTarget::RenderTarget()
    {
        // Default to no stats display
        mStatFlags = SF_NONE;
        resetStatistics();
    }

    RenderTarget::~RenderTarget()
    {
        // Delete viewports
        for (ViewportList::iterator i = mViewportList.begin();
            i != mViewportList.end(); ++i)
        {
            delete (*i).second;
        }

        // Write closing message
        LogManager::getSingleton().logMessage(
            LML_NORMAL,
            "Render Target '%s' Average FPS: %f Best FPS: %f Worst FPS: %f", 
            mName.c_str(), mAvgFPS, mBestFPS, mWorstFPS );

    }

    String RenderTarget::getName(void)
    {
        return mName;
    }


    void RenderTarget::getMetrics(int &width, int &height, int &colourDepth)
    {
        width = mWidth;
        height = mHeight;
        colourDepth = mColourDepth;
    }

    int RenderTarget::getWidth(void)
    {
        return mWidth;
    }
    int RenderTarget::getHeight(void)
    {
        return mHeight;
    }
    int RenderTarget::getColourDepth(void)
    {
        return mColourDepth;
    }

    void RenderTarget::update(void)
    {
        mTris = 0;
        // Go through viewports in Z-order
        // Tell each to refresh
        ViewportList::iterator it = mViewportList.begin();
        while (it != mViewportList.end())
        {
            (*it).second->update();
            mTris += (*it++).second->_getNumRenderedFaces();

        }

        // Update statistics
        updateStats();

    }

    Viewport* RenderTarget::addViewport(Camera* cam, int ZOrder, float left, float top ,
        float width , float height)
    {
        char msg[100];
        // Check no existing viewport with this Z-order
        ViewportList::iterator it = mViewportList.find(ZOrder);

        if (it != mViewportList.end())
        {
            sprintf(msg, "Can't create another viewport for %s with Z-Order %i "
                " because a viewport exists with this Z-Order already.",
                this->getName().c_str(), ZOrder);
            Except(9999, msg, "RenderTarget::addViewport");
        }
        // Add viewport to list
        // Order based on Z-Order
        Viewport* vp = new Viewport(cam, this, left, top, width, height, ZOrder);

        mViewportList.insert(ViewportList::value_type(ZOrder, vp));

        return vp;
    }

    void RenderTarget::removeViewport(int ZOrder)
    {
        ViewportList::iterator it = mViewportList.find(ZOrder);

        if (it != mViewportList.end())
        {
            delete (*it).second;
            mViewportList.erase(ZOrder);
        }
    }

    void RenderTarget::removeAllViewports(void)
    {


        for (ViewportList::iterator it = mViewportList.begin(); it != mViewportList.end(); ++it)
        {
            delete (*it).second;
        }

        mViewportList.clear();

    }
    void RenderTarget::setStatsDisplay(StatFlags sf)
    {
        mStatFlags = sf;

    }

    void RenderTarget::getStatistics(float& lastFPS, float& avgFPS,
            float& bestFPS, float& worstFPS)
    {

        // Note - the will have been updated by the last render
        lastFPS = mLastFPS;
        avgFPS = mAvgFPS;
        bestFPS = mBestFPS;
        worstFPS = mWorstFPS;


    }

    float RenderTarget::getLastFPS()
    {
        return mLastFPS;
    }
    float RenderTarget::getAverageFPS()
    {
        return mAvgFPS;
    }
    float RenderTarget::getBestFPS()
    {
        return mBestFPS;
    }
    float RenderTarget::getWorstFPS()
    {
        return mWorstFPS;
    }


    void RenderTarget::resetStatistics(void)
    {
        mAvgFPS = 0.0;
        mBestFPS = 0.0;
        mLastFPS = 0.0;
        mLastFPS = 0.0;
        mWorstFPS = 999.0;
    }

    void RenderTarget::updateStats(void)
    {
        static float lastTime = 0.0f;
        static long numFrames  = 0;

        // Keep track of the time lapse and frame count
        float fTime = ((float)clock())/CLOCKS_PER_SEC; // Get current time in seconds
        ++numFrames;

        // Update the frame rate once per second
        if( fTime - lastTime > 1.0f )
        {
            // Don't update stats first time (first time includes init times)
            if (lastTime)
            {
                mLastFPS = numFrames / (fTime - lastTime);
                if (mAvgFPS == 0)
                    mAvgFPS = mLastFPS;
                else
                    mAvgFPS = (mAvgFPS + mLastFPS) / 2;

                if (mBestFPS < mLastFPS)
                    mBestFPS = mLastFPS;

                if (mWorstFPS > mLastFPS)
                    mWorstFPS = mLastFPS;
            }


            lastTime = fTime;
            numFrames  = 0;
        }

        // Display the stats
        char strStats[255];
        int iLine;

        iLine = 0;

        // HACK
        // Prevents inexplicable framerate drop in some cases when frame counters are turned off??!?!?
        outputText(0,512, "");

        if (mStatFlags != SF_NONE)
        {
            if (mStatFlags & SF_FPS)
            {
                sprintf(strStats, "Current FPS: %.2f", mLastFPS);
                outputText(0,iLine*16,strStats);
                iLine++;
            }
            if (mStatFlags & SF_AVG_FPS)
            {
                sprintf(strStats, "Average FPS: %.2f", mAvgFPS);
                outputText(0,iLine*16,strStats);
                iLine++;
            }
            if (mStatFlags & SF_BEST_FPS)
            {
                sprintf(strStats, "Best FPS: %.2f", mBestFPS);
                outputText(0,iLine*16,strStats);
                iLine++;
            }
            if (mStatFlags & SF_WORST_FPS)
            {
                sprintf(strStats, "Worst FPS: %.2f", mWorstFPS);
                outputText(0,iLine*16,strStats);
                iLine++;
            }
            if (mStatFlags & SF_TRIANGLE_COUNT)
            {
                sprintf(strStats, "Tris: %d", mTris);
                outputText(0,iLine*16,strStats);
                iLine++;
            }
        }

        // Output debug text
        outputText(0, iLine*16, mDebugText);

    }

    void RenderTarget::getCustomAttribute(String name, void* pData)
    {
        Except(Exception::ERR_INVALIDPARAMS, "Attribute not found.", "RenderTarget::getCustomAttribute");
    }
    //-----------------------------------------------------------------------
    void RenderTarget::setDebugText(const String& text)
    {
        mDebugText = text;
    }

}        