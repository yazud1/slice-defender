#include "menuwindow.h"
#include "ui_menuwindow.h"  // Assure-toi que c'est là
#include <QApplication>
#include <QFontDatabase>

MenuWindow::MenuWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MenuWindow)
{
    ui->setupUi(this);
    int id = QFontDatabase::addApplicationFont(QCoreApplication::applicationDirPath() + "/images/minecraft_font.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont minecraftFont(family, 48);  // 24 est la taille, à ajuster
    ui->titleLabel->setFont(minecraftFont);
    qDebug() << "Police appliquée :" << ui->titleLabel->font().family();
    ui->titleLabel->setAlignment(Qt::AlignCenter);

    backgroundLabel = new QLabel(this);
    backgroundLabel->setPixmap(QPixmap(QCoreApplication::applicationDirPath() + "/images/fond_mc.png"));
    backgroundLabel->setScaledContents(true);
    backgroundLabel->setGeometry(this->rect());  // Remplit toute la fenêtre
    backgroundLabel->lower();  // Envoie ce label derrière tous les autres widgets

    // Rendre les widgets enfants transparents
    ui->titleLabel->setStyleSheet("background-color: rgba(255,255,255,150);");
    ui->descLabel->setStyleSheet("background-color: rgba(255,255,255,150);");

    ui->titleLabel->setStyleSheet("background: transparent; border: none;");
    ui->descLabel->setStyleSheet("background: transparent; border: none;");
    ui->startButton->setStyleSheet(
                       "QPushButton {"
                       "   border-image: url(:/images/fond_boutton.png) 4 4 4 4 stretch stretch;"
                       "   background-color: rgb(130, 130, 130);"  // Gris clair opaque proche texture
                       "   color: white;"
                       "   font-weight: bold;"
                       "   font-size: 16px;"
                       "}"
                       "QPushButton:hover {"
                       "   background-color: rgb(150, 150, 150);"  // Un peu plus clair au survol"
                       "}"
                       "QPushButton:pressed {"
                       "   background-color: rgb(90, 90, 90);"  // Plus foncé quand pressé"
                       "}");
    ui->quitButton->setStyleSheet(ui->startButton->styleSheet());
    ui->titleLabel->setStyleSheet("color: yellow; background: transparent; border: none;");
    ui->descLabel->setStyleSheet("color: yellow; background: transparent; border: none;");
    playMenuMusic();


}

void MenuWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    backgroundLabel->setGeometry(this->rect());
}



void MenuWindow::on_quitButton_clicked()
{
    stopMenuMusic();
    qApp->quit();
}

void MenuWindow::playMenuMusic()
{
    // Joue le son en boucle, non bloquant
    PlaySound(TEXT("images/mc_accueil.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}

void MenuWindow::stopMenuMusic()
{
    PlaySound(NULL, 0, 0); // Stoppe le son
}



MenuWindow::~MenuWindow()
{
    delete ui;
}

void MenuWindow::on_startButton_clicked()
{
    PlaySound(TEXT("images/start.wav"), NULL, SND_FILENAME | SND_SYNC);

    emit startGameRequested();  // Lance la partie après la transition sonore
    this->close();  // Ferme le menu
}
