/***************************************************************************
octreenode.cpp  -  description
-------------------
begin                : Fri Sep 27 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Lesser General Public License as        *
*   published by the Free Software Foundation; either version 2 of the    * 
*   License, or (at your option) any later version.                       *
*                                                                         *
***************************************************************************/

#include <OgreRoot.h>

#include <OgreOctreeNode.h>
#include <OgreOctreeSceneManager.h>

namespace Ogre
{
    unsigned long green = 0xFFFFFFFF;

    unsigned short OctreeNode::mIndexes[ 24 ] = {0, 1, 1, 2, 2, 3, 3, 0,    //back
        0, 6, 6, 5, 5, 1,          //left
        3, 7, 7, 4, 4, 2,          //right
        6, 7, 5, 4 };          //front
    unsigned long OctreeNode::mColors[ 8 ] = {green, green, green, green, green, green, green, green };

    OctreeNode::OctreeNode( SceneManager* creator ) : SceneNode( creator )
    {
        mOctant = 0;
    }

    OctreeNode::OctreeNode( SceneManager* creator, const String& name ) : SceneNode( creator, name )
    {
        mOctant = 0;
    }

    OctreeNode::~OctreeNode()
    {
    }

    void OctreeNode::_update( Camera* cam, bool updateChildren )
    {
        _updateFromParent();

        // Tell attached objects about camera position (incase any extra processing they want to do)

        ObjectMap::iterator i = mObjectsByName.begin();
        while( i != mObjectsByName.end() )
        {
            i -> second ->_notifyCurrentCamera( cam );
            ++i;
        }

        if ( updateChildren )
        {
            ChildNodeMap::iterator i = mChildren.begin();
            while( i != mChildren.end() )
            {
                SceneNode* sceneChild = static_cast < SceneNode* > ( i->second );
                sceneChild->_update( cam, updateChildren );
                ++i;
            }
        }

        _updateOctreeBounds();

    }

    //same as SceneNode, only it doesn't care about children...
    void OctreeNode::_updateOctreeBounds( void )
    {
        mWorldAABB.setNull();
        mLocalAABB.setNull();

        // Update bounds from own attached objects
        ObjectMap::iterator i = mObjectsByName.begin();
        AxisAlignedBox bx;

        while( i != mObjectsByName.end() )
        {

            // Get local bounds of object
            bx = i->second ->getBoundingBox();

            mLocalAABB.merge( bx );

            // Transform by aggregated transform
            bx.transform( _getFullTransform() );

            mWorldAABB.merge( bx );
            ++i;
        }


        //update the OctreeSceneManager that things might have moved.
        // if it hasn't been added to the octree, add it, and if has moved
        // enough to leave it's current node, we'll update it.
        if ( ! mWorldAABB.isNull() )
        {
            static_cast < OctreeSceneManager * > ( mCreator ) -> _updateOctreeNode( this );
        }

    }

    /** Since we are loose, only check the center.
    */
    bool OctreeNode::_isIn( AxisAlignedBox &box )
    {

        Vector3 center = mWorldAABB.getMaximum().midPoint( mWorldAABB.getMinimum() );

        Vector3 bmin = box.getMinimum();
        Vector3 bmax = box.getMaximum();

        return ( bmax > center && bmin < center );

    }

    /** Addes the attached objects of this OctreeScene node into the queue. */
    void OctreeNode::_addToRenderQueue( RenderQueue *queue )
    {
        ObjectMap::iterator mit = mObjectsByName.begin();

        while ( mit != mObjectsByName.end() )
        {
            MovableObject * mo = mit->second;
            mo -> _updateRenderQueue( queue );
            ++mit;
        }

    }


    void OctreeNode::getRenderOperation( RenderOperation& rend )
    {

        rend.useIndexes = true;
        rend.numTextureCoordSets = 0; // no textures
        rend.vertexOptions = RenderOperation::VO_DIFFUSE_COLOURS;
        rend.operationType = RenderOperation::OT_LINE_LIST;
        rend.numVertices = 8;
        rend.numIndexes = 24;

        rend.pVertices = mCorners;
        rend.pIndexes = mIndexes;
        rend.pDiffuseColour = mColors;

        const Vector3 * corners = _getLocalAABB().getAllCorners();

        int index = 0;

        for ( int i = 0; i < 8; i++ )
        {
            rend.pVertices[ index ] = corners[ i ].x;
            index++;
            rend.pVertices[ index ] = corners[ i ].y;
            index++;
            rend.pVertices[ index ] = corners[ i ].z;
            index++;
        }


    }
}
