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
#include "OgreStableHeaders.h"

#include "OgreParticleSystem.h"
#include "OgreParticleSystemManager.h"
#include "OgreRenderQueue.h"
#include "OgreBillboardSet.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleAffector.h"
#include "OgreParticle.h"
#include "OgreSceneNode.h"
#include "OgreCamera.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreParticleAffectorFactory.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreMaterialManager.h"

namespace Ogre {
    // Init statics
    ParticleSystem::CmdCull ParticleSystem::msCullCmd;
    ParticleSystem::CmdHeight ParticleSystem::msHeightCmd;
    ParticleSystem::CmdMaterial ParticleSystem::msMaterialCmd;
    ParticleSystem::CmdQuota ParticleSystem::msQuotaCmd;
    ParticleSystem::CmdWidth ParticleSystem::msWidthCmd;
    ParticleSystem::CmdRenderer ParticleSystem::msRendererCmd;

    //-----------------------------------------------------------------------
    ParticleSystem::ParticleSystem() :
		mSpeedFactor(1.0f),
        mBoundsAutoUpdate(true),
        mBoundsUpdateTime(10.0f),
        mResourceGroupName(ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME),
        mIsRendererConfigured(false),
        mPoolSize(0),
        mRenderer(0),
        mCullIndividual(false)
    {
        initParameters();
        mAABB.setExtents(-1, -1, -1, 1, 1, 1);
        mBoundingRadius = 1;
        // Init world AABB to something silly
        Vector3 min( Math::POS_INFINITY, Math::POS_INFINITY, Math::POS_INFINITY );
        Vector3 max( Math::NEG_INFINITY, Math::NEG_INFINITY, Math::NEG_INFINITY );
        mWorldAABB.setExtents(min, max);

        // Default to billboard renderer
        setRenderer("Billboard");

    }
    //-----------------------------------------------------------------------
    ParticleSystem::ParticleSystem(const String& name, const String& resourceGroup):
		mSpeedFactor(1.0f),
        mBoundsAutoUpdate(true),
        mBoundsUpdateTime(10.0f),
        mResourceGroupName(resourceGroup),
        mIsRendererConfigured(false),
        mPoolSize(0),
        mRenderer(0),
        mCullIndividual(false)
    {
        mName = name;
        setDefaultDimensions( 100, 100 );
        setMaterialName( "BaseWhite" );
        // Default to 10 particles, expect app to specify (will only be increased, not decreased)
        setParticleQuota( 10 );
        initParameters();
        mAABB.setExtents(-1, -1, -1, 1, 1, 1);
        mBoundingRadius = 1;
        // Init world AABB to something silly
        Vector3 min( Math::POS_INFINITY, Math::POS_INFINITY, Math::POS_INFINITY );
        Vector3 max( Math::NEG_INFINITY, Math::NEG_INFINITY, Math::NEG_INFINITY );
        mWorldAABB.setExtents(min, max);

        // Default to billboard renderer
        setRenderer("Billboard");
    }
    //-----------------------------------------------------------------------
    ParticleSystem::~ParticleSystem()
    {
		// Arrange for the deletion of emitters & affectors
        removeAllEmitters();
        removeAllAffectors();

		// Deallocate all particles
		destroyVisualParticles(0, mParticlePool.size());
        // Free pool items
        ParticlePool::iterator i;
        for (i = mParticlePool.begin(); i != mParticlePool.end(); ++i)
        {
            delete *i;
        }

        if (mRenderer)
        {
            ParticleSystemManager::getSingleton()._destroyRenderer(mRenderer);
            mRenderer = 0;
        }

    }
    //-----------------------------------------------------------------------
    ParticleEmitter* ParticleSystem::addEmitter(const String& emitterType)
    {
        ParticleEmitter* em = 
            ParticleSystemManager::getSingleton()._createEmitter(emitterType, this);
        mEmitters.push_back(em);
        return em;
    }
    //-----------------------------------------------------------------------
    ParticleEmitter* ParticleSystem::getEmitter(unsigned short index) const
    {
        assert(index < mEmitters.size() && "Emitter index out of bounds!");
        return mEmitters[index];
    }
    //-----------------------------------------------------------------------
    unsigned short ParticleSystem::getNumEmitters(void) const
    {
        return static_cast< unsigned short >( mEmitters.size() );
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::removeEmitter(unsigned short index)
    {
        assert(index < mEmitters.size() && "Emitter index out of bounds!");
        ParticleEmitterList::iterator ei = mEmitters.begin() + index;
        ParticleSystemManager::getSingleton()._destroyEmitter(*ei);
        mEmitters.erase(ei);
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::removeAllEmitters(void)
    {
        // DON'T delete directly, we don't know what heap these have been created on
        ParticleEmitterList::iterator ei;
        for (ei = mEmitters.begin(); ei != mEmitters.end(); ++ei)
        {
            ParticleSystemManager::getSingleton()._destroyEmitter(*ei);
        }
        mEmitters.clear();
    }
    //-----------------------------------------------------------------------
    ParticleAffector* ParticleSystem::addAffector(const String& affectorType)
    {
        ParticleAffector* af = 
            ParticleSystemManager::getSingleton()._createAffector(affectorType, this);
        mAffectors.push_back(af);
        return af;
    }
    //-----------------------------------------------------------------------
    ParticleAffector* ParticleSystem::getAffector(unsigned short index) const
    {
        assert(index < mAffectors.size() && "Affector index out of bounds!");
        return mAffectors[index];
    }
    //-----------------------------------------------------------------------
    unsigned short ParticleSystem::getNumAffectors(void) const
    {
        return static_cast< unsigned short >( mAffectors.size() );
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::removeAffector(unsigned short index)
    {
        assert(index < mAffectors.size() && "Affector index out of bounds!");
        ParticleAffectorList::iterator ai = mAffectors.begin() + index;
        ParticleSystemManager::getSingleton()._destroyAffector(*ai);
        mAffectors.erase(ai);
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::removeAllAffectors(void)
    {
        // DON'T delete directly, we don't know what heap these have been created on
        ParticleAffectorList::iterator ai;
        for (ai = mAffectors.begin(); ai != mAffectors.end(); ++ai)
        {
            ParticleSystemManager::getSingleton()._destroyAffector(*ai);
        }
        mAffectors.clear();
    }
    //-----------------------------------------------------------------------
    ParticleSystem& ParticleSystem::operator=(const ParticleSystem& rhs)
    {
        // Blank this system's emitters & affectors
        removeAllEmitters();
        removeAllAffectors();

        // Copy emitters
        unsigned int i;
        for(i = 0; i < rhs.getNumEmitters(); ++i)
        {
            ParticleEmitter* rhsEm = rhs.getEmitter(i);
            ParticleEmitter* newEm = addEmitter(rhsEm->getType());
            rhsEm->copyParametersTo(newEm);
        }
        // Copy affectors
        for(i = 0; i < rhs.getNumAffectors(); ++i)
        {
            ParticleAffector* rhsAf = rhs.getAffector(i);
            ParticleAffector* newAf = addAffector(rhsAf->getType());
            rhsAf->copyParametersTo(newAf);
        }
        setParticleQuota(rhs.getParticleQuota());
        setMaterialName(rhs.mMaterialName);
        mDefaultHeight = rhs.mDefaultHeight;
        mDefaultWidth = rhs.mDefaultWidth;
        mCullIndividual = rhs.mCullIndividual;

        setRenderer(rhs.getRendererName());
        // Copy settings
        if (mRenderer && rhs.getRenderer())
        {
            rhs.getRenderer()->copyParametersTo(mRenderer);
        }

        return *this;

    }
    //-----------------------------------------------------------------------
    size_t ParticleSystem::getNumParticles(void) const
    {
        return mActiveParticles.size();
    }
    //-----------------------------------------------------------------------
    size_t ParticleSystem::getParticleQuota(void) const
    {
        return mParticlePool.size();
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setParticleQuota(size_t size)
    {
        // Never shrink below size()
        size_t currSize = mParticlePool.size();

        if( currSize < size )
        {
            this->increasePool(size);

            for( size_t i = currSize; i < size; ++i )
            {
                // Add new items to the queue
                mFreeParticles.push_back( mParticlePool[i] );
            }

            mPoolSize = size;
            // Tell the renderer
            if (mRenderer)
            {
                mRenderer->_notifyParticleQuota(size);
            }
            
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_update(Real timeElapsed)
    {
		// Scale incoming speed
		timeElapsed *= mSpeedFactor;

        // Init renderer if not done already
        configureRenderer();

		// Only update if attached to a node
		if (mParentNode)
		{
			// Update existing particles
        	_expire(timeElapsed);
        	_triggerAffectors(timeElapsed);
        	_applyMotion(timeElapsed);
			// Emit new particles
        	_triggerEmitters(timeElapsed);

            if (!mBoundsAutoUpdate && mBoundsUpdateTime > 0.0f)
                mBoundsUpdateTime -= timeElapsed; // count down 
            _updateBounds();
		}


    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_expire(Real timeElapsed)
    {
        ActiveParticleList::iterator i, itEnd;
        Particle* pParticle;

        itEnd = mActiveParticles.end();

        for (i = mActiveParticles.begin(); i != itEnd; )
        {
            pParticle = static_cast<Particle*>(*i);
            if (pParticle->timeToLive < timeElapsed)
            {
                // Destroy this one
                mFreeParticles.push_back( *i );
                i = mActiveParticles.erase( i );
            }
            else
            {
                // Decrement TTL
                pParticle->timeToLive -= timeElapsed;
				++i;
            }

        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_triggerEmitters(Real timeElapsed)
    {
        // Add up requests for emission
        static std::vector<unsigned> requested;
        if( requested.size() != mEmitters.size() )
            requested.resize( mEmitters.size() );

        size_t totalRequested, emitterCount, i, emissionAllowed;
        ParticleEmitterList::iterator	itEmit, iEmitEnd;
        ParticleAffectorList::iterator	itAff, itAffEnd;
			    
        iEmitEnd = mEmitters.end();
        emitterCount = mEmitters.size();
        emissionAllowed = getParticleQuota() - mActiveParticles.size();
        totalRequested = 0;

        // Count up total requested emissions
        for (itEmit = mEmitters.begin(), i = 0; itEmit != iEmitEnd; ++itEmit, ++i)
        {
            requested[i] = (*itEmit)->_getEmissionCount(timeElapsed);
            totalRequested += requested[i];
        }


        // Check if the quota will be exceeded, if so reduce demand
        if (totalRequested > emissionAllowed)
        {
            // Apportion down requested values to allotted values
            Real ratio =  (Real)emissionAllowed / (Real)totalRequested;
            for (i = 0; i < emitterCount; ++i)
            {
                requested[i] *= (unsigned int)ratio;
            }
        }

        // Emit
		// For each emission, apply a subset of the motion for the frame
		// this ensures an even distribution of particles when many are
		// emitted in a single frame
        for (itEmit = mEmitters.begin(), i = 0; itEmit != iEmitEnd; ++itEmit, ++i)
        {
			Real timePoint = 0.0f;
			Real timeInc = timeElapsed / requested[i];
	        for (unsigned int j = 0; j < requested[i]; ++j)
            {
                // Create a new particle & init using emitter
                Particle* p = createParticle();
                (*itEmit)->_initParticle(p);

				// Translate position & direction into world space
                // Maybe make emitter do this?
                p->position  = (mParentNode->_getDerivedOrientation() * p->position) + mParentNode->_getDerivedPosition();
                p->direction = (mParentNode->_getDerivedOrientation() * p->direction);

				// apply partial frame motion to this particle
            	p->position += (p->direction * timePoint);

				// apply particle initialization by the affectors
				itAffEnd = mAffectors.end();
				for (itAff = mAffectors.begin(); itAff != itAffEnd; ++itAff)
					(*itAff)->_initParticle(p);

				// Increment time fragment
				timePoint += timeInc;
            }
        }


    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_applyMotion(Real timeElapsed)
    {
        ActiveParticleList::iterator i, itEnd;
        Particle* pParticle;

        itEnd = mActiveParticles.end();
        for (i = mActiveParticles.begin(); i != itEnd; ++i)
        {
            pParticle = static_cast<Particle*>(*i);
            pParticle->position += (pParticle->direction * timeElapsed);
        }

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_triggerAffectors(Real timeElapsed)
    {
        ParticleAffectorList::iterator i, itEnd;
        
        itEnd = mAffectors.end();
        for (i = mAffectors.begin(); i != itEnd; ++i)
        {
            (*i)->_affectParticles(this, timeElapsed);
        }

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::increasePool(size_t size)
    {
        size_t oldSize = mParticlePool.size();

        // Increase size
        mParticlePool.reserve(size);
        mParticlePool.resize(size);

        // Create new particles
        for( size_t i = oldSize; i < size; i++ )
		{
            mParticlePool[i] = new Particle();
		}

		if (mIsRendererConfigured)
		{
			createVisualParticles(oldSize, size);
		}


    }
    //-----------------------------------------------------------------------
    ParticleIterator ParticleSystem::_getIterator(void)
    {
        return ParticleIterator(mActiveParticles.begin(), mActiveParticles.end());
    }
    //-----------------------------------------------------------------------
	Particle* ParticleSystem::getParticle(size_t index) 
	{
		assert (index < mActiveParticles.size() && "Index out of bounds!");
		ActiveParticleList::iterator i = mActiveParticles.begin();
		std::advance(i, index);
		return *i;
	}
    //-----------------------------------------------------------------------
    Particle* ParticleSystem::createParticle(void)
    {
        // Fast creation (don't use superclass since emitter will init)
        Particle* p = mFreeParticles.front();
        mFreeParticles.pop_front();
        mActiveParticles.push_back(p);

        p->_notifyOwner(this);

        return p;

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_updateRenderQueue(RenderQueue* queue)
    {
        if (mRenderer)
        {
            mRenderer->_updateRenderQueue(queue, mActiveParticles, mCullIndividual);
        }
    }
    /*
    //-----------------------------------------------------------------------
    void ParticleSystem::genBillboardAxes(const Camera& cam, Vector3* pX, Vector3 *pY, const Billboard* pBill)    
    {
        // Orientation different from BillboardSet
        // Billboards are in world space (to decouple them from emitters in node space)
        Quaternion camQ;

        switch (mBillboardType)
        {
        case BBT_POINT:
            // Get camera world axes for X and Y (depth is irrelevant)
            // No inverse transform
            camQ = cam.getDerivedOrientation();
            *pX = camQ * Vector3::UNIT_X;
            *pY = camQ * Vector3::UNIT_Y;
           
            break;
        case BBT_ORIENTED_COMMON:
             // Y-axis is common direction
            // X-axis is cross with camera direction 
            *pY = mCommonDirection;
            *pX = cam.getDerivedDirection().crossProduct(*pY);
           
            break;
        case BBT_ORIENTED_SELF:
            // Y-axis is direction
            // X-axis is cross with camera direction 

            // Scale direction first
            *pY = (pBill->mDirection * 0.01);
            *pX = cam.getDerivedDirection().crossProduct(*pY);

            break;
        }

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::getWorldTransforms(Matrix4* xform) const
    {
        // Particles are already in world space
        *xform = Matrix4::IDENTITY;

    }
    //-----------------------------------------------------------------------
    const Quaternion& ParticleSystem::getWorldOrientation(void) const
    {
        return mParentNode->_getDerivedOrientation();
    }
    //-----------------------------------------------------------------------
    const Vector3& ParticleSystem::getWorldPosition(void) const
    {
        return mParentNode->_getDerivedPosition();
    }
    //-----------------------------------------------------------------------
    */
    void ParticleSystem::initParameters(void)
    {
        if (createParamDictionary("ParticleSystem"))
        {
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("quota", 
                "The maximum number of particle allowed at once in this system.",
                PT_UNSIGNED_INT),
                &msQuotaCmd);

            dict->addParameter(ParameterDef("material", 
                "The name of the material to be used to render all particles in this system.",
                PT_STRING),
                &msMaterialCmd);

            dict->addParameter(ParameterDef("particle_width", 
                "The width of particles in world units.",
                PT_REAL),
                &msWidthCmd);

            dict->addParameter(ParameterDef("particle_height", 
                "The height of particles in world units.",
                PT_REAL),
                &msHeightCmd);

            dict->addParameter(ParameterDef("cull_each", 
                "If true, each particle is culled in it's own right. If false, the entire system is culled as a whole.",
                PT_BOOL),
                &msCullCmd);


        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_updateBounds()
    {

        if (mParentNode && (mBoundsAutoUpdate || mBoundsUpdateTime > 0.0f))
        {

            // Iterate over the particles in world space and grow the box as required
            Vector3 min = mWorldAABB.getMinimum();
            Vector3 max = mWorldAABB.getMaximum();
            ActiveParticleList::iterator p;
            for (p = mActiveParticles.begin(); p != mActiveParticles.end(); ++p)
            {
                min.makeFloor((*p)->position);
                max.makeCeil((*p)->position);
            }
            mWorldAABB.setExtents(min, max);


            // We've already put particles in world space to decouple them from the
            // node transform, so reverse transform back since we're expected to 
            // provide a local AABB
            Vector3 temp;
            const Vector3 *corner = mWorldAABB.getAllCorners();
            Quaternion invQ = mParentNode->_getDerivedOrientation().Inverse();
            Vector3 t = mParentNode->_getDerivedPosition();
            min.x = min.y = min.z = Math::POS_INFINITY;
            max.x = max.y = max.z = Math::NEG_INFINITY;
            for (int i = 0; i < 8; ++i)
            {
                // Reverse transform corner
                temp = invQ * (corner[i] - t);
                min.makeFloor(temp);
                max.makeCeil(temp);
            }
            AxisAlignedBox newAABB;
            newAABB.setExtents(min, max);
            // Merge calculated box with current AABB to preserve any user-set AABB
            mAABB.merge(newAABB);
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::fastForward(Real time, Real interval)
    {
        // First make sure all transforms are up to date

        for (Real ftime = 0; ftime < time; ftime += interval)
        {
            _update(interval);
        }
    }
    //-----------------------------------------------------------------------
    const String& ParticleSystem::getMovableType(void) const
    {
        static String mType = "ParticleSystem";
        return mType;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_notifyParticleResized(void)
    {
        if (mRenderer)
        {
            mRenderer->_notifyParticleResized();
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_notifyParticleRotated(void)
    {
        if (mRenderer)
        {
            mRenderer->_notifyParticleRotated();
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setDefaultDimensions( Real width, Real height )
    {
        mDefaultWidth = width;
        mDefaultHeight = height;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setDefaultWidth(Real width)
    {
        mDefaultWidth = width;
    }
    //-----------------------------------------------------------------------
    Real ParticleSystem::getDefaultWidth(void) const
    {
        return mDefaultWidth;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setDefaultHeight(Real height)
    {
        mDefaultHeight = height;
    }
    //-----------------------------------------------------------------------
    Real ParticleSystem::getDefaultHeight(void) const
    {
        return mDefaultHeight;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_notifyCurrentCamera(Camera* cam)
    {
        if (mRenderer)
        {
            mRenderer->_notifyCurrentCamera(cam);
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::_notifyAttached(Node* parent, bool isTagPoint)
    {
        MovableObject::_notifyAttached(parent, isTagPoint);
        if (mRenderer)
        {
            mRenderer->_notifyAttached(parent, isTagPoint);
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setMaterialName(const String& name)
    {
        mMaterialName = name;
        if (mIsRendererConfigured)
        {
            MaterialPtr mat = MaterialManager::getSingleton().load(
                mMaterialName, mResourceGroupName);
            mRenderer->_setMaterial(mat);
        }
    }
    //-----------------------------------------------------------------------
    const String& ParticleSystem::getMaterialName(void) const
    {
        return mMaterialName;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::clear()
    {
        // Insert actives into free list
        mFreeParticles.insert(mFreeParticles.end(), mActiveParticles.begin(), mActiveParticles.end());

        // Remove all active instances
        mActiveParticles.clear(); 

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setRenderer(const String& rendererName)
    {
		if (mRenderer)
		{
			// Destroy existing
			destroyVisualParticles(0, mParticlePool.size());
			ParticleSystemManager::getSingleton()._destroyRenderer(mRenderer);
			mRenderer = 0;
		}

        if (!rendererName.empty())
        {
			mRenderer = ParticleSystemManager::getSingleton()._createRenderer(rendererName);
            mIsRendererConfigured = false;
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::configureRenderer(void)
    {
        if (mRenderer && !mIsRendererConfigured)
        {
            mRenderer->_notifyParticleQuota(mParticlePool.size());
            createVisualParticles(0, mParticlePool.size());
            MaterialPtr mat = MaterialManager::getSingleton().load(
                mMaterialName, mResourceGroupName);
            mRenderer->_setMaterial(mat);
            mIsRendererConfigured = true;
        }
    }
    //-----------------------------------------------------------------------
    ParticleSystemRenderer* ParticleSystem::getRenderer(void) const
    {
        return mRenderer;
    }
    //-----------------------------------------------------------------------
    const String& ParticleSystem::getRendererName(void) const
    {
        if (mRenderer)
        {
            return mRenderer->getType();
        }
        else
        {
            return StringUtil::BLANK;
        }
    }
    //-----------------------------------------------------------------------
    bool ParticleSystem::getCullIndividually(void) const
    {
        return mCullIndividual;
    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setCullIndividually(bool cullIndividual)
    {
        mCullIndividual = cullIndividual;
    }
    //-----------------------------------------------------------------------
	void ParticleSystem::createVisualParticles(size_t poolstart, size_t poolend)
	{
		ParticlePool::iterator i = mParticlePool.begin();
		ParticlePool::iterator iend = mParticlePool.begin();
		std::advance(i, poolstart);
		std::advance(iend, poolend);
		for (; i != iend; ++i)
		{
			(*i)->_notifyVisualData(
				mRenderer->_createVisualData());
		}
	}
    //-----------------------------------------------------------------------
	void ParticleSystem::destroyVisualParticles(size_t poolstart, size_t poolend)
	{
		ParticlePool::iterator i = mParticlePool.begin();
		ParticlePool::iterator iend = mParticlePool.begin();
		std::advance(i, poolstart);
		std::advance(iend, poolend);
		for (; i != iend; ++i)
		{
			mRenderer->_destroyVisualData((*i)->getVisualData());
			(*i)->_notifyVisualData(0);
		}
	}
    //-----------------------------------------------------------------------
    void ParticleSystem::setBounds(const AxisAlignedBox& aabb)
    {
        mAABB = aabb;
        Real sqDist = std::max(mAABB.getMinimum().squaredLength(), 
            mAABB.getMaximum().squaredLength());
        mBoundingRadius = Math::Sqrt(sqDist);

    }
    //-----------------------------------------------------------------------
    void ParticleSystem::setBoundsAutoUpdated(bool autoUpdate, Real stopIn)
    {
        mBoundsAutoUpdate = autoUpdate;
        mBoundsUpdateTime = stopIn;
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdCull::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ParticleSystem*>(target)->getCullIndividually() );
    }
    void ParticleSystem::CmdCull::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setCullIndividually(
            StringConverter::parseBool(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdHeight::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ParticleSystem*>(target)->getDefaultHeight() );
    }
    void ParticleSystem::CmdHeight::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setDefaultHeight(
            StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdWidth::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ParticleSystem*>(target)->getDefaultWidth() );
    }
    void ParticleSystem::CmdWidth::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setDefaultWidth(
            StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdMaterial::doGet(const void* target) const
    {
        return static_cast<const ParticleSystem*>(target)->getMaterialName();
    }
    void ParticleSystem::CmdMaterial::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdQuota::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const ParticleSystem*>(target)->getParticleQuota() );
    }
    void ParticleSystem::CmdQuota::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setParticleQuota(
            StringConverter::parseUnsignedInt(val));
    }
    //-----------------------------------------------------------------------
    String ParticleSystem::CmdRenderer::doGet(const void* target) const
    {
        return static_cast<const ParticleSystem*>(target)->getRendererName();
    }
    void ParticleSystem::CmdRenderer::doSet(void* target, const String& val)
    {
        static_cast<ParticleSystem*>(target)->setRenderer(val);
    }
    //-----------------------------------------------------------------------
    ParticleAffector::~ParticleAffector() 
    {
    }
    //-----------------------------------------------------------------------
    ParticleAffectorFactory::~ParticleAffectorFactory() 
    {
        // Destroy all affectors
        std::vector<ParticleAffector*>::iterator i;
        for (i = mAffectors.begin(); i != mAffectors.end(); ++i)
        {
            delete (*i);
        }
            
        mAffectors.clear();

    }
    //-----------------------------------------------------------------------
    void ParticleAffectorFactory::destroyAffector(ParticleAffector* e)
    {
        std::vector<ParticleAffector*>::iterator i;
        for (i = mAffectors.begin(); i != mAffectors.end(); ++i)
        {
            if ((*i) == e)
            {
                mAffectors.erase(i);
                delete e;
                break;
            }
        }
    }

}
