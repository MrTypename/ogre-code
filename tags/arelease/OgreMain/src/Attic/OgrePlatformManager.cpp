/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright � 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#include "OgrePlatformManager.h"

#include "OgreDynLibManager.h"
#include "OgreDynLib.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    template<> PlatformManager* Singleton<PlatformManager>::ms_Singleton = 0;
    //-----------------------------------------------------------------------
    PlatformManager::PlatformManager()
    {
        // Load library
        String libraryName = "OgrePlatform.";
        #if OGRE_PLATFORM == PLATFORM_WIN32
            libraryName += "dll";
        #else
            libraryName += "so";
        #endif

        DynLib* lib = DynLibManager::getSingleton().load(libraryName);

        mpfCreateConfigDialog = (DLL_CREATECONFIGDIALOG)lib->getSymbol("createPlatformConfigDialog");
        mpfCreateErrorDialog = (DLL_CREATEERRORDIALOG)lib->getSymbol("createPlatformErrorDialog");
        mpfCreateInputReader = (DLL_CREATEINPUTREADER)lib->getSymbol("createPlatformInputReader");

        mpfDestroyConfigDialog = (DLL_DESTROYCONFIGDIALOG)lib->getSymbol("destroyPlatformConfigDialog");
        mpfDestroyErrorDialog = (DLL_DESTROYERRORDIALOG)lib->getSymbol("destroyPlatformErrorDialog");
        mpfDestroyInputReader = (DLL_DESTROYINPUTREADER)lib->getSymbol("destroyPlatformInputReader");

    }
    //-----------------------------------------------------------------------
    ConfigDialog* PlatformManager::createConfigDialog()
    {
        // Delegate
        ConfigDialog* pdlg;
        mpfCreateConfigDialog(&pdlg);
        return pdlg;
    }
    //-----------------------------------------------------------------------
    ErrorDialog* PlatformManager::createErrorDialog()
    {
        // Delegate
        ErrorDialog* pdlg;
        mpfCreateErrorDialog(&pdlg);
        return pdlg;
    }
    //-----------------------------------------------------------------------
    InputReader* PlatformManager::createInputReader()
    {
        // Delegate
        InputReader* pinput;
        mpfCreateInputReader(&pinput);
        return pinput;
    }
    //-----------------------------------------------------------------------
    void PlatformManager::destroyConfigDialog(ConfigDialog*  dlg)
    {
        // Delegate
        mpfDestroyConfigDialog(dlg);
    }
    //-----------------------------------------------------------------------
    void PlatformManager::destroyErrorDialog(ErrorDialog* dlg)
    {
        // Delegate
        mpfDestroyErrorDialog(dlg);
    }
    //-----------------------------------------------------------------------
    void PlatformManager::destroyInputReader(InputReader* reader)
    {
        // Delegate
        mpfDestroyInputReader(reader);
    }
    //-----------------------------------------------------------------------
    PlatformManager& PlatformManager::getSingleton(void)
    {
        return Singleton<PlatformManager>::getSingleton();
    }



}
