
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

#include "OgreKeyEvent.h"
#include "OgreEventListeners.h"
#include "OgreKeyTarget.h"	// HACK move higher
#include "OgreEventMulticaster.h"	



namespace Ogre {


	KeyTarget::KeyTarget() 
	{
		mKeyListener = 0;
	}
    //-----------------------------------------------------------------------

	void KeyTarget::processKeyEvent(KeyEvent* e) 
	{
		KeyListener* listener = mKeyListener;
		if (listener != NULL) 
		{
			int id = e->getID();
			switch(id) 
			{
			case KeyEvent::KE_KEY_PRESSED:
				listener->keyPressed(e);
				break;
			case KeyEvent::KE_KEY_RELEASED:
				listener->keyReleased(e);
				break;
			case KeyEvent::KE_KEY_CLICKED:
				listener->keyClicked(e);
				break;
			}
		}
	}

	void KeyTarget::addKeyListener(KeyListener* l) 
	{
		if (l == NULL) 
		{
			return;
		}
		mKeyListener = EventMulticaster::add(mKeyListener,l);
	}

	void KeyTarget::removeKeyListener(KeyListener* l) 
	{
		if (l == NULL) 
		{
			return;
		}
		mKeyListener = EventMulticaster::remove(mKeyListener,l);
	}
}
