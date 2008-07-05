/*
===============================================================================
This source file is part of the Ogre-Maya Tools.
Distributed as part of Ogre (Object-oriented Graphics Rendering Engine).

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
#include "OgreMayaMaterial.h"

#include <maya/MDagPath.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MFnSet.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItDag.h>

#include <iostream>

void showHelp();

using namespace OgreMaya;
using namespace std;

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
        MatGenerator      matGen;

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
        /*
        cout << "=== DAG Nodes ==============================\n";
        MItDag dagIter( MItDag::kBreadthFirst, MFn::kInvalid, 0 );
        for ( ; !dagIter.isDone(); dagIter.next()) {
            MDagPath dagPath;
            dagIter.getPath( dagPath );

            cout << "Node: "
               << dagPath.fullPathName().asChar()
               << "\n";
        }
        cout << "============================================\n";
        */
        


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
	    if (OPTIONS.exportMesh) {
			cout << "Exporting mesh...\n";	
			bStatus = meshGen.exportAll();
			if (!bStatus) {
				cout << "\tMeshGenerator::exportAll() failed!\n";
				return -4;
			}
		}


		// --- Material		

        if(OPTIONS.exportMaterial) {
            cout << "Exporting material...\n";	
		    bStatus = matGen.exportAll();
		    if (!bStatus) {            
			    cout << "\tMatGenerator::exportAll() failed!\n";
			    return -5;
		    }
        }  

    }


    return 1;
}


void showHelp()
{
	cout << "Usage: maya2ogre -in FILE [-mesh [FILE]] [-vba] [-skel [FILE]]\n";
    cout << "                 [-anim NAME START END STEP]\n";
    cout << "                 [-mat [FILE]] [-mprefix PREFIX]\n";
    cout << "                 [-n] [-c] [-t]\n\n";
	cout << " -in      FILE   input mb File\n";
    cout << " -mesh    FILE   export mesh (FILE is optional)\n";
    cout << " -vba            export vertex bone assignments\n";    
    cout << " -skel    FILE   export skeleton (FILE is optional)\n";
    cout << " -anim    NAME   export Animation beginning at START and ending\n";
    cout << "          START  at END with fixed STEP\n";
    cout << "          END\n";
    cout << "          STEP\n";
    cout << " -mat     FILE   export material (FILE is optional)\n";
    cout << " -mprefix PREFIX material prefix\n";
    cout << " -n              export normals\n";
    cout << " -c              export diffuse colours\n";
    cout << " -t              export texture coords\n\n";    
    cout << "Examples:\n";
    cout << " maya2ogre -in foo.mb -mesh -skel -mat\n";
    cout << "     => exports skeleton, mesh and material using default file names,\n";
    cout << "        in this case foo.mesh.xml, foo.skeleton.xml and foo.material\n\n";
    cout << " maya2ogre -in foo.mb -mesh custom_name.mesh.xml -skel custom_name.skel.xml\n";
    cout << "     => exports skeleton and mesh using user defined file names\n\n";
    cout << " maya2ogre -in foo.mb -skel -anim Walk 1 30 2 -anim Die 50 60 2\n";
    cout << "     => exports skeleton with animation tracks Walk and Die\n";

}