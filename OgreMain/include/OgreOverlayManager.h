/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef __OverlayManager_H__
#define __OverlayManager_H__

#include <set>
#include "OgrePrerequisites.h"
#include "OgreEventDispatcher.h"
#include "OgreEventListeners.h"
#include "OgreEventTarget.h"
#include "OgreSingleton.h"
#include "OgreStringVector.h"
#include "OgreEventDispatcher.h"
#include "OgreTargetManager.h"
#include "OgreOverlay.h"
#include "OgreScriptLoader.h"


namespace Ogre {

    /** Manages Overlay objects, parsing them from .overlay files and
        storing a lookup library of them.
    */
    class _OgreExport OverlayManager : public Singleton<OverlayManager>, 
        public ScriptLoader, public TargetManager, public EventTarget
    {
    public:
        typedef std::map<String, Overlay*> OverlayMap;
    protected:
        OverlayMap mOverlayMap;
        typedef std::list<MouseMotionListener*> MouseMotionListenerList;
        EventDispatcher mEventDispatcher;
		Overlay* mCursorLevelOverlay;
        bool mCursorGuiInitialised;
		GuiContainer* mCursorGuiRegistered;
		MouseMotionListener* mCursorListener;
        MouseMotionListenerList mMouseMotionListenerList;
        StringVector mScriptPatterns;

        void parseNewElement( DataStreamPtr& chunk, String& elemType, String& elemName, 
            bool isContainer, Overlay* pOverlay, bool isTemplate, String templateName = String(""), GuiContainer* container = 0);
        void parseAttrib( const String& line, Overlay* pOverlay);
        void parseElementAttrib( const String& line, Overlay* pOverlay, OverlayElement* pElement );
        void parseNewMesh(DataStreamPtr& chunk, String& meshName, String& entityName, Overlay* pOverlay);
        void skipToNextCloseBrace(DataStreamPtr& chunk);
        void skipToNextOpenBrace(DataStreamPtr& chunk);
        
        int mLastViewportWidth, mLastViewportHeight;
        bool mViewportDimensionsChanged;

	    bool parseChildren( DataStreamPtr& chunk, const String& line,
            Overlay* pOverlay, bool isTemplate, GuiContainer* parent = NULL);


    public:
        OverlayManager();
        virtual ~OverlayManager();

        /// @copydoc ScriptLoader::getScriptPatterns
        const StringVector& getScriptPatterns(void) const;
        /// @copydoc ScriptLoader::parseScript
        void parseScript(DataStreamPtr& stream, const String& groupName);
        /// @copydoc ScriptLoader::getLoadingOrder
        Real getLoadingOrder(void) const;

        /** Create a new Overlay. */
        Overlay* create(const String& name);
        /** Retrieve an Overlay by name 
        @returns A pointer to the Overlay, or 0 if not found
        */
        Overlay* getByName(const String& name);
        /** Destroys an existing overlay by name */
        void destroy(const String& name);
        /** Destroys an existing overlay */
        void destroy(Overlay* overlay);
        /** Destroys all existing overlays */
        void destroyAll(void);
        typedef MapIterator<OverlayMap> OverlayMapIterator;
        OverlayMapIterator getOverlayIterator(void);

        /** Internal method for queueing the visible overlays for rendering. */
        void _queueOverlaysForRendering(Camera* cam, RenderQueue* pQueue, Viewport *vp);

        /** Method for determining if the viewport has changed dimensions. 
        @remarks This is used by pixel-based OverlayElements to work out if they need to
            reclaculate their sizes.
        */
        bool hasViewportChanged(void) const;

        /** Gets the height of the destination viewport in pixels. */
        int getViewportHeight(void) const;
        
        /** Gets the width of the destination viewport in pixels. */
        int getViewportWidth(void) const;
        Real getViewportAspectRatio(void) const;

        /** This returns a PositionTarget at position x,y. */
        PositionTarget* getPositionTargetAt(Real x, Real y);

        void processEvent(InputEvent* e);

        /** register the default cursor GUI implementation with the manager */
        void setDefaultCursorGui(GuiContainer* cursor, MouseMotionListener*);
        /** register the cursor GUI implementation with the manager */
        void setCursorGui(GuiContainer* cursor);
        void addMouseMotionListener(MouseMotionListener* l);
        void removeMouseMotionListener(MouseMotionListener* l);
        Real getMouseX() { return mEventDispatcher.getMouseX(); }
        Real getMouseY() { return mEventDispatcher.getMouseY(); }
        void setDragDrop(bool dragDropOn) { mEventDispatcher.setDragDrop(dragDropOn); }
        /** returns the registered cursor GUI */
		GuiContainer* getCursorGui();

		/** create the high cursor level overlay and add the registered Cursor GUI implementation to it */
		void createCursorOverlay();
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static OverlayManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static OverlayManager* getSingletonPtr(void);
    };



}


#endif 
