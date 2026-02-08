#include "widget.h"
#include "menuwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MenuWindow menu;
    Widget gameWindow;

    QObject::connect(&menu, &MenuWindow::startGameRequested, [&]() {
        gameWindow.showFullScreen();
        gameWindow.startGame();  // Appelle le vrai démarrage logique du jeu
    });

    menu.showFullScreen();
    return a.exec();
}
