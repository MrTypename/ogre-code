/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2005 The OGRE Team
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
#ifndef _PixelFormat_H__
#define _PixelFormat_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"

namespace Ogre {
    /** The pixel format used for images, textures, and render surfaces */
    enum PixelFormat
    {
        /// Unknown pixel format.
        PF_UNKNOWN = 0,
        /// 8-bit pixel format, all bits luminace.
        PF_L8 = 1,
        /// 16-bit pixel format, all bits luminace.
        PF_L16 = 2,
        /// 8-bit pixel format, all bits alpha.
        PF_A8 = 3,
        /// 8-bit pixel format, 4 bits alpha, 4 bits luminace.
        PF_A4L4 = 4,
        /// 8-bit pixel format, 4 bits luminace, 4 bits alpha.
        PF_L4A4 = 5,
        /// 16-bit pixel format, 5 bits red, 6 bits green, 5 bits blue.
        PF_R5G6B5 = 6,
        /// 16-bit pixel format, 5 bits blue, 6 bits green, 5 bits red.
        PF_B5G6R5 = 7,
        /// 16-bit pixel format, 4 bits for alpha, red, green and blue.
        PF_A4R4G4B4 = 8,
        /// 16-bit pixel format, 4 bits for blue, green, red and alpha.
        PF_B4G4R4A4 = 9,
        /// 24-bit pixel format, 8 bits for red, green and blue.
        PF_R8G8B8 = 10,
        /// 24-bit pixel format, 8 bits for blue, green and red.
        PF_B8G8R8 = 11,
        /// 32-bit pixel format, 8 bits for alpha, red, green and blue.
        PF_A8R8G8B8 = 12,
        /// 32-bit pixel format, 8 bits for blue, green, red and alpha.
        PF_A8B8G8R8 = 13,
        /// 32-bit pixel format, 8 bits for blue, green, red and alpha.
        PF_B8G8R8A8 = 14,
        /// 32-bit pixel format, 2 bits for alpha, 10 bits for red, green and blue.
        PF_A2R10G10B10 = 15,
        /// 32-bit pixel format, 10 bits for blue, green and red, 2 bits for alpha.
        PF_A2B10G10R10 = 16,
        /// DDS (DirectDraw Surface) DXT1 format
        PF_DXT1 = 17,
        /// DDS (DirectDraw Surface) DXT2 format
        PF_DXT2 = 18,
        /// DDS (DirectDraw Surface) DXT3 format
        PF_DXT3 = 19,
        /// DDS (DirectDraw Surface) DXT4 format
        PF_DXT4 = 20,
        /// DDS (DirectDraw Surface) DXT5 format
        PF_DXT5 = 21,
        // 48-bit pixel format, 16 bits (float) for red, 16 bits (float) for green, 16 bits (float) for blue
        PF_FP_R16G16B16 = 22,
        // 64-bit pixel format, 16 bits (float) for red, 16 bits (float) for green, 16 bits (float) for blue, 16 bits (float) for alpha
        PF_FP_R16G16B16A16 = 23,
       // 96-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue
        PF_FP_R32G32B32 = 24,
        // 128-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue, 32 bits (float) for alpha
        PF_FP_R32G32B32A32 = 25,
        // Number of pixel formats currently defined
        PF_COUNT = 26
    };

    /**
     * Flags defining some on/off properties of pixel formats
     */
    enum PixelFormatFlags {
        // This format has an alpha channel
        PFF_HASALPHA        = 0x00000001,      
        // This format is compressed. This invalidates the values in elemBytes,
        // elemBits and the bit counts as these might not be fixed in a compressed format.
        PFF_COMPRESSED    = 0x00000002,
        // This is a floating point format
        PFF_FLOAT           = 0x00000004,         
        // This is a depth format (for depth textures)
        PFF_DEPTH           = 0x00000008,
        // Format is in native endian. Generally true for the 16, 24 and 32 bits
        // formats which can be represented as machine integers.
        PFF_NATIVEENDIAN    = 0x00000010,
        // This is an intensity format instead of a RGB one. The luminance
        // replaces R,G and B. (but not A)
        PFF_LUMINANCE       = 0x00000020
    };
	/** A primitive describing a volume (3D), image (2D) or line (1D) of pixels in memory.
     	In case of a rectangle, depth must be 1. 
     	Pixels are stored as a succession of "depth" slices, each containing "height" rows of 
     	"width" pixels.
    */
    class _OgreExport PixelBox: public Box {
    public:
    	/// Parameter constructor for setting the members manually
    	PixelBox() {}
		/** Constructor providing extents in the form of a Box object. This constructor
    		assumes the pixel data is laid out consecutively in memory. (this
    		means row after row, slice after slice, with no space in between)
    		@param extents	Extents of the region defined by data
    		@param format	Format of this buffer
    		@param data		Pointer to the actual data
    	*/
		PixelBox(const Box &extents, PixelFormat format, void *data=0):
			Box(extents), format(format), data(data)
		{
			setConsecutive();
		}
    	/** Constructor providing width, height and depth. This constructor
    		assumes the pixel data is laid out consecutively in memory. (this
    		means row after row, slice after slice, with no space in between)
    		@param width	Width of the region
    		@param height	Height of the region
    		@param depth	Depth of the region
    		@param format	Format of this buffer
    		@param data		Pointer to the actual data
    	*/
    	PixelBox(size_t width, size_t height, size_t depth, PixelFormat format, void *data=0):
    		Box(0, 0, 0, width, height, depth),
    		format(format), data(data)
    	{
    		setConsecutive();
    	}
    	
        /// The data pointer 
        void *data;
        /// The pixel format 
        PixelFormat format;
        /** Number of elements between the leftmost pixel of one row and the left
         	pixel of the next. This can be a negative value.
        */
        size_t rowPitch;
        /** Number of elements between the top left pixel of one (depth) slice and 
         	the top left pixel of the next. This can be a negative value. Must be a multiple of
         	rowPitch.
        */
        size_t slicePitch;
        
        /** Set the rowPitch and slicePitch so that the buffer is laid out consecutive 
         	in memory.
        */        
        void setConsecutive()
        {
            rowPitch = getWidth();
            slicePitch = getWidth()*getHeight();
        }
        /**	Get the number of elements between one past the rightmost pixel of 
         	one row and the leftmost pixel of the next row. (IE this is zero if rows
         	are consecutive).
        */
        size_t getRowSkip() const { return rowPitch - getWidth(); }
        /** Get the number of elements between one past the right bottom pixel of
         	one slice and the left top pixel of the next slice. (IE this is zero if slices
         	are consecutive).
        */
        size_t getSliceSkip() const { return slicePitch - (getHeight() * rowPitch); }

        /** Return whether this buffer is laid out consecutive in memory (ie the pitches
         	are equal to the dimensions)
        */        
        bool isConsecutive() const 
		{ 
			return rowPitch == getWidth() && slicePitch == getWidth()*getHeight(); 
		}
        /** Return the size (in bytes) this image would take if it was
        	laid out consecutive in memory
      	*/
      	size_t getConsecutiveSize() const;
      	/** Return a subvolume of this PixelBox.
      		@param def	Defines the bounds of the subregion to return
      		@returns	A pixel box describing the region and the data in it
      		@remarks	This function does not copy any data, it just returns
      			a PixelBox object with a data pointer pointing somewhere inside 
      			the data of object.
      		@throws	Exception(ERR_INVALIDPARAMS) if def is not fully contained
      	*/
      	PixelBox getSubVolume(const Box &def) const;
    };
    

    /**
     * Some utility functions for packing and unpacking pixel data
     */
    class _OgreExport PixelUtil {
    public:
        /** Returns the size in bytes of an element of the given pixel format.
         @returns
               The size in bytes of an element. See Remarks.
         @remarks
               Passing PF_UNKNOWN will result in returning a size of 0 bytes.
        */
        static size_t getNumElemBytes( PixelFormat format );

        /** Returns the size in bits of an element of the given pixel format.
          @returns
               The size in bits of an element. See Remarks.
           @remarks
               Passing PF_UNKNOWN will result in returning a size of 0 bits.
        */
        static size_t getNumElemBits( PixelFormat format );

		/** Returns the size in memory of a region with the given extents and pixel
			format with consecutive memory layout.
			@param width
				The width of the area
			@param height
				The height of the area
			@param depth
				The depth of the area
		  	@returns
		  		The size in bytes
			@remarks
				In case that the format is non-compressed, this simply returns
				width*height*depth*PixelUtil::getNumElemBytes(format). In the compressed
				case, this does serious magic.
		*/
		static size_t getMemorySize(size_t width, size_t height, size_t depth, PixelFormat format);
		
        /** Returns the property flags for this pixel format
          @returns
               A bitfield combination of PFF_HASALPHA, PFF_ISCOMPRESSED,
               PFF_FLOAT, PFF_DEPTH, PFF_NATIVEENDIAN, PFF_LUMINANCE
          @remarks
               This replaces the seperate functions for formatHasAlpha, formatIsFloat, ...
        */
        static unsigned int getFlags( PixelFormat format );

        /** Shortcut method to determine if the format has an alpha component */
        static bool hasAlpha(PixelFormat format);
        /** Shortcut method to determine if the format is floating point */
        static bool isFloatingPoint(PixelFormat format);
        /** Shortcut method to determine if the format is compressed */
        static bool isCompressed(PixelFormat format);
        /** Shortcut method to determine if the format is a depth format. */
        static bool isDepth(PixelFormat format);
        /** Shortcut method to determine if the format is in native endian format. */
        static bool isNativeEndian(PixelFormat format);
        /** Shortcut method to determine if the format is a luminance format. */
        static bool isLuminance(PixelFormat format);

        /** Gives the number of bits (RGBA) for a format. See remarks.          
          @remarks      For non-colour formats (dxt, depth) this returns [0,0,0,0].
        */
        static void getBitDepths(PixelFormat format, int rgba[4]);

        /** Gets the name of an image format
        */
        static String getFormatName(PixelFormat srcformat);

        /** Returns wether the format can be packed or unpacked with the packColour()
        and unpackColour() functions. This is generally not true for compressed and
        depth formats as they are special. It can only be true for formats with a
        fixed element size.
          @returns 
               true if yes, otherwise false
        */
        static bool isAccessible(PixelFormat srcformat);
        
        /** Pack a colour value to memory
        	@param colour	The colour
        	@param pf		Pixelformat in which to write the colour
        	@param dest		Destination memory location
        */
        static void packColour(const ColourValue &colour, PixelFormat pf,  void* dest);
        /** Pack a colour value to memory
        	@param r,g,b,a	The four colour components, range 0x00 to 0xFF
        	@param pf		Pixelformat in which to write the colour
        	@param dest		Destination memory location
        */
        static void packColour(uint8 r, uint8 g, uint8 b, uint8 a, PixelFormat pf,  void* dest);
         /** Pack a colour value to memory
        	@param r,g,b,a	The four colour components, range 0.0f to 1.0f
        					(an exception to this case exists for floating point pixel
        					formats, which don't clamp to 0.0f..1.0f)
        	@param pf		Pixelformat in which to write the colour
        	@param dest		Destination memory location
        */
        static void packColour(float r, float g, float b, float a, PixelFormat pf,  void* dest); 

        /** Unpack a colour value from memory
        	@param colour	The colour is returned here
        	@param pf		Pixelformat in which to read the colour
        	@param src		Source memory location
        */
        static void unpackColour(ColourValue *colour, PixelFormat pf,  const void* src);
        /** Unpack a colour value from memory
        	@param r,g,b,a	The colour is returned here (as byte)
        	@param pf		Pixelformat in which to read the colour
        	@param src		Source memory location
        	@remarks 	This function returns the colour components in 8 bit precision,
        		this will lose precision when coming from PF_A2R10G10B10 or floating
        		point formats.  
        */
        static void unpackColour(uint8 *r, uint8 *g, uint8 *b, uint8 *a, PixelFormat pf,  const void* src);
        /** Unpack a colour value from memory
        	@param r,g,b,a	The colour is returned here (as float)
        	@param pf		Pixelformat in which to read the colour
        	@param src		Source memory location
        */
        static void unpackColour(float *r, float *g, float *b, float *a, PixelFormat pf,  const void* src); 
        
        /** Convert consecutive pixels from one format to another. No dithering or filtering is being done. 
         	Converting from RGB to luminance takes the R channel.  In case the source and destination format match,
         	just a copy is done.
         	@param	src			Pointer to source region
         	@param	srcFormat	Pixel format of source region
         	@param   dst			Pointer to destination region
         	@param	dstFormat	Pixel format of destination region
         */
        static void bulkPixelConversion(void *src, PixelFormat srcFormat, void *dest, PixelFormat dstFormat, unsigned int count);

      	/** Convert pixels from one format to another. No dithering or filtering is being done. Converting
          	from RGB to luminance takes the R channel. 
		 	@param	src			PixelBox containing the source pixels, pitches and format
		 	@param	dst			PixelBox containing the destination pixels, pitches and format
		 	@remarks The source and destination boxes must have the same
         	dimensions. In case the source and destination format match, a plain copy is done.
        */
        static void bulkPixelConversion(const PixelBox &src, const PixelBox &dst);
    };



    /* 
     * DevIL specific utility class
     **/    
    class ILUtil {
    public:
    	/// Structure that encapsulates a devIL image format definition
		struct ILFormat {
			/// Construct an invalidated ILFormat structure
			ILFormat():
				numberOfChannels(0), format(-1), type(-1) {};

			/// Construct a ILFormat from parameters
			ILFormat(int channels, int format, int type=-1):
				numberOfChannels(channels), format(format), type(type) {}

			/// Return wether this structure represents a valid DevIL format
			bool isValid() { return format!=-1; }

			/// Number of channels, usually 3 or 4
			int numberOfChannels;
			/// IL_RGBA,IL_BGRA,IL_DXTx, ...
  			int format;
			/// IL_UNSIGNED_BYTE, IL_UNSIGNED_SHORT, ... may be -1 for compressed formats
  			int type;
		};

        /** Get OGRE format to which a given IL format can be most optimally converted.
         */
        static PixelFormat ilFormat2OgreFormat( int ImageFormat, int ImageType );
        /**	Get IL format that matches a given OGRE format exactly in memory.
        	@remarks	Returns an invalid ILFormat (.isValid()==false) when
        		there is no IL format that matches this.
         */
        static ILFormat OgreFormat2ilFormat( PixelFormat format );      
        /** Convert current IL image to an OGRE format. The size of the target will be
          	PixelUtil::getNumElemBytes(fmt) * ilGetInteger( IL_IMAGE_WIDTH ) * ilGetInteger( IL_IMAGE_HEIGHT ) * ilGetInteger( IL_IMAGE_DEPTH )
          	The IL image type must be IL(_UNSIGNED_)BYTE or IL_FLOAT.
        	The IL image format must be IL_RGBA, IL_BGRA, IL_RGB, IL_BGR, IL_LUMINANCE or IL_LUMINANCE_ALPHA
         
         	@param tar       Target pointer
         	@param ogrefmt   Ogre pixel format to employ
        */
        static void toOgre(uint8 *tar, PixelFormat ogrefmt);

        /** Convert an OGRE format image to current IL image.
         	@param src       Pixelbox; encapsulates source pointer, width, height, 
         					 depth and format
        */
        static void fromOgre(const PixelBox &src);
    };

}

#endif
