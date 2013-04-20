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

#ifndef LDMAINWINDOW_H
#define LDMAINWINDOW_H

#include "ui_MainWindow.h"

#include <QDir>
#include <QFileInfo>
#include <QBitArray>

#include <map>

// Forward declarations
class QLabel;
class QShortcut;


namespace LPUI
{


/**@brief The main application window. */
class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
   /// Standard constructor
   MainWindow(QWidget *parent = 0);
   /// Standard destructor
   virtual ~MainWindow();

private slots:

   /// Responds to File->Open
   void onOpenActionTriggered();
   /// Responds to File->Export
   void onExportActionTriggered();
   /// Responds to the user requesting to exit the app.
   void onExitActionTriggered();

   void recomputePreview();

   void onChannelOrderChanged();

   void onRedChannelMaskChanged();
   void onGreenChannelMaskChanged();
   void onBlueChannelMaskChanged();
   void onGrayChannelMaskChanged();

   void onWidthLineEditChanged();
   void onWidthSliderChanged(int);
   void onOffsetLineEditChanged();
   void onOffsetSliderChanged(int);

signals:

private:
   /**@brief Override of base function. */
   virtual void closeEvent( QCloseEvent* );

   void regenerate( const QString& filename = QString() );

   uint pixelRGB( unsigned int idx );
   uint pixelRBG( unsigned int idx );
   uint pixelGBR( unsigned int idx );
   uint pixelGRB( unsigned int idx );
   uint pixelBGR( unsigned int idx );
   uint pixelBRG( unsigned int idx );
   uint pixelGray( unsigned int idx );

   typedef uint (LPUI::MainWindow::*PixelFn)(unsigned int);
   PixelFn   m_pixelFunc;

   /// The Designer-generated user interface object.
   Ui::MainWindow		m_ui;

   QLabel*  m_previewImageLabel;

   QString    m_sourceFilename;
   QByteArray m_inputData;
   QBitArray  m_inputBits;

   unsigned char  m_redBitCount, m_greenBitCount, m_blueBitCount, m_grayBitCount;
}; 

}	// namespace LDUI

#endif	// LDMAINWINDOW_H


