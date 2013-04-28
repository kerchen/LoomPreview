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

#include "LPImager.h"


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

   void onBlockSizeLineEditChanged();
   void onBlockSizeSliderChanged(int);
   void onWidthLineEditChanged();
   void onWidthSliderChanged(int);
   void onOffsetLineEditChanged();
   void onOffsetSliderChanged(int);

signals:

private:
   /**@brief Override of base function. */
   virtual void closeEvent( QCloseEvent* );

   void regenerate( const QString& filename = QString() );

   /// The Designer-generated user interface object.
   Ui::MainWindow		m_ui;

   QFrame*  m_imagePreviewFrame;
   std::vector< QLabel* >  m_displayedLabels;

   LP::Imager*  m_imager;

   QString    m_sourceFilename;

   unsigned char  m_redBitCount, m_greenBitCount, m_blueBitCount, m_grayBitCount;

   LP::Imager::ChannelOrder   m_channelOrder;
}; 

}	// namespace LDUI

#endif	// LDMAINWINDOW_H


