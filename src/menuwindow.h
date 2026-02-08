#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include <QWidget>
#include <QFileInfo>
#include <QLabel>
#include <windows.h>


#include "ui_menuwindow.h"  // Ajoute bien ça !

class MenuWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MenuWindow(QWidget *parent = nullptr);
    ~MenuWindow();
    void resizeEvent(QResizeEvent *event);
    void playMenuMusic();
    void stopMenuMusic();



signals:
    void startGameRequested();  // Ton signal pour démarrer

private slots:
    void on_startButton_clicked();  // Ton slot pour le bouton
    void on_quitButton_clicked();

private:
    Ui::MenuWindow *ui;  // Important pour relier le .ui
    QLabel* backgroundLabel;   // Label pour l'image de fond
};

#endif // MENUWINDOW_H
