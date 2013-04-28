/******************************************************************************
* Loom Previewer
* Copyright (C) 2013 Paul Kerchen
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef LPIMAGER_H
#define LPIMAGER_H


#include <QString>

#include <vector>

/// Forward decls
class QBitArray;
class QImage;



namespace LP
{


class Imager //: public QObject
{
//   Q_OBJECT

public:
   /// Standard constructor
   Imager();
   /// Standard destructor
   virtual ~Imager();


   typedef enum
   {
      RGB, RBG, BGR, BRG, GRB, GBR, Grayscale 
   } ChannelOrder;

   bool load( const QString& filename, unsigned int blockSize );

   void regenerate( unsigned int redBitCount,
                    unsigned int greenBitCount,
                    unsigned int blueBitCount,
                    unsigned int grayBitCount,
                    ChannelOrder order,
                    unsigned int width,
                    unsigned int offset,
                    std::vector< QImage* >& imgVec
                  );


//signals:
   //void progress( int cur, int goal );

private:
   void unload();

   unsigned char mapBits( int bitCount, unsigned int& di, unsigned int& bai, unsigned int& limit );
   uint pixelRGB( unsigned int& di, unsigned int& bai, unsigned int& limit );
   uint pixelRBG( unsigned int& di, unsigned int& bai, unsigned int& limit );
   uint pixelGBR( unsigned int& di, unsigned int& bai, unsigned int& limit );
   uint pixelGRB( unsigned int& di, unsigned int& bai, unsigned int& limit );
   uint pixelBGR( unsigned int& di, unsigned int& bai, unsigned int& limit );
   uint pixelBRG( unsigned int& di, unsigned int& bai, unsigned int& limit );
   uint pixelGray( unsigned int& di, unsigned int& bai, unsigned int& limit );

   typedef uint (LP::Imager::*PixelFn)(unsigned int&, unsigned int&, unsigned int&);
   PixelFn   m_pixelFunc;

   std::vector< QBitArray* >  m_bitArrayVec;

   unsigned int  m_redBitCount, m_greenBitCount, m_blueBitCount, m_grayBitCount;
   unsigned int  m_bitsPerPixel;
   unsigned int  m_blockSize;
}; 

}  // namespace LP

#endif   // LPIMAGER_H


