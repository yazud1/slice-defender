QT += core gui widgets opengl openglwidgets

TARGET = MonJeu
TEMPLATE = app

SOURCES += \
    fragment.cpp \
    main.cpp \
    mainwindow.cpp \
    menuwindow.cpp \
    scenegl.cpp \
    projectile.cpp \
    widget.cpp

HEADERS += \
    fragment.h \
    mainwindow.h \
    menuwindow.h \
    scenegl.h \
    projectile.h \
    widget.h

LIBS += -lopengl32 -lglu32 -lwinmm
QT += core gui opengl widgets
CONFIG += c++11

# OpenCV : adapte selon ton système
INCLUDEPATH +=$$(OPENCV_DIR)\..\..\include

LIBS += -L$$(OPENCV_DIR)\lib \
        -lopencv_core4110 \
        -lopencv_imgproc4110 \
        -lopencv_highgui4110 \
        -lopencv_video4110 \
        -lopencv_videoio4110 \
        -lopencv_objdetect4110

FORMS += \
    menuwindow.ui \
    widget.ui

RESOURCES +=
    resources.qrc
