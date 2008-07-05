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



#include "OgreBMPCodec.h"

#include "OgreImage.h"

#include "OgreException.h"



#include <IL/il.h>

#include <IL/ilu.h>



namespace Ogre {





    //---------------------------------------------------------------------

    void BMPCodec::code( const DataChunk& input, DataChunk* output, ... ) const

    {        

        OgreGuard( "BMPCodec::code" );



        Except(Exception::UNIMPLEMENTED_FEATURE, "code to memory not implemented",

            "BMPCodec::code");



        OgreUnguard();



    }

    //---------------------------------------------------------------------

    Codec::CodecData * BMPCodec::decode( const DataChunk& input, DataChunk* output, ... ) const

    {

		OgreGuard( "BMPCodec::decode" );



		// DevIL variables

		ILuint ImageName;

		ILint ImageFormat, BytesPerPixel;

		ImageData * ret_data = new ImageData;



		// Load the image 

		ilGenImages( 1, &ImageName );

		ilBindImage( ImageName );



		ilLoadL( 

			getILType(), 

			( void * )const_cast< uchar * >( input.getPtr() ), 

			static_cast< ILuint >( input.getSize() ) );



		// Check if everything was ok

		ILenum PossibleError = ilGetError() ;

		if( PossibleError != IL_NO_ERROR )

		{

			Except( Exception::UNIMPLEMENTED_FEATURE, 

					"IL Error", 

					iluErrorString(PossibleError) ) ;

		}



		iluSwapColours();



		// Now sets some variables

		ImageFormat = ilGetInteger( IL_IMAGE_FORMAT );

		BytesPerPixel = ilGetInteger( IL_IMAGE_BYTES_PER_PIXEL ); 



		ret_data->format = ilFormat2OgreFormat( ImageFormat, BytesPerPixel );

		ret_data->width = ilGetInteger( IL_IMAGE_WIDTH );

		ret_data->height = ilGetInteger( IL_IMAGE_HEIGHT );
		ret_data->depth = ilGetInteger( IL_IMAGE_DEPTH );
        ret_data->num_mipmaps = ilGetInteger ( IL_NUM_MIPMAPS );


		uint ImageSize = ilGetInteger( IL_IMAGE_WIDTH ) * ilGetInteger( IL_IMAGE_HEIGHT ) * ilGetInteger( IL_IMAGE_BYTES_PER_PIXEL );


        ret_data->size = ImageSize;
        ret_data->flags = 0;

		// Move the image data to the output buffer

		output->allocate( ImageSize );

		memcpy( output->getPtr(), ilGetData(), ImageSize );



		ilDeleteImages( 1, &ImageName );



		OgreUnguardRet( ret_data );

    }

    //---------------------------------------------------------------------

    unsigned int BMPCodec::getILType(void) const

    { 

        return IL_BMP;

    }



}


