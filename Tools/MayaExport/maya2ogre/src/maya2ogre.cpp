/*
===============================================================================
This source file is part of the Ogre-Maya Tools.
Distributed as part of Ogre (Object-oriented Graphics Rendering Engine).
Copyright � 2003 Fifty1 Software Inc. 

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
===============================================================================
*/

#include "OgreMayaOptions.h"

#include "OgreMayaScene.h"
#include "OgreMayaMesh.h"
#include "OgreMayaSkeleton.h"

#include <maya/MDagPath.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MFnSet.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItDag.h>

void showHelp();

using namespace OgreMaya;

int main(int argc, char *argv[]) {

	// ===== Parse command line options
    OPTIONS.init(argc, argv);
    
    if(!OPTIONS.isValid()) {
        showHelp();
        return -1;
    }

    OPTIONS.debugOutput();

    {
  	    SceneMgr          sceneMgr;
	    MeshGenerator     meshGen;
	    SkeletonGenerator skelGen;

	    bool bStatus;

	    // ===== Initialize Maya and load scene
	    cout << "Loading Maya scene...\n";	
	    bStatus = sceneMgr.load();
	    if (!bStatus) {
		    cout << "\tSceneMgr::load() failed!\n";
		    return -2;
	    }
	    

        // DEBUG
        // ===== Iterate over mesh components of DAG       
        MItDag dagIter( MItDag::kBreadthFirst, MFn::kInvalid, 0 );
        for ( ; !dagIter.isDone(); dagIter.next()) {
            MDagPath dagPath;
            dagIter.getPath( dagPath );

            cout << "Node: "
               << dagPath.fullPathName().asChar()
               << "\n";
        }


	    // ===== Export
  	    // --- Skeleton
	    if (OPTIONS.exportSkeleton) {
		    cout << "Exporting skeleton...\n";	
		    bStatus = skelGen.exportAll();
		    if (!bStatus) {
			    printf("SkelGenerator::exportAll() failed!");
			    return -3;
		    }
	    }

	    
	    // --- Mesh	    
		cout << "Exporting mesh...\n";	
		bStatus = meshGen.exportAll();
		if (!bStatus) {
			cout << "\tMeshGenerator::exportAll() failed!\n";
			return -4;
		}

    }


    return 1;
}


void showHelp()
{
	cout << "Usage: maya2ogre -in FILE [-mesh FILE] [-skel FILE] [-s] [-n] [-c] [-t]\n\n";
	cout << " -in   FILE   input mb File\n";
    cout << " -mesh FILE   mesh file name\n";
    cout << " -skel FILE   skeleton file name\n";
    cout << " -s           export skeleton\n";
    cout << " -n           export normals\n";
    cout << " -c           export diffuse colours\n";
    cout << " -t           export texture coords\n";    
}