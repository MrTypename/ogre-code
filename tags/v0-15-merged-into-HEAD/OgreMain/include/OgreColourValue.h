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
#ifndef _COLOURVALUE_H__
#define _COLOURVALUE_H__

#include "OgrePrerequisites.h"

namespace Ogre {

    typedef unsigned long RGBA;
    typedef unsigned long ARGB;
    typedef unsigned long ABGR;

    /** Class representing colour.
	    @remarks
		    Colour is represented as 4 components, each of which is a
		    floating-point value from 0.0 to 1.0.
	    @par
		    The 3 'normal' colour components are red, green and blue, a higher
		    number indicating greater amounts of that component in the colour.
		    The forth component is the 'alpha' value, which represents
		    transparency. In this case, 0.0 is completely transparent and 1.0 is
		    fully opaque.
    */
    class _OgreExport ColourValue
    {
    public:
	    static ColourValue Black;
	    static ColourValue White;
	    static ColourValue Red;
	    static ColourValue Green;
	    static ColourValue Blue;

	    ColourValue( Real red = 1.0f,
				    Real green = 1.0f,
				    Real blue = 1.0f,
				    Real alpha = 1.0f ) : r(red), g(green), b(blue), a(alpha)
        { }

	    bool operator==(const ColourValue& rhs) const;
	    bool operator!=(const ColourValue& rhs) const;

        union {
            struct {
	            Real r,g,b,a;
            };
            Real val[4];
        };

	    /** Retrieves colour as RGBA.
	    */
	    RGBA getAsLongRGBA(void) const;

	    /** Retrieves colour as ARGB.
	    */
	    ARGB getAsLongARGB(void) const;

	    /** Retrieves colours as ABGR */
	    ABGR getAsLongABGR(void) const;

        // arithmetic operations
        inline ColourValue operator + ( const ColourValue& rkVector ) const
        {
            ColourValue kSum;

            kSum.r = r + rkVector.r;
            kSum.g = g + rkVector.g;
            kSum.b = b + rkVector.b;
            kSum.a = a + rkVector.a;

            return kSum;
        }

        inline ColourValue operator - ( const ColourValue& rkVector ) const
        {
            ColourValue kDiff;

            kDiff.r = r - rkVector.r;
            kDiff.g = g - rkVector.g;
            kDiff.b = b - rkVector.b;
            kDiff.a = a - rkVector.a;

            return kDiff;
        }

        inline ColourValue operator * ( Real fScalar ) const
        {
            ColourValue kProd;

            kProd.r = fScalar*r;
            kProd.g = fScalar*g;
            kProd.b = fScalar*b;
            kProd.a = fScalar*a;

            return kProd;
        }

        inline ColourValue operator * ( const ColourValue& rhs) const
        {
            ColourValue kProd;

            kProd.r = rhs.r * r;
            kProd.g = rhs.g * g;
            kProd.b = rhs.b * b;
            kProd.a = rhs.a * a;

            return kProd;
        }

        inline ColourValue operator / ( const ColourValue& rhs) const
        {
            ColourValue kProd;

            kProd.r = rhs.r / r;
            kProd.g = rhs.g / g;
            kProd.b = rhs.b / b;
            kProd.a = rhs.a / a;

            return kProd;
        }

        inline ColourValue operator / ( Real fScalar ) const
        {
            assert( fScalar != 0.0 );

            ColourValue kDiv;

            Real fInv = 1.0 / fScalar;
            kDiv.r = r * fInv;
            kDiv.g = g * fInv;
            kDiv.b = b * fInv;
            kDiv.a = a * fInv;

            return kDiv;
        }

        inline friend ColourValue operator * ( Real fScalar, const ColourValue& rkVector )
        {
            ColourValue kProd;

            kProd.r = fScalar * rkVector.r;
            kProd.g = fScalar * rkVector.g;
            kProd.b = fScalar * rkVector.b;
            kProd.a = fScalar * rkVector.a;

            return kProd;
        }

        // arithmetic updates
        inline ColourValue& operator += ( const ColourValue& rkVector )
        {
            r += rkVector.r;
            g += rkVector.g;
            b += rkVector.b;
            a += rkVector.a;

            return *this;
        }

        inline ColourValue& operator -= ( const ColourValue& rkVector )
        {
            r -= rkVector.r;
            g -= rkVector.g;
            b -= rkVector.b;
            a -= rkVector.a;

            return *this;
        }

        inline ColourValue& operator *= ( Real fScalar )
        {
            r *= fScalar;
            g *= fScalar;
            b *= fScalar;
            a *= fScalar;
            return *this;
        }

        inline ColourValue& operator /= ( Real fScalar )
        {
            assert( fScalar != 0.0 );

            Real fInv = 1.0 / fScalar;

            r *= fInv;
            g *= fInv;
            b *= fInv;
            a *= fInv;

            return *this;
        }


    };

} // namespace

#endif