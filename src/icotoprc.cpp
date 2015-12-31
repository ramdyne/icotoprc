/*
 * icotoprc.cpp
 *
 * Tool for converting a Windows Icon file to a format 
 * useable by the PWLib application "pwrc"
 *
 * Copyright (c) 2001 Philips Electronics N.V.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is icotoprc
 *
 * The Initial Developer of the Original Code is Phlilips Electronics N.V.
 *
 */

#include <ptlib.h>

#include "icotoprc.h"

PCREATE_PROCESS(ICOToPRC)

PString getArgumentsParseString()
{
	return PString( 
    "f-filename:"
    );
}

const PString CR_LF = "\r\n";

PString InttoStr( int in ) 
{
  if (in == 0 ) return PString( "0" );
  
  PString result;
  char lv_s[255];
  sprintf(lv_s, "%d", in);
  result = lv_s;
  
  return result;
}

void ICOToPRC::Main()
{

  PArgList & commandLine = GetArguments();
	commandLine.Parse( getArgumentsParseString() );

  PString lv_filename;
  
  if ( commandLine.HasOption( "filename" ) )
  {
    lv_filename = commandLine.GetOptionString( "filename" );
  }
  
  // What, the ****. Is this a bug?
  if ( lv_filename != "" )
  {
    PFile lv_file;

    lv_file.Open( lv_filename, PFile::ReadOnly );

    if ( lv_file.Open() )
    {
      PString lv_outputFilename = lv_filename;
      lv_outputFilename.Replace( ".ico", ".partprc" );
      lv_outputFilename.Replace( ".ICO", ".partprc" );

      cout << "Creating file " << lv_outputFilename << endl;
      PFile lv_outputFile( lv_outputFilename );
      if ( lv_outputFile.Open() ) 
      {
        lv_outputFile.WriteString( "This is NOT a valid PRC file!!!" + CR_LF );
        lv_outputFile.WriteString( "It only contains ONE icon...." + CR_LF + CR_LF + CR_LF );

        lv_outputFile.WriteString( "Icon @IDI_MAIN_WINDOW" + CR_LF + "{" + CR_LF);
        // Read preamble. Assumiong it's a vaild ICON file, we skip the header and start decoding first icon entry
        // We have to save the number of icons, because they might be  important
        int lv_nrIcons = 0; //WORD
        {
          PBYTEArray pArray( 6 );
          lv_file.Read( pArray.GetPointer(), pArray.GetSize() );

          lv_nrIcons = (WORD) (* ( pArray.GetPointer() + 4 ) );
        }
        cout << "Number of icons in the file: " << lv_nrIcons << endl;

        // Read the dimensions
        int lv_width;// BYTE
        int lv_height;// BYTE
        int lv_numColours;// BYTE
        {
          PBYTEArray pArray( 8 );
          lv_file.Read( pArray.GetPointer(), pArray.GetSize() );

          lv_width      = pArray[ 0 ];
          lv_height     = pArray[ 1 ];
          lv_numColours = pArray[ 2 ];
        }
        cout << "Dimensions: " << lv_width << " " << lv_height << " " << lv_numColours  << endl;
        lv_outputFile.WriteString( "    Dimensions " + InttoStr( lv_width ) + "," 
          + InttoStr( lv_height ) + "," + InttoStr( lv_numColours ) +";" + CR_LF );

        // Read the datasize and data offset for the current icon
        int lv_dataSize; // DWORD 
        int lv_dataOffset; // DWORD
        {
          PBYTEArray pArray( 4 );

          lv_file.Read( pArray.GetPointer(), pArray.GetSize() );
          lv_dataSize = ( DWORD ) (* pArray.GetPointer() );
          
          lv_file.Read( pArray.GetPointer(), pArray.GetSize() );
          lv_dataOffset = ( DWORD ) (* pArray.GetPointer() );
        }
        cout << "Data size = " << lv_dataSize << ", offset = " << lv_dataOffset << endl;
        
        // Skip the rest of the icon descriptions to the real icon data
        if ( lv_nrIcons > 1 )
        {
          PBYTEArray pArray( ( lv_nrIcons - 1 ) * 16);
          lv_file.Read( pArray.GetPointer(), pArray.GetSize() );

          cout << "Jumping from byte 22 to byte " << 22 + ( lv_nrIcons - 1 ) * 16 << ", file says this should be byte " << lv_dataOffset << endl;
        }

        // Read the icon itself
        // First the bitmap header
        int lv_bmpSize = 0;
        int lv_bmpWidth = 0;
        int lv_bmpHeight = 0;
        int lv_bmpPlanes = 0;
        int lv_bmpBitsPerPixel = 0;
        int lv_bmpSizeOfBitmap = 0;
        {
          PBYTEArray pArray( 40 );

          lv_file.Read( pArray.GetPointer(), pArray.GetSize() );
  
          lv_bmpSize = ( DWORD ) (* pArray.GetPointer() );
          lv_bmpWidth = ( DWORD ) (* ( pArray.GetPointer() + 4 ) );
          lv_bmpHeight = ( DWORD ) (* ( pArray.GetPointer() + 8 ) );
          lv_bmpPlanes = ( WORD ) (* ( pArray.GetPointer() + 12 ) );
          lv_bmpBitsPerPixel = ( WORD ) (* ( pArray.GetPointer() + 14 ) );
          // Skip DWORD  biCompression;
          lv_bmpSizeOfBitmap = ( DWORD ) (* ( pArray.GetPointer() + 20 ) );
        }

        cout << "Bitmap size = " << lv_bmpSize << " width = " << lv_bmpWidth << " height = " << lv_bmpHeight 
             << "\n       planes = " << lv_bmpPlanes << " bitsperpixel = " << lv_bmpBitsPerPixel << " size of bitmap = " << lv_bmpSizeOfBitmap << endl;
        
        // Second the palette
        lv_outputFile.WriteString( "    Colours {" + CR_LF);

        int lv_paletteSize;
        switch ( lv_bmpBitsPerPixel )
        {
        case 1:
          lv_paletteSize = 2;
          break;
        case 4:
          lv_paletteSize = 16;
          break;
        default:
          cout << "Illegal BitsPerPixel value, aborting" << endl;
          exit( 1 );
          break;
        }

        for ( int i=0; i < lv_paletteSize; i++ )
        {
          if ( i != 0 )
          {
            lv_outputFile.WriteString( "," + CR_LF);
          }

          PBYTEArray pArray( 4 );
          lv_file.Read( pArray.GetPointer(), pArray.GetSize() );
          
          //int lv_blue = pArray[ 0 ];
          //int lv_green = pArray[ 1 ];
          //int lv_red = pArray[ 2 ];
          
          int lv_red = pArray[ 0 ];
          int lv_green = pArray[ 1 ];
          int lv_blue = pArray[ 2 ];
          
          lv_outputFile.WriteString( "        " + InttoStr( lv_blue ) + ", " + InttoStr( lv_green ) + ", " + InttoStr( lv_red ) );
          // the reserved byte is skipped
        }
        lv_outputFile.WriteString( CR_LF + "    }" + CR_LF);

        // The BMP height is the sum of XOR and AND map height
        // Since they are equal....
        int lv_readHeight = lv_bmpHeight / 2;
        
        // The XOR map byte length is width * height / ( 8 / bits per pixel )
        int lv_xorMapLength = ( lv_bmpWidth * lv_readHeight ) / ( 8 / lv_bmpBitsPerPixel );
        cout << "Length of the XOR map = " << lv_xorMapLength << endl;
        
        // The XOR mas is what PWRC calls it Pixels element
        lv_outputFile.WriteString( "    Pixels {" + CR_LF);
        for ( i = 0; i < lv_readHeight; i ++ )
        {
          int lv_readWidth = lv_bmpWidth / ( 8 / lv_bmpBitsPerPixel );
          PBYTEArray pArray( lv_readWidth );

          lv_file.Read( pArray.GetPointer(), pArray.GetSize() );

          if ( lv_bmpBitsPerPixel == 1 )
          {
            // Read a byte and bla bla
          }
          else
          {
            // Assuming a 4 bits / pixel bitmap
            lv_outputFile.WriteString("        ");
            for ( int j = 0; j < lv_readWidth; j++ )
            {
              if ( j != 0 )
              {
                lv_outputFile.WriteString( " " );
              }
              int lv_highNibble = ( pArray[ j ] & 240 ) >> 4;
              int lv_lowNibble =  pArray[ j ] & 15;
              
              lv_outputFile.WriteString( InttoStr( lv_highNibble ) + " " + InttoStr( lv_lowNibble ) );
            }
          }
          lv_outputFile.WriteString( CR_LF );
        }
        lv_outputFile.WriteString( "    }" + CR_LF);
        
        // The AND map is just 1 bit per pixel
        int lv_andMapLength = ( lv_bmpWidth * lv_readHeight ) / 8;
        cout << "Length of the AND map = " << lv_andMapLength << endl;

        lv_outputFile.WriteString( "    AndMask {" + CR_LF);

        PString XorMask;
        XorMask += "    XorMask {" + CR_LF;

        for ( i = 0; i < lv_readHeight; i ++ )
        {
          int lv_readWidth = lv_bmpWidth / 8 ;
          PBYTEArray pArray( lv_readWidth );
          
          lv_file.Read( pArray.GetPointer(), pArray.GetSize() );
          
          lv_outputFile.WriteString("        ");
          XorMask += "        ";
          for ( int j = 0; j < lv_readWidth; j++ )
          {
            
            if ( ( pArray[ j ] & 128 ) >> 7 == 1 )
            {
              lv_outputFile.WriteString( "1 " );
              XorMask += "0 ";
            }
            else
            {
              lv_outputFile.WriteString( "0 " );
              XorMask += "1 ";
            }

            if ( ( pArray[ j ] & 64 ) >> 6 == 1 )
            {
              lv_outputFile.WriteString( "1 " );
              XorMask += "0 ";
            }
            else
            {
              lv_outputFile.WriteString( "0 " );
              XorMask += "1 ";
            }
            if ( ( pArray[ j ] & 32 ) >> 5 == 1 )
            {
              lv_outputFile.WriteString( "1 " );
              XorMask += "0 ";
            }
            else
            {
              lv_outputFile.WriteString( "0 " );
              XorMask += "1 ";
            }
            if ( ( pArray[ j ] & 16 ) >> 4 == 1 )
            {
              lv_outputFile.WriteString( "1 " );
              XorMask += "0 ";
            }
            else
            {
              lv_outputFile.WriteString( "0 " );
              XorMask += "1 ";
            }
            if ( ( pArray[ j ] & 8 ) >> 3 == 1 )
            {
              lv_outputFile.WriteString( "1 " );
              XorMask += "0 ";
            }
            else
            {
              lv_outputFile.WriteString( "0 " );
              XorMask += "1 ";
            }
            if ( ( pArray[ j ] & 4 ) >> 2 == 1 )
            {
              lv_outputFile.WriteString( "1 " );
              XorMask += "0 ";
            }
            else
            {
              lv_outputFile.WriteString( "0 " );
              XorMask += "1 ";
            }
            if ( ( pArray[ j ] & 2 >> 1 ) == 1 )
            {
              lv_outputFile.WriteString( "1 " );
              XorMask += "0 ";
            }
            else
            {
              lv_outputFile.WriteString( "0 " );
              XorMask += "1 ";
            }
            if ( pArray[ j ] & 1 == 1 )
            {
              lv_outputFile.WriteString( "1 " );
              XorMask += "0 ";
            }
            else
            {
              lv_outputFile.WriteString( "0 " );
              XorMask += "1 ";
            }

          }
          lv_outputFile.WriteString( CR_LF );
          XorMask += CR_LF;
        }

        XorMask += "    }" + CR_LF;
        lv_outputFile.WriteString( "    }" + CR_LF);

        lv_outputFile.WriteString( XorMask );

        lv_outputFile.WriteString( "}" + CR_LF);

        lv_outputFile.WriteString( CR_LF + CR_LF + "End of Icon section" + CR_LF );

        lv_outputFile.Close();
      }
      else
      {
        cout << "Could not create file " << lv_outputFilename << endl;  
      }
    } 
    else
    {
      cout << "Could not open file " << lv_filename << endl;
    }
  }
  else
  {
    cout << "No filename supplied" << endl;
  }
  
}

// End of H323Server.cpp
