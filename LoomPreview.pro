CONFIG += qt
CONFIG += uitools debug_and_release console
UI_HEADERS_DIR = ./ui_inc
MOC_DIR = ./moc
RCC_DIR = ./qrc
OBJECTS_DIR = ./release
DESTDIR = ./release
CONFIG(debug,debug|release) {
	OBJECTS_DIR = ./debug
	DESTDIR = ./debug
}
TARGET = LoomPreview

win32 {
	SLASH = \\
}
linux-g++ {
	SLASH = /
}

INCLUDEPATH += src
INCLUDEPATH += $${UI_HEADERS_DIR}


TEMPLATE = app

FORMS = ui/MainWindow.ui 

RESOURCES = ui/LoomPreview.qrc

SOURCES +=  src/LPImager.cpp \
            src/LPMain.cpp \
            src/LPMainWindow.cpp 

HEADERS +=  src/LPImager.h \
            src/LPMainWindow.h 

