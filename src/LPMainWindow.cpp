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
#include <QProgressDialog>
#include <QTextStream>
#include <QThread>
#include <QShortcut>
#include <QTime>
#include <QFileInfo>
#include <QSettings>

#include "LPMainWindow.h"
#include "LPImager.h"

#include <assert.h>
#include <math.h>



namespace LPUI
{
#if 0
   class DataLoader : public QThread
   {
   public:
      DataLoader( LP::Imager* imgr, const QString& filename ) 
         : QThread() 
         , m_filename( filename )
         , m_imager( imgr )
      { }

      virtual void run()
      {
         m_imager->load( m_filename );
      }

   private:
      QString     m_filename;
      LP::Imager* m_imager;
   };
#endif


MainWindow::MainWindow(QWidget *parent)
: QMainWindow( parent )
, m_imager(NULL)
, m_channelOrder( LP::Imager::RGB )
{
   m_ui.setupUi(this);

   m_imagePreviewFrame = new QFrame();
   m_imagePreviewFrame->setLayout( new QVBoxLayout( m_imagePreviewFrame ) );

   m_ui.m_previewScrollArea->setWidget(m_imagePreviewFrame);

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

   connect(m_ui.m_blockSizeLineEdit,
      SIGNAL( textEdited(const QString&) ),
      SLOT(onBlockSizeLineEditChanged()));

   connect(m_ui.m_blockSizeSlider,
      SIGNAL( valueChanged(int) ),
      SLOT(onBlockSizeSliderChanged(int)));

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
      LP::Imager*  newImg( new LP::Imager() );
      // DataLoader   loader( newImg, filename );
      if ( newImg->load( filename, m_ui.m_blockSizeSlider->value() * 1024 * 1024 ) )
      {
         QFileInfo   fi( filename );

         delete m_imager;
         m_imager = newImg;
         m_sourceFilename = filename;
         m_ui.m_sourceFilenameLabel->setText( filename );
         m_ui.m_sourceSizeLabel->setText( QString::number( fi.size() ) + tr(" bytes") );
         regenerate();
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
                           tr("Specify base output filename"), 
                           filename,
                           tr("TIFF Files (*.tiff)") );

   if ( ! filename.isEmpty() )
   {
      regenerate( filename );
   }
}


void MainWindow::onChannelOrderChanged()
{
   if ( m_ui.m_rgbChOrderRadioButton->isChecked() )
      m_channelOrder = LP::Imager::RGB;
   else if ( m_ui.m_rbgChOrderRadioButton->isChecked() )
      m_channelOrder = LP::Imager::RBG;
   else if ( m_ui.m_gbrChOrderRadioButton->isChecked() )
      m_channelOrder = LP::Imager::GBR;
   else if ( m_ui.m_grbChOrderRadioButton->isChecked() )
      m_channelOrder = LP::Imager::GRB;
   else if ( m_ui.m_brgChOrderRadioButton->isChecked() )
      m_channelOrder = LP::Imager::BRG;
   else if ( m_ui.m_bgrChOrderRadioButton->isChecked() )
      m_channelOrder = LP::Imager::BGR;
   else if ( m_ui.m_grayChOrderRadioButton->isChecked() )
      m_channelOrder = LP::Imager::Grayscale;

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


void MainWindow::onBlockSizeLineEditChanged()
{
   int bs( m_ui.m_blockSizeLineEdit->text().toInt() );

   if ( bs < m_ui.m_blockSizeSlider->minimum() )
   {
      bs = m_ui.m_blockSizeSlider->minimum();
      m_ui.m_blockSizeLineEdit->setText( QString::number( bs ) );
   }
   else if ( bs > m_ui.m_blockSizeSlider->maximum() )
   {
      bs = m_ui.m_blockSizeSlider->maximum();
      m_ui.m_blockSizeLineEdit->setText( QString::number( bs ) );
   }

   m_ui.m_blockSizeSlider->setValue( bs );
}




void MainWindow::onBlockSizeSliderChanged(int val)
{
   m_ui.m_blockSizeLineEdit->setText( QString::number( val ) );
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
   unsigned int width( m_ui.m_widthLineEdit->text().toInt() );
   if ( width < 1 )
      width = 1;
   unsigned int offset( m_ui.m_offsetLineEdit->text().toInt() );

   m_redBitCount = m_ui.m_redBitsSpinBox->value();
   m_greenBitCount = m_ui.m_greenBitsSpinBox->value();
   m_blueBitCount = m_ui.m_blueBitsSpinBox->value();
   m_grayBitCount = m_ui.m_grayBitsSpinBox->value();

   if ( m_imager )
   {
      std::vector< QImage* >   imgVec;

      m_imager->regenerate( m_redBitCount, m_greenBitCount, 
            m_blueBitCount, m_grayBitCount, m_channelOrder,
            width, offset, imgVec );

      QLayout* layout = m_imagePreviewFrame->layout();
      for ( size_t i = 0; i < m_displayedLabels.size(); ++i )
      {
         layout->removeWidget( m_displayedLabels[i] );
         delete m_displayedLabels[i];
      }
      m_displayedLabels.clear();

      delete layout;

      layout = new QVBoxLayout( m_imagePreviewFrame );
      layout->setSpacing( 1 );

      for ( size_t i = 0; i < imgVec.size(); ++i )
      {
         QLabel*  stats = new QLabel( m_imagePreviewFrame );
         stats->setText( tr("Image %1 (of %2): %3 x %4")
                           .arg( i+1 )
                           .arg( imgVec.size() )
                           .arg( imgVec[i]->width() )
                           .arg( imgVec[i]->height() ) );
         layout->addWidget( stats ); 
         m_displayedLabels.push_back( stats );

         QLabel*  image( new QLabel( m_imagePreviewFrame ) );
         image->setPixmap( QPixmap::fromImage( *imgVec[i] ) );
         layout->addWidget( image );
         m_displayedLabels.push_back( image );
      }
      m_imagePreviewFrame->setLayout( layout );

      if ( ! filename.isEmpty() )
      {
         QFileInfo   fi( filename );

         for ( size_t i = 0; i < imgVec.size(); ++i )
         {
            QString  fname( fi.absolutePath() + "/" + fi.completeBaseName() + 
                  QString::number(i+1) + "_of_" + QString::number(imgVec.size()) + "." +
                  fi.suffix() );

            if ( ! imgVec[i]->save( fname, "TIFF" ) )
            {
               QMessageBox::critical( this, tr("Save Failed"),
                     tr("Could not save file.") );
            }
         }
      }

      for ( size_t i = 0; i < imgVec.size(); ++i )
         delete imgVec[i];
      imgVec.clear();
   }
}




}; // Namespace LPUI


