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
#include "OgreConfigFile.h"

#include "OgreException.h"

#include <iostream>

namespace Ogre {

    //-----------------------------------------------------------------------
    ConfigFile::ConfigFile()
    {
    }
    //-----------------------------------------------------------------------
    void ConfigFile::clear(void)
    {
        for (SettingsBySection::iterator seci = mSettings.begin(); 
            seci != mSettings.end(); ++seci)
        {
            delete seci->second;
        }
        mSettings.clear();
    }
    //-----------------------------------------------------------------------
    void ConfigFile::load(const String& filename, const String& separators, bool trimWhitespace)
    {
        /* Clear current settings map */
        clear();

        String currentSection = StringUtil::BLANK;
        SettingsMultiMap* currentSettings = new SettingsMultiMap();
        mSettings[currentSection] = currentSettings;

        
        /* Open the configuration file */
        std::ifstream fp;
        fp.open(filename.c_str());
        if(!fp)
            Except(
                Exception::ERR_FILE_NOT_FOUND, "'" + filename + "' file not found!", "ConfigFile::load" );
        
        /* Process the file line for line */
        String line, optName, optVal;
        while (std::getline(fp, line))
        {
            StringUtil::trim(line);
            /* Ignore comments & blanks */
            if (line.length() > 0 && line.at(0) != '#' && line.at(0) != '@')
            {
                if (line.at(0) == '[' && line.at(line.length()-1) == ']')
                {
                    // Section
                    currentSection = line.substr(1, line.length() - 2);
                    currentSettings = new SettingsMultiMap();
                    mSettings[currentSection] = currentSettings;

                }
                else
                {
                    /* Find the first seperator character and split the string there */
                    int separator_pos = line.find_first_of(separators, 0);
                    if (separator_pos != std::string::npos)
                    {
                        optName = line.substr(0, separator_pos);
                        /* Find the first non-seperator character following the name */
                        int nonseparator_pos = line.find_first_not_of(separators, separator_pos);
                        /* ... and extract the value */
                        optVal = line.substr(nonseparator_pos);
                        if (trimWhitespace)
                        {
                            StringUtil::trim(optVal);
                            StringUtil::trim(optName);
                        }
                        currentSettings->insert(std::multimap<String, String>::value_type(optName, optVal));
                    }
                }
            }
        }
        fp.close();


    }
    //-----------------------------------------------------------------------
    String ConfigFile::getSetting(const String& key, const String& section) const
    {
        
        SettingsBySection::const_iterator seci = mSettings.find(section);
        if (seci == mSettings.end())
        {
            return StringUtil::BLANK;
        }
        else
        {
            SettingsMultiMap::const_iterator i = seci->second->find(key);
            if (i == seci->second->end())
            {
                return StringUtil::BLANK;
            }
            else
            {
                return i->second;
            }
        }
    }
    //-----------------------------------------------------------------------
    StringVector ConfigFile::getMultiSetting(const String& key, const String& section) const
    {
        StringVector ret;


        SettingsBySection::const_iterator seci = mSettings.find(section);
        if (seci != mSettings.end())
        {
            SettingsMultiMap::const_iterator i;

            i = seci->second->find(key);
            // Iterate over matches
            while (i != seci->second->end() && i->first == key)
            {
                ret.push_back(i->second);
                ++i;
            }
        }
        return ret;


    }
    //-----------------------------------------------------------------------
    ConfigFile::SettingsIterator ConfigFile::getSettingsIterator(const String& section)
    {
        SettingsBySection::const_iterator seci = mSettings.find(section);
        if (seci == mSettings.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, 
                "Cannot find section " + section, 
                "ConfigFile::getSettingsIterator");
        }
        else
        {
            return SettingsIterator(seci->second->begin(), seci->second->end());
        }
    }
    //-----------------------------------------------------------------------
    ConfigFile::SectionIterator ConfigFile::getSectionIterator(void)
    {
        return SectionIterator(mSettings.begin(), mSettings.end());
    }

}
