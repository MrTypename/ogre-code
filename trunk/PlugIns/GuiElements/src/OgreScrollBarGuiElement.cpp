/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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

#include "OgreScrollBarGuiElement.h"	
#include "OgreStringConverter.h"	
#include "OgreGuiManager.h"	
#include "OgreResource.h"	
#include "OgreException.h"	


namespace Ogre {

    String ScrollBarGuiElement::msTypeName = "ScrollBar";
	ScrollBarGuiElement::CmdUpButton ScrollBarGuiElement::msCmdUpButton;
	ScrollBarGuiElement::CmdDownButton ScrollBarGuiElement::msCmdDownButton;
	ScrollBarGuiElement::CmdScrollBit ScrollBarGuiElement::msCmdScrollBit;

	ScrollBarGuiElement::ScrollBarGuiElement(const String& name) :
		PanelGuiElement(name)
	{
        if (createParamDictionary("ScrollBarGuiElement"))
        {
            addBaseParameters();
        }
		mUpButton = 0;
		mDownButton = 0;
		mScrollBit = 0;
		mTotalItems = 0;
		mStartingItem = 0;
		mVisibilityRange = 0;
		mouseHeldAtY = -1;

		mSpacing = 0.001;
	}

    //---------------------------------------------------------------------
    void ScrollBarGuiElement::addBaseParameters(void)
    {
        PanelGuiElement::addBaseParameters();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("up_button", 
            "The template of Up Button."
            , PT_STRING),
            &ScrollBarGuiElement::msCmdUpButton);

        dict->addParameter(ParameterDef("down_button", 
            "The template of Down Button."
            , PT_STRING),
            &ScrollBarGuiElement::msCmdDownButton);

        dict->addParameter(ParameterDef("scroll_bit", 
            "The template of Scroll Bit."
            , PT_STRING),
            &ScrollBarGuiElement::msCmdScrollBit);
	}
    //---------------------------------------------------------------------
    // Command objects
    //---------------------------------------------------------------------

    //-----------------------------------------------------------------------
    String ScrollBarGuiElement::CmdUpButton::doGet(void* target)
    {
        return static_cast<ScrollBarGuiElement*>(target)->getUpButtonName();
    }
    void ScrollBarGuiElement::CmdUpButton::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<ScrollBarGuiElement*>(target)->setUpButtonName(val);
    }
    //-----------------------------------------------------------------------
    String ScrollBarGuiElement::CmdDownButton::doGet(void* target)
    {
        return static_cast<ScrollBarGuiElement*>(target)->getDownButtonName();
    }
    void ScrollBarGuiElement::CmdDownButton::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<ScrollBarGuiElement*>(target)->setDownButtonName(val);
    }
    //-----------------------------------------------------------------------
    String ScrollBarGuiElement::CmdScrollBit::doGet(void* target)
    {
        return static_cast<ScrollBarGuiElement*>(target)->getScrollBitName();
    }
    void ScrollBarGuiElement::CmdScrollBit::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<ScrollBarGuiElement*>(target)->setScrollBitName(val);
    }
    //-----------------------------------------------------------------------

	String ScrollBarGuiElement::getUpButtonName()
	{
		return mUpButtonName;
	}
	String ScrollBarGuiElement::getDownButtonName()
	{
		return mDownButtonName;
	}
	String ScrollBarGuiElement::getScrollBitName()
	{
		return mScrollBitName;
	}
//-----------------------------------------------------------------------

	void ScrollBarGuiElement::setUpButtonName(const String& val)
	{
		mUpButtonName = val;
		Real buttonSize = getWidth();
		mUpButton = static_cast<BorderButtonGuiElement*> (
			GuiManager::getSingleton().createGuiElementFromTemplate(mUpButtonName, "", mName + "/" + "UpButton"));


		// do not make this cloneable, otherwise there will be 2 copies of it when it is cloned,
		// one copy when the children are copied, and another copy when setUpButtonName is set.
		mUpButton->setCloneable(false);

		addChild(mUpButton);
//		mUpButton->setButtonCaption("SS/Templates/BasicText", "UP");
		mUpButton->addActionListener(this);
	}
	void ScrollBarGuiElement::setDownButtonName(const String& val)
	{
		Real buttonSize = getWidth();
		mDownButtonName = val;
		mDownButton = static_cast<BorderButtonGuiElement*> (
			GuiManager::getSingleton().createGuiElementFromTemplate(mDownButtonName, "", mName + "/" + "DownButton"));

		// do not make this cloneable, otherwise there will be 2 copies of it when it is cloned,
		// one copy when the children are copied, and another copy when setDownButtonName is set.
		mDownButton->setCloneable(false);
		addChild(mDownButton);
//		mDownButton->setButtonCaption("SS/Templates/BasicText", "DOWN");
		mDownButton->addActionListener(this);
	}
	void ScrollBarGuiElement::setScrollBitName(const String& val)
	{
		Real buttonSize = getWidth();
		mScrollBitName = val;
		mScrollBit = static_cast<PanelGuiElement*> (
			GuiManager::getSingleton().createGuiElementFromTemplate(mScrollBitName, "", mName + "/" + "ScrollBit"));
		// do not make this cloneable, otherwise there will be 2 copies of it when it is cloned,
		// one copy when the children are copied, and another copy when setScrollBitName is set.
		mScrollBit->setCloneable(false);
		mScrollBit->addMouseMotionListener(this);
		mScrollBit->addMouseListener(this);
		addMouseListener(this);

		addChild(mScrollBit);

	}
//-----------------------------------------------------------------------

	void ScrollBarGuiElement::setLimits(int first, int visibleRange, int total)
	{
		mTotalItems = total;
		mStartingItem = first;
		mVisibilityRange = visibleRange;

		layoutItems();

	}
	void ScrollBarGuiElement::layoutItems()
	{
		Real buttonSize = getWidth();

		mUpButton->setLeft(mSpacing);
		mUpButton->setTop(mSpacing);
		mUpButton->setWidth(buttonSize - mSpacing*2);
		mUpButton->setHeight(buttonSize - mSpacing*2);	// buttons are square

		mDownButton->setLeft(mSpacing);
		mDownButton->setTop(getHeight() - buttonSize + mSpacing);
		mDownButton->setWidth(buttonSize - mSpacing*2);
		mDownButton->setHeight(buttonSize - mSpacing*2);	// buttons are square

		mScrollBit->setLeft(mSpacing );
		mScrollBit->setTop(buttonSize + mSpacing);
		mScrollBit->setWidth(buttonSize - mSpacing*2);
		mScrollBit->setHeight(getHeight() - 2*buttonSize - mSpacing*2);	// buttons are square
		if (mTotalItems == 0 )
		{

			mScrollBit->setTop(buttonSize + mSpacing );
			mScrollBit->setHeight((getHeight() - 2*buttonSize - mSpacing*2) );			

		}
		else
		{
			mScrollBit->setTop(buttonSize + mSpacing + (getHeight() - 2*buttonSize - mSpacing*2) * mStartingItem / mTotalItems );
			mScrollBit->setHeight((getHeight() - 2*buttonSize - mSpacing*2) * mVisibilityRange / mTotalItems );			
		}
	}

    //---------------------------------------------------------------------
    const String& ScrollBarGuiElement::getTypeName(void)
    {
        return msTypeName;
    }

	void ScrollBarGuiElement::actionPerformed(ActionEvent* e) 
	{
		if (e->getActionCommand() == mUpButton->getName())
		{
			if (mStartingItem >0)
			{
				mStartingItem--;
				fireScrollPerformed();

			}
		}
		else if (e->getActionCommand() == mDownButton->getName())
		{
			if (mStartingItem < mTotalItems-mVisibilityRange)
			{
				mStartingItem++;
				fireScrollPerformed();

			}
		}


	}
    //-----------------------------------------------------------------------
	void ScrollBarGuiElement::fireScrollPerformed()
	{
		ScrollEvent* se = new ScrollEvent(this, ScrollEvent::SE_SCROLL_PERFORMED, 0, 0, mStartingItem, mVisibilityRange, mTotalItems);
		processEvent(se);
		delete se;
	}

    //-----------------------------------------------------------------------
	void ScrollBarGuiElement::processEvent(InputEvent* e) 
	{
		PanelGuiElement::processEvent(e);

		if (!e->isConsumed())
		{
			switch(e->getID()) 
			{
			case ScrollEvent::SE_SCROLL_PERFORMED:
				processScrollEvent(static_cast<ScrollEvent*>(e));
				break;
			default:
				break;
			}
		}
	}
	void ScrollBarGuiElement::mouseMoved(MouseEvent* e)
	{


	}
	void ScrollBarGuiElement::mouseDragged(MouseEvent* e)
	{

		if (mouseHeldAtY == -1)
		{
			int err =1;

		}
		else
		{
			Real moveY = e->getY() - mouseHeldAtY + mScrollBit->getTop();
			moveScrollBitTo(moveY);
		}

	}
	void ScrollBarGuiElement::mousePressed(MouseEvent* e) 
	{
		Real buttonSize = getWidth();
		Real mouseY = e->getY() - mDerivedTop;
		if ((MouseTarget*)e->getSource() == (GuiElement*)(mScrollBit))
		{
			mouseHeldAtY = mouseY;
		}
		else if ((MouseTarget*)e->getSource() == (GuiElement*)this)
		{
			Real topToScroll = mouseY;
			if (mouseY > mScrollBit->getTop())
			{
				// always take scroll point from the top of scrollBit
				topToScroll -= mScrollBit->getHeight();

			}

			moveScrollBitTo(topToScroll - buttonSize + mSpacing);

			mouseHeldAtY = mouseY;

		}

	}
	void ScrollBarGuiElement::mouseReleased(MouseEvent* e) 
	{
		mouseHeldAtY = -1;

	}

	void ScrollBarGuiElement::moveScrollBitTo(Real moveY)
	{
		Real buttonSize = getWidth();
		if (moveY <0)
		{
			moveY = 0;
		}
		if (moveY > getHeight() - 2*buttonSize - mSpacing*2 - mScrollBit->getHeight())
		{
			moveY = getHeight() - 2*buttonSize - mSpacing*2 - mScrollBit->getHeight();
		}
		mScrollBit->setTop(buttonSize + mSpacing + moveY);
		mStartingItem = ((mScrollBit->getTop() - buttonSize - mSpacing) * mTotalItems) / (getHeight() - 2*buttonSize - mSpacing*2);
		fireScrollPerformed();
	}
}

