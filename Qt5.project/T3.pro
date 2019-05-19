#-------------------------------------------------
#
# Project created by QtCreator 2019-04-15T04:58:37
#
#-------------------------------------------------

QT       += core gui xml opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = T3
TEMPLATE = app

#-------------------------------------------------
VERSION = 0.9.0    # major.minor.patch
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += APP_NAME=\"\\\"$$TARGET\\\"\"
DEFINES += APP_VERSION=\"\\\"$$VERSION\\\"\"
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        gl_viewer.cpp \
        gl_robot.cpp \
        gl_particle.cpp

HEADERS += \
        mainwindow.h \
        gl_viewer.h \
        gl_robot.h \
        gl_particle.h \
        fabrik.h \
    my_utils.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


win32: {
    LIBS += -lopengl32 -lglu32
    CONFIG(release, debug|release):  {
        LIBS += "$$PWD/../../libQGLViewer/QGLViewer/QGLViewer2.dll"
        message("Release!")
    } else {
        LIBS += "$$PWD/../../libQGLViewer/QGLViewer/QGLViewerd2.dll"
        message("Debug!")
        }
    }
else:mac: LIBS += -F$$PWD/../../libQGLViewer/QGLViewer/ -framework QGLViewer
else:unix: LIBS += -L$$PWD/../../libQGLViewer/QGLViewer/ -lQGLViewer

INCLUDEPATH += $$PWD/../../libQGLViewer/QGLViewer
DEPENDPATH += $$PWD/../../libQGLViewer/QGLViewer

#For Windows: remember copy QGLViewerxx.dll to Windows/System32
#For Mac: remember to add QGLViewer framework path to DYLD_FRAMEWORK_PATH
macx|darwin-g++ {#for lolocating OpenGL/glu.h
    # Or whatever exists in /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/
    QMAKE_MAC_SDK = macosx10.14
}
