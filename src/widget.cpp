#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QMutexLocker>
#include <QImage>
#include <QPixmap>
#include <QApplication>
#include <QElapsedTimer>
#include "menuwindow.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    isRunning_(false),
    handDetected_(false),
    hasPrevFrame_(false),
    m_gameStarted(false)   // initialise à false

{
    ui->setupUi(this);
    ui->scene_->setMinimumSize(800, 600);
    connect(ui->scene_, &SceneGL::gameOver, this, &Widget::handleGameOver);
    ui->overlayWidget->setVisible(true);
    ui->overlayWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false); // Pour que les boutons reçoivent bien les clics
    ui->overlayWidget->setStyleSheet("background: transparent; border: none;");
    ui->scene_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->scene_->stackUnder(ui->overlayWidget);
    ui->overlayWidget->raise();

    // Webcam
    webCam_ = new cv::VideoCapture(0);
    if (!webCam_->isOpened()) {
        ui->label->setText("Error opening the default camera!");
    } else {
        int width = webCam_->get(cv::CAP_PROP_FRAME_WIDTH);
        int height = webCam_->get(cv::CAP_PROP_FRAME_HEIGHT);
        webCam_->set(cv::CAP_PROP_FPS, 30);
        ui->label->setText(QString("Camera ready, %1x%2").arg(width).arg(height));
    }

    // Charger le fichier cascade pour la main
    QString palmPath = QCoreApplication::applicationDirPath() + "/palm.xml";
    qDebug() << "Trying to load palm.xml from:" << palmPath;

    if (!handCascade_.load(palmPath.toStdString())) {
        QMessageBox::warning(this, "Cascade File Not Found", "Could not load palm.xml from: " + palmPath);
        ui->label->setText(ui->label->text() + " | Using alternative detection");
    } else {
        ui->label->setText(ui->label->text() + " | Hand detection ready");
    }

    initKalmanFilter();
    uiTimer_ = new QTimer(this);
    connect(uiTimer_, &QTimer::timeout, this, &Widget::updateUI);

    showMenu();

}



void Widget::showMenu()
{
    ui->scene_->hide();                 // Cache la scène
    ui->label->setText("Bienvenue ! Cliquez sur 'Démarrer le jeu'");
}

void Widget::startGame()
{
    ui->scene_->resetGame();     // ✅ assure un état propre
    ui->scene_->show();
    ui->scene_->startGame();
    m_gameStarted = true;
    m_gameTimer.start();
    PlaySound(TEXT("images/musique.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}


void Widget::on_startGameButton_clicked()
{
    startGame();
}


void Widget::stopGame()
{
    ui->scene_->stopGame();
    ui->scene_->resetGame();  // ✅ Ajoute cette ligne

    m_gameStarted = false;
    ui->scene_->hide();
    ui->label->setText("Jeu arrêté.");
}


void Widget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    ui->overlayWidget->setGeometry(this->rect()); // Adapter overlay à toute la fenêtre
    ui->scene_->setGeometry(this->rect());

}



void Widget::on_stopButton__clicked()
{
    if (m_gamePaused) {
        m_gamePaused = false;
        ui->scene_->resumeGame();
        ui->label->setText("Jeu repris");
        ui->stopButton_->setText("Stop");  // Texte bouton devient "Stop" quand jeu en cours
    } else {
        m_gamePaused = true;
        ui->scene_->stopGame();
        ui->label->setText("Jeu en pause");
        ui->stopButton_->setText("Start"); // Texte bouton devient "Start" quand jeu en pause
    }
}


void Widget::on_quitButton_clicked()
{
    // Demande confirmation avant de quitter (optionnel)
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Quitter", "Voulez-vous vraiment quitter le jeu ?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QApplication::quit();
    }
}




Widget::~Widget()
{
    isRunning_ = false;
    if (captureThread_.joinable()) captureThread_.join();
    delete ui;
    delete webCam_;
    delete uiTimer_;
}

void Widget::handleGameOver()
{
    isRunning_ = false;
    if (captureThread_.joinable()) captureThread_.join();
    uiTimer_->stop();
    
    // Récupérer le score et le temps avant de nettoyer la scène
    int finalScore = ui->scene_->getScore();
    double finalTime = ui->scene_->getGameTimeSeconds();
    
    stopGame();  // Nettoie la scène
    PlaySound(TEXT("images/mort.wav"), NULL, SND_FILENAME | SND_ASYNC);
    
    // Créer une MessageBox avec les deux options et afficher le score et le temps
    QMessageBox msgBox;
    msgBox.setWindowTitle("Game Over");
    msgBox.setText(QString("Tu as perdu !\n\nScore final : %1\nTemps de jeu : %2 s")
                   .arg(finalScore)
                   .arg(QString::number(finalTime, 'f', 1)));
    msgBox.setStandardButtons(QMessageBox::NoButton); // On n'utilise pas les boutons standard
    
    // Ajouter nos boutons personnalisés
    QPushButton* retryButton = msgBox.addButton("Réessayer", QMessageBox::AcceptRole);
    QPushButton* quitButton = msgBox.addButton("Quitter le jeu", QMessageBox::RejectRole);
    
    msgBox.exec(); // Afficher la boîte de dialogue
    
    if (msgBox.clickedButton() == quitButton) {
        QApplication::quit();
    } 
    else if (msgBox.clickedButton() == retryButton) {
        this->show();
        startGame();
        if (!isRunning_) {
            isRunning_ = true;
            captureThread_ = std::thread(&Widget::captureAndProcess, this);
            uiTimer_->start(30);
            ui->captureButton_->setText("Stop Tracking");
        }
    }
}


void Widget::on_captureButton__clicked()
{
    if (!isRunning_) {
        isRunning_ = true;
        captureThread_ = std::thread(&Widget::captureAndProcess, this);
        uiTimer_->start(30);
        ui->captureButton_->setText("Stop Tracking");
    } else {
        isRunning_ = false;
        if (captureThread_.joinable()) captureThread_.join();
        uiTimer_->stop();
        ui->captureButton_->setText("Start Tracking");
    }
}

void Widget::captureAndProcess()
{
    while (isRunning_) {
        cv::Mat frame;
        if (!webCam_->read(frame)) continue;
        cv::flip(frame, frame, 1);
        frame.copyTo(prevFrame_);
        hasPrevFrame_ = true;

        cv::Point localHandPosition;
        bool detected = detectHand(frame, localHandPosition);
        if (detected) {
            localHandPosition = predictKalmanFilter(localHandPosition);
            QMutexLocker locker(&handPositionMutex_);
            handPosition_ = localHandPosition;
            handDetected_ = true;
        } else {
            QMutexLocker locker(&handPositionMutex_);
            handDetected_ = false;
        }

        QMutexLocker locker(&frameMutex_);
        frame.copyTo(latestFrame_);
    }
}

void Widget::updateUI()
{
    cv::Mat frameCopy;
    {
        QMutexLocker locker(&frameMutex_);
        if (latestFrame_.empty()) return;
        latestFrame_.copyTo(frameCopy);
    }

    bool detected;
    cv::Point pos;
    {
        QMutexLocker locker2(&handPositionMutex_);
        detected = handDetected_;
        pos = handPosition_;
    }

    if (detected) {
        // DESSINER LE CERCLE D'ABORD sur BGR
        cv::circle(frameCopy, pos, 20, cv::Scalar(0, 255, 0), 2);  // Cercle vert

        QPoint qtPos(pos.x, pos.y);  // ✅ Correct
        ui->scene_->setMainVirtuellePos2D(qtPos);
        ui->label->setText(QString("Hand detected at (%1, %2)").arg(pos.x, pos.y));
    } else {
        ui->label->setText("No hand detected");
    }

    // PUIS convertir et afficher
    cv::cvtColor(frameCopy, frameCopy, cv::COLOR_BGR2RGB);
    QImage img((const unsigned char*)(frameCopy.data), frameCopy.cols, frameCopy.rows, frameCopy.step, QImage::Format_RGB888);
    ui->imageLabel_->setPixmap(QPixmap::fromImage(img).scaled(ui->imageLabel_->size(), Qt::KeepAspectRatio));
}


bool Widget::detectHand(const cv::Mat& image, cv::Point& handPosition)
{
    if (!handCascade_.empty()) {
        cv::Mat gray;
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);

        std::vector<cv::Rect> hands;
        handCascade_.detectMultiScale(gray, hands, 1.1, 3, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(90, 90));

        if (!hands.empty()) {
            cv::Rect bestHand = hands[0];
            handPosition.x = bestHand.x + bestHand.width / 2;
            handPosition.y = bestHand.y + bestHand.height / 2;
            return true;
        }
    }

    return detectHandByColor(image, handPosition) || detectHandByContour(image, handPosition);
}

bool Widget::detectHandByColor(const cv::Mat& image, cv::Point& handPosition)
{
    cv::Mat hsv, mask;
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(0, 20, 70), cv::Scalar(20, 255, 255), mask);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (!contours.empty()) {
        size_t maxIndex = 0;
        double maxArea = 0;
        for (size_t i = 0; i < contours.size(); i++) {
            double area = cv::contourArea(contours[i]);
            if (area > maxArea) {
                maxArea = area;
                maxIndex = i;
            }
        }

        if (maxArea > 5000) {
            cv::Moments m = cv::moments(contours[maxIndex]);
            handPosition.x = static_cast<int>(m.m10 / m.m00);
            handPosition.y = static_cast<int>(m.m01 / m.m00);
            return true;
        }
    }

    return false;
}

bool Widget::detectHandByContour(const cv::Mat& image, cv::Point& handPosition)
{
    cv::Mat gray, thresh;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, thresh, 60, 255, cv::THRESH_BINARY_INV);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (!contours.empty()) {
        size_t maxIndex = 0;
        double maxArea = 0;
        for (size_t i = 0; i < contours.size(); i++) {
            double area = cv::contourArea(contours[i]);
            if (area > maxArea) {
                maxArea = area;
                maxIndex = i;
            }
        }

        if (maxArea > 5000) {
            cv::Rect boundRect = cv::boundingRect(contours[maxIndex]);
            handPosition.x = boundRect.x + boundRect.width / 2;
            handPosition.y = boundRect.y + boundRect.height / 2;
            return true;
        }
    }

    return false;
}

void Widget::initKalmanFilter()
{
    KF_ = cv::KalmanFilter(4, 2, 0);
    measurement_ = cv::Mat_<float>(2, 1);
    cv::setIdentity(KF_.transitionMatrix);
    KF_.transitionMatrix.at<float>(0, 2) = 1;
    KF_.transitionMatrix.at<float>(1, 3) = 1;
    cv::setIdentity(KF_.measurementMatrix);
    cv::setIdentity(KF_.processNoiseCov, cv::Scalar::all(1e-4));
    cv::setIdentity(KF_.measurementNoiseCov, cv::Scalar::all(1e-1));
    cv::setIdentity(KF_.errorCovPost, cv::Scalar::all(1));
    KF_.statePost = (cv::Mat_<float>(4, 1) << 200, 200, 0, 0);
}

cv::Point Widget::predictKalmanFilter(const cv::Point& measurement)
{
    KF_.predict();
    measurement_.at<float>(0) = measurement.x;
    measurement_.at<float>(1) = measurement.y;
    cv::Mat estimated = KF_.correct(measurement_);
    return cv::Point(estimated.at<float>(0), estimated.at<float>(1));
}



