#include "mainwindow.h"
#include "scenegl.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    scene = new SceneGL(this);
    setCentralWidget(scene);
    resize(800, 600);
    setWindowTitle("Jeu de Défense - Scène 3D");
}

MainWindow::~MainWindow() {}
