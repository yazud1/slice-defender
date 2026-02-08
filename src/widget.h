#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QMutex>
#include <atomic>
#include <thread>
#include <QElapsedTimer>
#include <opencv2/opencv.hpp>
#include "scenegl.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
    void startGame();   // Lance le jeu
    void resizeEvent(QResizeEvent *event);

private slots:
    void on_captureButton__clicked();       // Démarre ou arrête la détection
    //void on_startGameButton_clicked();      // Bouton pour démarrer le jeu
    void on_stopButton__clicked();          // Bouton pour arrêter le jeu
    void updateUI();                        // Met à jour l'affichage caméra + main
    void on_startGameButton_clicked();
    void on_quitButton_clicked();

    void handleGameOver();
private:
    Ui::Widget *ui;

    // Composants OpenGL
    SceneGL *scene_;

    // Capture vidéo
    cv::VideoCapture *webCam_;
    std::thread captureThread_;
    std::atomic<bool> isRunning_;
    QTimer *uiTimer_;

    // Détection de la main
    cv::CascadeClassifier handCascade_;
    cv::Point handPosition_;
    bool handDetected_;
    bool hasPrevFrame_;
    cv::Mat latestFrame_;
    cv::Mat prevFrame_;

    // Sécurité des threads
    QMutex frameMutex_;
    QMutex handPositionMutex_;

    // Kalman Filter
    cv::KalmanFilter KF_;
    cv::Mat_<float> measurement_;

    // Méthodes internes
    void captureAndProcess();               // Thread principal de capture
    bool detectHand(const cv::Mat &frame, cv::Point &handPosition);
    bool detectHandByColor(const cv::Mat &image, cv::Point &handPosition);
    bool detectHandByContour(const cv::Mat &image, cv::Point &handPosition);
    void initKalmanFilter();
    cv::Point predictKalmanFilter(const cv::Point &measurement);

    // 🔹 Gestion du jeu
    void showMenu();    // Affiche le menu
    void stopGame();    // Stoppe le jeu
    bool m_gameStarted; // Indique si une partie est en cours
    bool m_gamePaused = false;


    QElapsedTimer m_gameTimer;
};

#endif // WIDGET_H
