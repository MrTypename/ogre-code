/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2002 The OGRE Team
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

#include "OgreString.h"
#include "OgreGuiManager.h"
#include "OgreGuiElementFactories.h"
#include "OgreException.h"
#include "OgreStringVector.h"
#include "OgreRoot.h"
#include "OgreCursorGuiElement.h"
#include "OgreOverlayManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    GuiElementFactory* pPanelFactory = NULL;
    GuiElementFactory* pBorderPanelFactory = NULL;
    GuiElementFactory* pTextAreaFactory = NULL;
    GuiElementFactory* pButtonFactory = NULL;
    GuiElementFactory* pListFactory = NULL;
	CursorGuiElement* pCursorGui = NULL;
    //-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    extern "C" void dllStartPlugin(void)
    {
        SET_TERM_HANDLER;

        pPanelFactory = new PanelGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pPanelFactory);

        pBorderPanelFactory = new BorderPanelGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pBorderPanelFactory);
        
        pTextAreaFactory = new TextAreaGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pTextAreaFactory);

		pButtonFactory = new ButtonGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pButtonFactory);

		pListFactory = new ListGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pListFactory);

		pCursorGui = new CursorGuiElement("Cursor default");
		OverlayManager::getSingleton().setCursorGui(pCursorGui, pCursorGui);

    }

    //-----------------------------------------------------------------------
    extern "C" void dllStopPlugin(void)
    {
        delete pPanelFactory;
        delete pBorderPanelFactory;
        delete pTextAreaFactory;
		delete pButtonFactory;
		delete pListFactory;
		delete pCursorGui;

    }

}