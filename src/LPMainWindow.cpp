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




#include <QUiLoader>
#include <QMessageBox>
#include <QLabel>
#include <QCloseEvent>
#include <QFileDialog>
#include <QTextStream>
#include <QShortcut>
#include <QTime>
#include <QFileInfo>
#include <QSettings>

#include "LPMainWindow.h"

#include <assert.h>
#include <math.h>



namespace LPUI
{


MainWindow::MainWindow(QWidget *parent)
: QMainWindow( parent )
, m_pixelFunc( &LPUI::MainWindow::pixelRGB )
{
   m_ui.setupUi(this);

   m_previewImageLabel = new QLabel();

   m_ui.m_previewScrollArea->setWidget(m_previewImageLabel);

   m_redBitCount = 3;
   m_greenBitCount = 2;
   m_blueBitCount = 3;

   m_ui.m_redBitsSpinBox->setValue( m_redBitCount );
   m_ui.m_greenBitsSpinBox->setValue( m_greenBitCount );
   m_ui.m_blueBitsSpinBox->setValue( m_blueBitCount );

   connect(m_ui.actionOpen,
      SIGNAL(triggered()),
      SLOT(onOpenActionTriggered()));

   connect(m_ui.actionExport,
      SIGNAL(triggered()),
      SLOT(onExportActionTriggered()));

   connect(m_ui.actionExit,
      SIGNAL(triggered()),
      SLOT(onExitActionTriggered()));

   connect(m_ui.m_redBitsSpinBox,
      SIGNAL( valueChanged(int) ),
      SLOT(onRedChannelMaskChanged()));

   connect(m_ui.m_greenBitsSpinBox,
      SIGNAL( valueChanged(int) ),
      SLOT(onGreenChannelMaskChanged()));

   connect(m_ui.m_blueBitsSpinBox,
      SIGNAL( valueChanged(int) ),
      SLOT(onBlueChannelMaskChanged()));

   connect(m_ui.m_grayBitsSpinBox,
      SIGNAL( valueChanged(int) ),
      SLOT(onGrayChannelMaskChanged()));

   connect(m_ui.m_widthLineEdit,
      SIGNAL( textEdited(const QString&) ),
      SLOT(onWidthLineEditChanged()));

   connect(m_ui.m_widthSlider,
      SIGNAL( valueChanged(int) ),
      SLOT(onWidthSliderChanged(int)));

   connect(m_ui.m_offsetLineEdit,
      SIGNAL( textEdited(const QString&) ),
      SLOT(onOffsetLineEditChanged()));

   connect(m_ui.m_offsetSlider,
      SIGNAL( valueChanged(int) ),
      SLOT(onOffsetSliderChanged(int)));

   connect(m_ui.m_rgbChOrderRadioButton,
      SIGNAL( clicked() ),
      SLOT(onChannelOrderChanged()) );

   connect(m_ui.m_rbgChOrderRadioButton,
      SIGNAL( clicked() ),
      SLOT(onChannelOrderChanged()) );

   connect(m_ui.m_gbrChOrderRadioButton,
      SIGNAL( clicked() ),
      SLOT(onChannelOrderChanged()) );

   connect(m_ui.m_grbChOrderRadioButton,
      SIGNAL( clicked() ),
      SLOT(onChannelOrderChanged()) );

   connect(m_ui.m_brgChOrderRadioButton,
      SIGNAL( clicked() ),
      SLOT(onChannelOrderChanged()) );

   connect(m_ui.m_bgrChOrderRadioButton,
      SIGNAL( clicked() ),
      SLOT(onChannelOrderChanged()) );

   connect(m_ui.m_grayChOrderRadioButton,
      SIGNAL( clicked() ),
      SLOT(onChannelOrderChanged()) );
}


MainWindow::~MainWindow()
{
}



void MainWindow::onExitActionTriggered()
{
   close();
}




void MainWindow::onOpenActionTriggered()
{
   QString filename;

   filename = QFileDialog::getOpenFileName( this, 
                           tr("Choose a source data file"), 
                           QString(), 	// Starting dir
                           tr("All Files (*.*)") );

   if ( ! filename.isEmpty() )
   {
      QFile f( filename );
      QFileInfo   fi( filename );
      int   maxSize( INT_MAX / 8 );

      if ( fi.size() > maxSize )
      {
         // Currently we're limited by the size of the index type (signed int) of
         // QBitArray's accessor functions.
         QMessageBox::critical( this, tr("File Too Big"),
               tr("File too big.  Max size is %1 MB (%2 bytes). Sorry!")
                  .arg( maxSize / (1024 * 1024) )
                  .arg( maxSize ) );
         return;
      }
      if ( f.open( QIODevice::ReadOnly ) )
      {
         m_inputData = f.readAll();
         f.close();
         m_sourceFilename = filename;
         m_ui.m_sourceFilenameLabel->setText( filename );

         m_ui.m_offsetSlider->setMaximum( m_inputData.size() - 1 );
         m_inputBits.resize( m_inputData.size() * 8 );

         for ( int i = 0; i < m_inputData.size(); ++i )
         {
            unsigned int byte( m_inputData[i] );

            for ( int j = 0; j < 8; ++j )
               m_inputBits.setBit( i * 8 + j, ( byte >> j ) & 1 );
         }
         recomputePreview();
      }
      else
      {
         QMessageBox::critical( this, tr("File Open Failed"), 
               tr("Could not open file.  Sorry.") );
      }
   }
}


void MainWindow::onExportActionTriggered()
{
   if ( m_sourceFilename.isEmpty() )
   {
      QMessageBox::warning( this, tr("No source file"),
            tr("No data file has been loaded yet!") );
      return;
   }

   QString   filename;
   QFileInfo fi( m_sourceFilename );

   filename = fi.absolutePath() + "/" + fi.completeBaseName() + ".tiff";
   filename = QFileDialog::getSaveFileName( this, 
                           tr("Specify output file"), 
                           filename,
                           tr("TIFF Files (*.tiff)") );

   if ( ! filename.isEmpty() )
   {
      regenerate(filename);
   }
}


void MainWindow::onChannelOrderChanged()
{
   if ( m_ui.m_rgbChOrderRadioButton->isChecked() )
      m_pixelFunc = &LPUI::MainWindow::pixelRGB;
   else if ( m_ui.m_rbgChOrderRadioButton->isChecked() )
      m_pixelFunc = &LPUI::MainWindow::pixelRBG;
   else if ( m_ui.m_gbrChOrderRadioButton->isChecked() )
      m_pixelFunc = &LPUI::MainWindow::pixelGBR;
   else if ( m_ui.m_grbChOrderRadioButton->isChecked() )
      m_pixelFunc = &LPUI::MainWindow::pixelGRB;
   else if ( m_ui.m_brgChOrderRadioButton->isChecked() )
      m_pixelFunc = &LPUI::MainWindow::pixelBRG;
   else if ( m_ui.m_bgrChOrderRadioButton->isChecked() )
      m_pixelFunc = &LPUI::MainWindow::pixelBGR;
   else if ( m_ui.m_grayChOrderRadioButton->isChecked() )
      m_pixelFunc = &LPUI::MainWindow::pixelGray;

   recomputePreview();
}






void MainWindow::onRedChannelMaskChanged()
{
   recomputePreview();
}



void MainWindow::onBlueChannelMaskChanged()
{
   recomputePreview();
}



void MainWindow::onGreenChannelMaskChanged()
{
   recomputePreview();
}


void MainWindow::onGrayChannelMaskChanged()
{
   recomputePreview();
}




void MainWindow::onWidthLineEditChanged()
{
   int w( m_ui.m_widthLineEdit->text().toInt() );

   if ( w < m_ui.m_widthSlider->minimum() )
   {
      w = m_ui.m_widthSlider->minimum();
      m_ui.m_widthLineEdit->setText( QString::number( w ) );
   }
   else if ( w > m_ui.m_widthSlider->maximum() )
   {
      w = m_ui.m_widthSlider->maximum();
      m_ui.m_widthLineEdit->setText( QString::number( w ) );
   }

   m_ui.m_widthSlider->setValue( w );
   recomputePreview();
}



void MainWindow::onWidthSliderChanged(int val)
{
   m_ui.m_widthLineEdit->setText( QString::number( val ) );
   recomputePreview();
}



void MainWindow::onOffsetLineEditChanged()
{
   m_ui.m_offsetSlider->setValue( m_ui.m_offsetLineEdit->text().toInt() );
   recomputePreview();
}



void MainWindow::onOffsetSliderChanged(int val)
{
   m_ui.m_offsetLineEdit->setText( QString::number( val ) );
   recomputePreview();
}





void MainWindow::closeEvent( QCloseEvent* event )
{
   event->accept();
}




void MainWindow::recomputePreview()
{
   regenerate();
}



void MainWindow::regenerate( const QString& filename )
{
   int width( m_ui.m_widthLineEdit->text().toInt() );
   if ( width < 1 )
      width = 1;
   int offset( m_ui.m_offsetLineEdit->text().toInt() );

   m_redBitCount = m_ui.m_redBitsSpinBox->value();
   m_greenBitCount = m_ui.m_greenBitsSpinBox->value();
   m_blueBitCount = m_ui.m_blueBitsSpinBox->value();
   m_grayBitCount = m_ui.m_grayBitsSpinBox->value();

   int bitsPerPixel;
  
   if ( m_ui.m_grayChOrderRadioButton->isChecked() )
   {
      bitsPerPixel = m_grayBitCount;
      if ( bitsPerPixel < 1 )
         bitsPerPixel = m_grayBitCount = 1;
   }
   else
   {
      bitsPerPixel = m_redBitCount + m_greenBitCount + m_blueBitCount;

      if ( bitsPerPixel < 1 )
         bitsPerPixel = m_redBitCount = 1;
   }

   int height( 8 * ( m_inputData.size() - offset ) / ( width * bitsPerPixel ) );
   QImage dst_img( width, height, QImage::Format_RGB32 );
   int    i, j;

   i = 0;
   j = 0;

   for ( int di = offset * 8, limit = m_inputBits.size(); di+bitsPerPixel < limit; di+=bitsPerPixel )
   {
      dst_img.setPixel( i, j, (this->*m_pixelFunc)( di ) ); 
      if ( ++i == width )
      {
         i = 0;
         ++j;
      }
   }

   if ( ! filename.isEmpty() )
   {
      if ( ! dst_img.save( filename, "TIFF" ) )
      {
         QMessageBox::critical( this, tr("Save Failed"),
               tr("Could not save file.") );
      }
   }

   m_previewImageLabel->setPixmap( QPixmap::fromImage(dst_img));
}


unsigned char mapBits( QBitArray& ba, unsigned int& idx, unsigned int bitCount )
{
   if ( ! bitCount )
      return 0;

   float scale( 255.0 / ( pow( 2.0f, (int) bitCount ) - 1 ) );
   int val( 0 );

   while( bitCount-- )
   {
      val *= 2;
      if ( ba.testBit(idx++) )
         val += 1;
   }

   return val * scale;
}


uint MainWindow::pixelRGB( unsigned int idx )
{
   unsigned char r( mapBits( m_inputBits, idx, m_redBitCount ) );
   unsigned char g( mapBits( m_inputBits, idx, m_greenBitCount ) );
   unsigned char b( mapBits( m_inputBits, idx, m_blueBitCount ) );

   return qRgb( r, g, b );
}




uint MainWindow::pixelRBG( unsigned int idx )
{
   unsigned char r( mapBits( m_inputBits, idx, m_redBitCount ) );
   unsigned char b( mapBits( m_inputBits, idx, m_blueBitCount ) );
   unsigned char g( mapBits( m_inputBits, idx, m_greenBitCount ) );

   return qRgb( r, g, b );
}




uint MainWindow::pixelGBR( unsigned int idx )
{
   unsigned char g( mapBits( m_inputBits, idx, m_greenBitCount ) );
   unsigned char b( mapBits( m_inputBits, idx, m_blueBitCount ) );
   unsigned char r( mapBits( m_inputBits, idx, m_redBitCount ) );

   return qRgb( r, g, b );
}




uint MainWindow::pixelGRB( unsigned int idx )
{
   unsigned char g( mapBits( m_inputBits, idx, m_greenBitCount ) );
   unsigned char r( mapBits( m_inputBits, idx, m_redBitCount ) );
   unsigned char b( mapBits( m_inputBits, idx, m_blueBitCount ) );

   return qRgb( r, g, b );
}




uint MainWindow::pixelBGR( unsigned int idx )
{
   unsigned char b( mapBits( m_inputBits, idx, m_blueBitCount ) );
   unsigned char g( mapBits( m_inputBits, idx, m_greenBitCount ) );
   unsigned char r( mapBits( m_inputBits, idx, m_redBitCount ) );

   return qRgb( r, g, b );
}




uint MainWindow::pixelBRG( unsigned int idx )
{
   unsigned char b( mapBits( m_inputBits, idx, m_blueBitCount ) );
   unsigned char r( mapBits( m_inputBits, idx, m_redBitCount ) );
   unsigned char g( mapBits( m_inputBits, idx, m_greenBitCount ) );

   return qRgb( r, g, b );
}




uint MainWindow::pixelGray( unsigned int idx )
{
   unsigned char gr( mapBits( m_inputBits, idx, m_grayBitCount ) );
   return qRgb( gr, gr, gr );
}






}; // Namespace LPUI


