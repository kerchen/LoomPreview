/******************************************************************************
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

#include "LPImager.h"

#include <QByteArray>
#include <QBitArray>
#include <QFile>
#include <QFileInfo>
#include <QImage>

#include <math.h>
#include <assert.h>

namespace LP
{
   Imager::Imager()
   : m_pixelFunc( &LP::Imager::pixelRGB )
   {
   }
   
   bool Imager::load( const QString& filename, unsigned int blockSize )
   {
      QFile f( filename );
      bool  success( false );

      unload();

      m_blockSize = blockSize;

      if ( f.open( QIODevice::ReadOnly ) )
      {
         QFileInfo   fi( f );
         QByteArray  ba;
         qint64      fileSize( fi.size() );
         int         bytesToRead( m_blockSize );

         for ( qint64 offset = 0; offset < fileSize; offset+=m_blockSize )
         {
            if ( offset + m_blockSize > fileSize )
               bytesToRead = fileSize - offset;
            ba = f.read( bytesToRead );

            //emit progress( offset / m_blockSize, fileSize / m_blockSize );

            QBitArray*  bits( new QBitArray( bytesToRead * 8 ) );

            for ( int i = 0; i < ba.size(); ++i )
            {
               unsigned int byte( ba[i] );

               for ( int j = 0; j < 8; ++j )
                  bits->setBit( i * 8 + j, ( byte >> j ) & 1 );
            }
            assert( bits->size() == bytesToRead * 8 );
            m_bitArrayVec.push_back( bits );
         }
         //emit progress( fileSize / m_blockSize, fileSize / m_blockSize );
         success = true;
         f.close();
      }

      return success;
   }


   Imager::~Imager()
   {
      unload();
   }


   void Imager::regenerate( unsigned int redBitCount,
                    unsigned int greenBitCount,
                    unsigned int blueBitCount,
                    unsigned int grayBitCount,
                    ChannelOrder order,
                    unsigned int width,
                    unsigned int offset,
                    std::vector< QImage* >& imgVec
                  )
   {
      m_redBitCount = redBitCount;
      m_greenBitCount = greenBitCount;
      m_blueBitCount = blueBitCount;
      m_grayBitCount = grayBitCount;
     
      m_bitsPerPixel = m_redBitCount + m_greenBitCount + m_blueBitCount;

      if ( m_bitsPerPixel < 1 )
         m_bitsPerPixel = m_redBitCount = 1;

      if ( order == RGB )
         m_pixelFunc = &LP::Imager::pixelRGB;
      else if ( order == RBG ) 
         m_pixelFunc = &LP::Imager::pixelRBG;
      else if ( order == GBR )
         m_pixelFunc = &LP::Imager::pixelGBR;
      else if ( order == GRB )
         m_pixelFunc = &LP::Imager::pixelGRB;
      else if ( order == BRG )
         m_pixelFunc = &LP::Imager::pixelBRG;
      else if ( order == BGR )
         m_pixelFunc = &LP::Imager::pixelBGR;
      else if ( order == Grayscale )
      {
         m_pixelFunc = &LP::Imager::pixelGray;
         m_bitsPerPixel = m_grayBitCount;
         if ( m_bitsPerPixel < 1 )
            m_bitsPerPixel = m_grayBitCount = 1;
      }

      size_t bai = 0;

      // Skip bytes as dictated by offset
      if ( offset > 0 )
      {
         while ( bai < m_bitArrayVec.size() && offset > m_bitArrayVec[bai]->size() / 8 )
         {
            offset -= m_bitArrayVec[bai]->size() / 8;
            ++bai;
         }
      }

      unsigned int    i, j, di, limit;

      di = offset * 8;

      while ( bai < m_bitArrayVec.size() )
      {
         int height( m_blockSize * 8 / ( width * m_bitsPerPixel ) );

         if ( bai == m_bitArrayVec.size() - 1 )
         {
            height =  ( m_bitArrayVec[bai]->size() - di ) / ( width * m_bitsPerPixel );
         }
         else if ( m_bitArrayVec[bai]->size() - di + m_bitArrayVec[bai+1]->size() < m_blockSize * 8 )
         {
            height =  ( m_bitArrayVec[bai]->size() - di + m_bitArrayVec[bai+1]->size() ) / ( width * m_bitsPerPixel );
         }

         if ( height == 0 )
            return;

         i = 0;
         j = 0;

         limit = m_bitArrayVec[bai]->size();

         QImage* dst_img( new QImage( width, height, QImage::Format_RGB32 ) );

         while( j < height )
         {
            dst_img->setPixel( i, j, (this->*m_pixelFunc)( di, bai, limit ) ); 
            if ( ++i == width )
            {
               i = 0;
               ++j;
            }
         }

         imgVec.push_back( dst_img );

         if ( di >= limit )
            ++bai;
      }

   }



   void Imager::unload()
   {
      for( size_t i = 0; i < m_bitArrayVec.size(); ++i )
         delete m_bitArrayVec[i];
      m_bitArrayVec.clear();
   }


   unsigned char Imager::mapBits( int bitCount, unsigned int& di, unsigned int& bai, unsigned int& limit )
   {
      if ( ! bitCount )
         return 0;

      float scale( 255.0 / ( pow( 2.0f, bitCount ) - 1 ) );
      int val( 0 );

      while( bitCount-- )
      {
         val *= 2;
         if ( m_bitArrayVec[bai]->testBit(di++) )
            val += 1;

         if ( di >= limit )
         {
            di = 0;
            ++bai;
            if ( bai >= m_bitArrayVec.size() )
               return 0;

            limit = m_bitArrayVec[bai]->size();
         }
      }

      return val * scale;
   }


   uint Imager::pixelRGB( unsigned int& di, unsigned int& bai, unsigned int& limit )
   {
      unsigned char r( mapBits( m_redBitCount, di, bai, limit ) );
      unsigned char g( mapBits( m_greenBitCount, di, bai, limit ) );
      unsigned char b( mapBits( m_blueBitCount, di, bai, limit ) );

      return qRgb( r, g, b );
   }




   uint Imager::pixelRBG( unsigned int& di, unsigned int& bai, unsigned int& limit )
   {
      unsigned char r( mapBits( m_redBitCount, di, bai, limit ) );
      unsigned char b( mapBits( m_blueBitCount, di, bai, limit ) );
      unsigned char g( mapBits( m_greenBitCount, di, bai, limit ) );

      return qRgb( r, g, b );
   }




   uint Imager::pixelGBR( unsigned int& di, unsigned int& bai, unsigned int& limit )
   {
      unsigned char g( mapBits( m_greenBitCount, di, bai, limit ) );
      unsigned char b( mapBits( m_blueBitCount, di, bai, limit ) );
      unsigned char r( mapBits( m_redBitCount, di, bai, limit ) );

      return qRgb( r, g, b );
   }




   uint Imager::pixelGRB( unsigned int& di, unsigned int& bai, unsigned int& limit )
   {
      unsigned char g( mapBits( m_greenBitCount, di, bai, limit ) );
      unsigned char r( mapBits( m_redBitCount, di, bai, limit ) );
      unsigned char b( mapBits( m_blueBitCount, di, bai, limit ) );

      return qRgb( r, g, b );
   }




   uint Imager::pixelBGR( unsigned int& di, unsigned int& bai, unsigned int& limit )
   {
      unsigned char b( mapBits( m_blueBitCount, di, bai, limit ) );
      unsigned char g( mapBits( m_greenBitCount, di, bai, limit ) );
      unsigned char r( mapBits( m_redBitCount, di, bai, limit ) );

      return qRgb( r, g, b );
   }




   uint Imager::pixelBRG( unsigned int& di, unsigned int& bai, unsigned int& limit )
   {
      unsigned char b( mapBits( m_blueBitCount, di, bai, limit ) );
      unsigned char r( mapBits( m_redBitCount, di, bai, limit ) );
      unsigned char g( mapBits( m_greenBitCount, di, bai, limit ) );

      return qRgb( r, g, b );
   }




   uint Imager::pixelGray( unsigned int& di, unsigned int& bai, unsigned int& limit )
   {
      unsigned char gr( mapBits( m_grayBitCount, di, bai, limit ) );
      return qRgb( gr, gr, gr );
   }



}  // namespace LP



