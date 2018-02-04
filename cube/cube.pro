QT += core gui widgets

TARGET = cube
TEMPLATE = app

SOURCES += main.cpp \
    engine.cpp \
    glbox.cpp \
    GLView.cpp \
    jpegload.cpp \
    globjwin.cpp \
    coordconv.cpp

SOURCES += \
    mainwidget.cpp \
    geometryengine.cpp

HEADERS += \
    mainwidget.h \
    geometryengine.h \
    globals.h \
    engine.h \
    glbox.h \
    GLView.h \
    globjwin.h \
    jpeglib.h \
    coordconv.h

RESOURCES += \
    shaders.qrc \
    textures.qrc

QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS   += -std=c++11

# install
target.path = $$[QT_INSTALL_EXAMPLES]/opengl/cube
INSTALLS += target
