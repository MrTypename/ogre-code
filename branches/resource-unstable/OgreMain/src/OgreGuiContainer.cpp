
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"

#include "OgreGuiContainer.h"
#include "OgreException.h"
#include "OgreMouseEvent.h"
#include "OgreGuiManager.h"

namespace Ogre {

    //---------------------------------------------------------------------
    GuiContainer::GuiContainer(const String& name)
        : OverlayElement(name),
		mChildrenProcessEvents(true)
    {
    }
    //---------------------------------------------------------------------
    GuiContainer::~GuiContainer()
    {
        GuiContainer::ChildIterator ci = getChildIterator();
        while (ci.hasMoreElements())
        {
            OverlayElement* child = ci.getNext();
            child->_setParent(0);
        }
    }
    //---------------------------------------------------------------------
    void GuiContainer::addChild(OverlayElement* elem)
    {
        if (elem->isContainer())
		{
			addChildImpl(static_cast<GuiContainer*>(elem));
		}
		else
		{
			addChildImpl(elem);
		}

	}
    //---------------------------------------------------------------------
    void GuiContainer::addChildImpl(OverlayElement* elem)
    {
        String name = elem->getName();
        ChildMap::iterator i = mChildren.find(name);
        if (i != mChildren.end())
        {
            Except(Exception::ERR_DUPLICATE_ITEM, "Child with name " + name + 
                " already defined.", "GuiContainer::addChild");
        }

        mChildren.insert(ChildMap::value_type(name, elem));
        // tell child about parent & ZOrder
        elem->_notifyParent(this, mOverlay);
	    elem->_notifyZOrder(mZOrder + 1);
	    elem->_notifyWorldTransforms(mXForm);
	    elem->_notifyViewport();

    }
    //---------------------------------------------------------------------
    void GuiContainer::addChildImpl(GuiContainer* cont)
    {
        // Add to main map first 
        // This will pick up duplicates
        OverlayElement* pElem = cont;
        addChildImpl(pElem);

        /*
        cont->_notifyParent(this, mOverlay);
        cont->_notifyZOrder(mZOrder + 1);
	    cont->_notifyWorldTransforms(mXForm);

		// tell children of new container the current overlay
        ChildIterator it = cont->getChildIterator();
        while (it.hasMoreElements())
        {
            // Give children ZOrder 1 higher than this
            GuiElement* pElemChild = it.getNext();
			pElemChild->_notifyParent(cont, mOverlay);
            pElemChild->_notifyZOrder(cont->getZOrder() + 1);
    	    pElemChild->_notifyWorldTransforms(mXForm);
        }
        */

        // Now add to specific map too
        mChildContainers.insert(ChildContainerMap::value_type(cont->getName(), cont));

    }
    //---------------------------------------------------------------------
    void GuiContainer::removeChild(const String& name)
    {
        ChildMap::iterator i = mChildren.find(name);
        if (i == mChildren.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Child with name " + name + 
                " not found.", "GuiContainer::removeChild");
        }

        OverlayElement* element = i->second;
        mChildren.erase(i);

            // remove from container list (if found)
        ChildContainerMap::iterator j = mChildContainers.find(name);
        if (j != mChildContainers.end())
            mChildContainers.erase(j);

        element->_setParent(0);
    }
    //---------------------------------------------------------------------
    void GuiContainer::_addChild(OverlayElement* elem)
    {
        if (elem->isContainer())
		{
			addChildImpl(static_cast<GuiContainer*>(elem));
		}
		else
		{
			addChildImpl(elem);
		}
	}
    //---------------------------------------------------------------------
    void GuiContainer::_removeChild(const String& name)
    {
        ChildMap::iterator i = mChildren.find(name);
        if (i == mChildren.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Child with name " + name + 
                " not found.", "GuiContainer::removeChild");
        }

        OverlayElement* element = i->second;
        mChildren.erase(i);

            // remove from container list (if found)
        ChildContainerMap::iterator j = mChildContainers.find(name);
        if (j != mChildContainers.end())
            mChildContainers.erase(j);

        element->_setParent(0);
    }
    //---------------------------------------------------------------------
    OverlayElement* GuiContainer::getChild(const String& name)
    {
        ChildMap::iterator i = mChildren.find(name);
        if (i == mChildren.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Child with name " + name + 
                " not found.", "GuiContainer::getChild");
        }

        return i->second;
    }
    //---------------------------------------------------------------------
    GuiContainer::ChildIterator GuiContainer::getChildIterator(void)
    {
        return ChildIterator(mChildren.begin(), mChildren.end());
    }
    //---------------------------------------------------------------------
    GuiContainer::ChildContainerIterator GuiContainer::getChildContainerIterator(void)
    {
        return ChildContainerIterator(mChildContainers.begin(), mChildContainers.end());
    }

    //---------------------------------------------------------------------
	void GuiContainer::_positionsOutOfDate(void)
	{
		OverlayElement::_positionsOutOfDate();

        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
			it.getNext()->_positionsOutOfDate();
        }
	}

    //---------------------------------------------------------------------
    void GuiContainer::_update(void)
    {
        // call superclass
        OverlayElement::_update();

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            it.getNext()->_update();
        }


    }
    //---------------------------------------------------------------------
    void GuiContainer::_notifyZOrder(ushort newZOrder)
    {
        OverlayElement::_notifyZOrder(newZOrder);

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            // Give children ZOrder 1 higher than this
            it.getNext()->_notifyZOrder(newZOrder + 1);
        }
    }
    //---------------------------------------------------------------------
    void GuiContainer::_notifyWorldTransforms(const Matrix4& xform)
    {
        OverlayElement::_notifyWorldTransforms(xform);

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            it.getNext()->_notifyWorldTransforms(xform);
        }
    }
    //---------------------------------------------------------------------
    void GuiContainer::_notifyViewport()
    {
        OverlayElement::_notifyViewport();

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            it.getNext()->_notifyViewport();
        }
    }
    //---------------------------------------------------------------------
    void GuiContainer::_notifyParent(GuiContainer* parent, Overlay* overlay)
    {
        OverlayElement::_notifyParent(parent, overlay);

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            // Notify the children of the overlay 
            it.getNext()->_notifyParent(this, overlay);
        }
    }

    //---------------------------------------------------------------------
    void GuiContainer::_updateRenderQueue(RenderQueue* queue)
    {
        if (mVisible)
        {

            OverlayElement::_updateRenderQueue(queue);

            // Also add children
            ChildIterator it = getChildIterator();
            while (it.hasMoreElements())
            {
                // Give children ZOrder 1 higher than this
                it.getNext()->_updateRenderQueue(queue);
            }
        }

    }


	OverlayElement* GuiContainer::findElementAt(Real x, Real y) 		// relative to parent
	{

		OverlayElement* ret = NULL;

		int currZ = -1;

		if (mVisible)
		{
			ret = OverlayElement::findElementAt(x,y);	//default to the current container if no others are found
			if (ret && mChildrenProcessEvents)
			{
				ChildIterator it = getChildIterator();
				while (it.hasMoreElements())
				{
					OverlayElement* currentOverlayElement = it.getNext();
					if (currentOverlayElement->isVisible() && currentOverlayElement->isEnabled())
					{
						int z = currentOverlayElement->getZOrder();
						if (z > currZ)
						{
							OverlayElement* elementFound = currentOverlayElement->findElementAt(x ,y );
							if (elementFound)
							{
								currZ = z;
								ret = elementFound;
							}
						}
					}
				}
			}
		}
		return ret;
	}

    void GuiContainer::copyFromTemplate(OverlayElement* templateGui)
    {
        OverlayElement::copyFromTemplate(templateGui);

		    if (templateGui->isContainer() && isContainer())
		    {
    	     GuiContainer::ChildIterator it = static_cast<GuiContainer*>(templateGui)->getChildIterator();
  		     while (it.hasMoreElements())
			     {
				       OverlayElement* oldChildElement = it.getNext();
				       if (oldChildElement->isCloneable())
				       {
                   OverlayElement* newChildElement = 
                       GuiManager::getSingleton().createOverlayElement(oldChildElement->getTypeName(), mName+"/"+oldChildElement->getName());
                   oldChildElement->copyParametersTo(newChildElement);
                   addChild((GuiContainer*)newChildElement);
               }
           }
        }
    }

    OverlayElement* GuiContainer::clone(const String& instanceName)
    {
        GuiContainer *newContainer;

        newContainer = static_cast<GuiContainer*>(OverlayElement::clone(instanceName));

    	  ChildIterator it = getChildIterator();
  		  while (it.hasMoreElements())
			  {
				    OverlayElement* oldChildElement = it.getNext();
				    if (oldChildElement->isCloneable())
				    {
                OverlayElement* newChildElement = oldChildElement->clone(instanceName);
                newContainer->_addChild(newChildElement);
            }
        }

        return newContainer;
    }

}
