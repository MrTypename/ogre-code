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
#include "OgreStableHeaders.h"
#include "OgreCompositorManager.h"
#include "OgreCompositor.h"
#include "OgreCompositorChain.h"
#include "OgreCompositionPass.h"
#include "OgreCompositionTargetPass.h"
#include "OgreCompositionTechnique.h"

namespace Ogre {

template<> CompositorManager* Singleton<CompositorManager>::ms_Singleton = 0;
//-----------------------------------------------------------------------
CompositorManager::CompositorManager():
	mRectangle(0)
{
	initialise();
}
//-----------------------------------------------------------------------
CompositorManager::~CompositorManager()
{
    freeChains();
	delete mRectangle;
}
//-----------------------------------------------------------------------
Resource* CompositorManager::createImpl(const String& name, ResourceHandle handle, 
    const String& group, bool isManual, ManualResourceLoader* loader,
    const NameValuePairList* params)
{
    return new Compositor(this, name, handle, group, isManual, loader);
}
//-----------------------------------------------------------------------
void CompositorManager::initialise(void)
{
    /// Create "default" compositor
    /** Compositor that is used to implicitly represent the original
        render in the chain. This is an identity compositor with only an output pass:
    compositor Ogre/Scene
    {
        technique
        {
            target_output
            {
				pass clear
				{
					/// Clear frame
				}
                pass render_scene
                {
					visibility_mask FFFFFFFF
					render_queues SKIES_EARLY SKIES_LATE
                }
            }
        }
    };
    */
    CompositorPtr scene = create("Ogre/Scene", ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
    CompositionTechnique *t = scene->createTechnique();
    CompositionTargetPass *tp = t->getOutputTargetPass();
    tp->setVisibilityMask(0xFFFFFFFF);
	{
		CompositionPass *pass = tp->createPass();
		pass->setType(CompositionPass::PT_CLEAR);
	}
	{
		CompositionPass *pass = tp->createPass();
		pass->setType(CompositionPass::PT_RENDERSCENE);
		/// Render everything, including skies
		pass->setFirstRenderQueue(RENDER_QUEUE_SKIES_EARLY);
		pass->setLastRenderQueue(RENDER_QUEUE_SKIES_LATE);
	}
	
}
//-----------------------------------------------------------------------
void CompositorManager::parseScript(DataStreamPtr& stream, const String& groupName)
{
}
//-----------------------------------------------------------------------
CompositorChain *CompositorManager::getCompositorChain(Viewport *vp)
{
    Chains::iterator i=mChains.find(vp);
    if(i != mChains.end())
    {
        return i->second;
    }
    else
    {
        CompositorChain *chain = new CompositorChain(vp);
        mChains[vp] = chain;
        return chain;
    }
}
//-----------------------------------------------------------------------
void CompositorManager::freeChains()
{
    Chains::iterator i, iend=mChains.end();
    for(i=mChains.begin(); i!=iend;++i)
    {
        delete i->second;
    }
    mChains.clear();
}

Renderable *CompositorManager::_getTexturedRectangle2D()
{
	if(!mRectangle)
	{
		/// 2D rectangle, to use for render_quad passes
		mRectangle = new Rectangle2D(true);
		mRectangle->setCorners(-1,1,1,-1);
	}
	return mRectangle;
}
//-----------------------------------------------------------------------
CompositorInstance *CompositorManager::addCompositor(Viewport *vp, const String &compositor, int addPosition)
{
	CompositorPtr comp = getByName(compositor);
	if(comp.isNull())
		return 0;
	CompositorChain *chain = getCompositorChain(vp);
	return chain->addCompositor(comp, addPosition==-1?CompositorChain::LAST:(size_t)addPosition);
}
//-----------------------------------------------------------------------
void CompositorManager::removeCompositor(Viewport *vp, const String &compositor)
{
	CompositorChain *chain = getCompositorChain(vp);
	CompositorChain::InstanceIterator it = chain->getCompositors();
	for(size_t pos=0; pos < chain->getNumCompositors(); ++pos)
	{
		CompositorInstance *instance = chain->getCompositor(pos);
		if(instance->getCompositor()->getName() == compositor)
		{
			chain->removeCompositor(pos);
			break;
		}
	}
}
//-----------------------------------------------------------------------
void CompositorManager::setCompositorEnabled(Viewport *vp, const String &compositor, bool value)
{
	CompositorChain *chain = getCompositorChain(vp);
	CompositorChain::InstanceIterator it = chain->getCompositors();
	for(size_t pos=0; pos < chain->getNumCompositors(); ++pos)
	{
		CompositorInstance *instance = chain->getCompositor(pos);
		if(instance->getCompositor()->getName() == compositor)
		{
			chain->setCompositorEnabled(pos, value);
			break;
		}
	}
}

}
