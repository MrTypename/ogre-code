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
#ifndef _BspLevel_H__
#define _BspLevel_H__

#include "OgreBspPrerequisites.h"
#include "OgreResource.h"
#include "OgreStaticFaceGroup.h"
#include "OgreSceneManager.h"


namespace Ogre {

    /** Holds all the data associated with a Binary Space Parition
        (BSP) based indoor level.
        The data used here is populated by loading level files via
        the BspLevelManager::load method, although application users
        are more likely to call SceneManager::setWorldGeometry which will
        automatically arrange the loading of the level. Note that this assumes
        that you have asked for an indoor-specialised SceneManager (specify
        ST_INDOOR when calling Root::getSceneManager).</p>
        Ogre currently only supports loading from Quake3 Arena level files,
        although any source that can be converted into this classes structure
        could also be used. The Quake3 level load process is in a different
        class called Quake3Level to keep the specifics separate.</p>
    */
    class BspLevel : public Resource
    {
        friend class BspSceneManager;
    public:
        /** Default constructor - used by BspResourceManager (do not call directly) */
        BspLevel(String name);
        ~BspLevel();

        /** Generic load - called by BspResourceManager. */
        virtual void load(void);
        /** Generic unload - called by BspResourceManager. */
        virtual void unload(void);

        /** Determines if one leaf node is visible from another. */
        bool isLeafVisible(const BspNode* from, const BspNode* to) const;

        /** Returns a pointer to the root node (BspNode) of the BSP tree. */
        const BspNode* getRootNode(void);

        /** Walks the entire BSP tree and returns the leaf
            which contains the given point.
        */
        BspNode* findLeaf(const Vector3& point) const;


    protected:
        /** Pointer to the root node of the BSP tree;
            This pointer actually has a dual purpose; to avoid allocating lots of small chunks of
            memory, the BspLevel actually allocates all nodes required through this pointer. So this
            pointer is the handle for the allocation of memory for all nodes. It also happens to point
            to the root node, since the first one in the memory chunk is the root node.
        */
        BspNode* mRootNode;
        int mNumNodes;
        int mNumLeaves;
        int mLeafStart; // the index at which leaf nodes begin

        /** Vertex format for fixed geometry.
            Note that in this case vertex components (position, normal, texture coords etc)
            are held interleaved in the same buffer. This is because selected vertices will be
            copied from this main buffer to a rendering pipeline buffer ('vertex cache' in Quake3 speak)
            and it's easier & quicker to just copy a contiguous block from one buffer for all components
            than to copy from several separate buffers.
        */
        struct BspVertex
        {
            Real position[3];
            Real texcoords[2];
            Real lightmap[2];
            Real normal[3];
            int colour;
        };
        /** Array of vertices for whole level. */
        BspVertex* mVertices;
        int mNumVertices;

        /** Array of indexes into the mFaceGroups array. This buffer is organised
            by leaf node so leaves can just use contiguous chunks of it and
            get repointed to the actual entries in mFaceGroups. */
        int* mLeafFaceGroups;
        int mNumLeafFaceGroups;

        /** Array of face groups, indexed into by contents of mLeafFaceGroups. */
        StaticFaceGroup* mFaceGroups;
        int mNumFaceGroups;

        /** Array of elements i.e. vertex indexes as used by face groups.*/
        int* mElements;
        int mNumElements;


        /** Vector of player start points */
        std::vector<ViewPoint> mPlayerStarts;

        /** Internal utility function for loading data from Quake3. */
        void loadQuake3Level(const Quake3Level& q3lvl);
        /** Internal lookup table to determine visibility between leaves.
            Leaf nodes are assigned to 'clusters' of nodes, which are used to group nodes together for
            visibility testing. This data holds a lookup table which is used to determine if one cluster of leaves
            is visible from another cluster. Whilst it would be possible to expand all this out so that
            each node had a list of pointers to other visible nodes, this would be very expensive in terms
            of storage (using the cluster method there is a table which is 1-bit squared per cluster, rounded
            up to the nearest byte obviously, which uses far less space than 4-bytes per linked node per source
            node). Of course the limitation here is that you have to each leaf in turn to determine if it is visible
            rather than just following a list, but since this is only done once per frame this is not such a big
            overhead.</p>
            Each row in the table is a 'from' cluster, with each bit in the row corresponding to a 'to' cluster,
            both ordered based on cluster index. A 0 in the bit indicates the 'to' cluster is not visible from the
            'from' cluster, whilst a 1 indicates it is.</p>
            As many will notice, this is lifted directly from the Quake implementation of PVS.
        */
        struct VisData
        {
            unsigned char *tableData;
            int numClusters;            // Number of clusters, therefore number of rows
            int rowLength;                // Length in bytes of each row (num clusters / 8 rounded up)
        };

        VisData mVisData;


        /** Internal method for parsing chosen entities. */
        void loadEntities(const Quake3Level& q3lvl);


    };

}

#endif