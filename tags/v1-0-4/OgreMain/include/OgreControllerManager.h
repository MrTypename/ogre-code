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
#ifndef __ControllerManager_H__
#define __ControllerManager_H__

#include "OgrePrerequisites.h"

#include "OgreCommon.h"
#include "OgreSingleton.h"
#include "OgreController.h"
#include "OgrePredefinedControllers.h"
#include "OgreTextureUnitState.h"
#include "OgreSharedPtr.h"

namespace Ogre {

    typedef SharedPtr< ControllerValue<Real> > ControllerValueRealPtr;
    typedef SharedPtr< ControllerFunction<Real> > ControllerFunctionRealPtr;

    /** Class for managing Controller instances.
        @remarks
            This class is responsible to keeping tabs on all the Controller instances registered
            and updating them when requested. It also provides a number of convenience methods
            for creating commonly used controllers (such as texture animators).
    */
    class _OgreExport ControllerManager : public Singleton<ControllerManager>
    {
    protected:
        typedef std::set<Controller<Real>*> ControllerList;
        ControllerList mControllers;

        /// Global predefined controller
        SharedPtr< ControllerValue<Real> > mFrameTimeController;

    public:
        ControllerManager();
        ~ControllerManager();

        /** Creates a new controller and registers it with the manager.
        */
        Controller<Real>* createController(ControllerValueRealPtr src, 
			ControllerValueRealPtr dest, ControllerFunctionRealPtr func);

        /** Destroys all the controllers in existence.
        */
        void clearControllers(void);

        /** Updates all the registered controllers.
        */
        void updateAllControllers(void);


        /** Returns a ControllerValue which provides the time since the last frame as a control value source.
            @remarks
                A common source value to use to feed into a controller is the time since the last frame. This method
                returns a pointer to a common source value which provides this information.
            @par
                Remember the value will only be up to date after the RenderSystem::beginFrame method is called.
            @see
                RenderSystem::beginFrame
        */
        ControllerValueRealPtr getFrameTimeSource(void) const;

        /** Creates a texture layer animator controller.
            @remarks
                This helper method creates the Controller, ControllerValue and ControllerFunction classes required
                to animate a texture.
            @param
                layer TextureUnitState object to animate
            @param
                sequenceTime The amount of time in seconds it will take to loop through all the frames.
        */
        Controller<Real>* createTextureAnimator(TextureUnitState* layer, Real sequenceTime);

        /** Creates a basic time-based texture coordinate modifier designed for creating scrolling textures.
            @remarks
                This simple method allows you to easily create constant-speed scrolling textures. If you want more
                control, look up the ControllerManager::createTextureWaveTransformer for more complex wave-based
                scrollers / stretchers / rotaters.
            @param
                layer The texture layer to animate.
            @param
                uSpeed Speed of horizontal (u-coord) scroll, in complete wraps per second
            @param
                vSpeed Speed of vertical (v-coord) scroll, in complete wraps per second
        */
        Controller<Real>* createTextureScroller(TextureUnitState* layer, Real uSpeed, Real vSpeed);

        /** Creates a basic time-based texture coordinate modifier designed for creating rotating textures.
            @return
                This simple method allows you to easily create constant-speed rotating textures. If you want more
                control, look up the ControllerManager::createTextureWaveTransformer for more complex wave-based
                scrollers / stretchers / rotaters.
            @param
                layer The texture layer to rotate.
            @param
                vSpeed Speed of rotation, in complete anticlockwise revolutions per second
        */
        Controller<Real>* createTextureRotater(TextureUnitState* layer, Real speed);

        /** Creates a very flexible time-based texture transformation which can alter the scale, position or
            rotation of a texture based on a wave function.
            @param
                layer The texture layer to affect
            @param
                ttype The type of transform, either translate (scroll), scale (stretch) or rotate (spin)
            @param
                waveType The shape of the wave, see WaveformType enum for details
            @param
                base The base value of the output
            @param
                frequency The speed of the wave in cycles per second
            @param
                phase The offset of the start of the wave, e.g. 0.5 to start half-way through the wave
            @param
                amplitude Scales the output so that instead of lying within 0..1 it lies within 0..1*amplitude for exaggerated effects
        */
        Controller<Real>* createTextureWaveTransformer(TextureUnitState* layer, TextureUnitState::TextureTransformType ttype,
            WaveformType waveType, Real base = 0, Real frequency = 1, Real phase = 0, Real amplitude = 1);

        /** Creates a controller for passing a frame time value through to a vertex / fragment program parameter.
        @remarks
            The destination parameter is expected to be a float, and the '.x' attribute will be populated
            with the appropriately scaled time value.
        @param params The parameters to update
        @param paramIndex The index of the parameter to update; if you want a named parameter, then
            retrieve the index beforehand using GpuProgramParameters::getParamIndex
        @param factor The factor by which to adjust the time elapsed by before passing it to the program
        */
        Controller<Real>* createGpuProgramTimerParam(GpuProgramParameters* params, size_t paramIndex,
            Real timeFactor = 1.0f);

        /** Removes & destroys the controller passed in as a pointer.
        */
        void destroyController(Controller<Real>* controller);

		/** Return relative speed of time as perceived by time based controllers.
        @remarks
            See setTimeFactor for full information on the meaning of this value.
		*/
		Real getTimeFactor(void) const;

		/** Set the relative speed to update frame time based controllers.
        @remarks
            Normally any controllers which use time as an input (FrameTimeController) are updated
            automatically in line with the real passage of time. This method allows you to change
            that, so that controllers are told that the time is passing slower or faster than it
            actually is. Use this to globally speed up / slow down the effect of time-based controllers.
        @param tf The virtual speed of time (1.0 is real time).
		*/
		void setTimeFactor(Real tf);
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
        static ControllerManager& getSingleton(void);
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
        static ControllerManager* getSingletonPtr(void);
    };


}
#endif
