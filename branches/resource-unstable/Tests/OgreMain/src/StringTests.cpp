/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2004 The OGRE Team
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
#include "StringTests.h"
#include "OgreString.h"

// Regsiter the suite
CPPUNIT_TEST_SUITE_REGISTRATION( StringTests );

void StringTests::setUp()
{
	testFileNoPath = "testfile.txt";
	testFileRelativePathUnix = "this/is/relative/testfile.txt";
	testFileRelativePathWindows = "this\\is\\relative\\testfile.txt";
	testFileAbsolutePathUnix = "/this/is/absolute/testfile.txt";
	testFileAbsolutePathWindows = "c:\\this\\is\\absolute\\testfile.txt";
}
void StringTests::tearDown()
{
}

void StringTests::testSplitFileNameNoPath()
{
	String basename, path;
	StringUtil::splitFilename(testFileNoPath, basename, path);

    CPPUNIT_ASSERT_EQUAL(basename, testFileNoPath);
    CPPUNIT_ASSERT(path.empty());
}
void StringTests::testSplitFileNameRelativePath()
{
	String basename, path;
	// Unix
	StringUtil::splitFilename(testFileRelativePathUnix, basename, path);
    CPPUNIT_ASSERT_EQUAL(basename, "testfile.txt");
    CPPUNIT_ASSERT_EQUAL(path, "this/is/relative/");
	// Windows
	StringUtil::splitFilename(testFileRelativePathWindows, basename, path);
    CPPUNIT_ASSERT_EQUAL(basename, "testfile.txt");
    CPPUNIT_ASSERT_EQUAL(path, "this\\is\\relative\\");

}
void StringTests::testSplitFileNameAbsolutePath()
{
	String basename, path;
	// Unix
	StringUtil::splitFilename(testFileAbsolutePathUnix, basename, path);
    CPPUNIT_ASSERT_EQUAL(basename, "testfile.txt");
    CPPUNIT_ASSERT_EQUAL(path, "/this/is/absolute/");
	// Windows
	StringUtil::splitFilename(testFileAbsolutePathWindows, basename, path);
    CPPUNIT_ASSERT(basename == "testfile.txt");
	CPPUNIT_ASSERT(path == "c:\\this\\is\\absolute\\");
}

void StringTests::testMatchCaseSensitive()
{
	// Test positive
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, testFileNoPath, true));
	// Test negative
	String upperCase = testFileNoPath;
	StringUtil::toUpper(upperCase);
	CPPUNIT_ASSERT_ASSERTION_FAIL(StringUtil::match(testFileNoPath, upperCase, true));
}
void StringTests::testMatchCaseInSensitive()
{
	// Test positive
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, testFileNoPath, false));
	// Test positive
	String upperCase = testFileNoPath;
	StringUtil::toUpper(upperCase);
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, upperCase, false));
}
void StringTests::testMatchGlobAll()
{
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "*", true));
}
void StringTests::testMatchGlobStart()
{
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "*stfile.txt", true));
	CPPUNIT_ASSERT_ASSERTION_FAIL(StringUtil::match(testFileNoPath, "*astfile.txt", true));
}
void StringTests::testMatchGlobEnd()
{
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "testfile.*", true));
	CPPUNIT_ASSERT_ASSERTION_FAIL(StringUtil::match(testFileNoPath, "testfile.d*", true));
}
void StringTests::testMatchGlobStartAndEnd()
{
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "*stfile.*", true));
	CPPUNIT_ASSERT_ASSERTION_FAIL(StringUtil::match(testFileNoPath, "*astfile.d*", true));
}
void StringTests::testMatchGlobMiddle()
{
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "test*.txt", true));
	CPPUNIT_ASSERT_ASSERTION_FAIL(StringUtil::match(testFileNoPath, "last*.txt*", true));
}
void StringTests::testMatchSuperGlobtastic()
{
	CPPUNIT_ASSERT(StringUtil::match(testFileNoPath, "*t*f*e.t*t*", true));
}
