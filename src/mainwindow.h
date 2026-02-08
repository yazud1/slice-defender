#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class SceneGL;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    SceneGL* scene;
};

#endif // MAINWINDOW_H
