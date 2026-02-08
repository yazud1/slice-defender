#ifndef SCENEGL_H
#define SCENEGL_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QElapsedTimer>
#include <QTimer>
#include <QMouseEvent>
#include <QVector3D>
#include <vector>

#include "projectile.h"
#include "fragment.h"

class SceneGL : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit SceneGL(QWidget *parent = nullptr);
    ~SceneGL();

    void setMainVirtuellePos2D(const QPoint& pos);
    void startGame();
    void resetGame();  // Optionnel pour réinitialiser si tu veux un mode rejouable
    void stopGame();
    void resumeGame();
    double getGameTimeSeconds() const { return m_gameTimeSeconds; }
    int getScore() const { return m_score; }
    GLuint m_sabreTexture;
    void loadTexture();
    GLuint chargerTexture(const QString& path);
    void drawMinecraftGround();
    GLuint generateMinecraftTexture();
    void setGamePaused(bool paused);



protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mouseMoveEvent(QMouseEvent* event) override;

private slots:
    void updateScene();

signals:
    void gameOver();     // Signal quand les PV tombent à 0

private:
    void drawZoneDecoupe();
    bool m_gameStarted = false;
    void drawMainVirtuelle();
    void lancerProjectile();
    int m_score = 0;
    int m_pv = 3;
    bool m_gamePaused = false;
    qint64 m_elapsedPausedTime = 0;
    qint64 m_cumulatedTime = 0;
    void drawSol();
    void drawMurs();
    void drawSkybox(); 
    void drawPortailNether();   // NOUVELLE MÉTHODE
    void drawCube(float size);  // MÉTHODE UTILITAIRE
    GLuint m_textureSol;
    GLuint m_textureMur;
    QVector3D m_mainVirtuellePos3D;
    QVector3D m_mousePos3D;

    QTimer* m_timer;
    QElapsedTimer m_elapsed;
    float m_tempsDepuisDernierTir = 0.0f;

    QElapsedTimer m_gameTimer;
    int m_gameTimeSeconds = 0;

    std::vector<Projectile> m_projectiles;
    std::vector<Fragment> m_fragments;

    bool m_useMouseInput;  // 💥 Pour alterner entre souris et main virtuelle
    GLuint m_textureFireball = 0;
    GLuint m_textureFleche = 0;
    GLuint m_textureTNT = 0;
    GLuint m_textureCiel;
    GLuint m_textureObsidienne;  // NOUVELLE TEXTURE
    GLuint m_texturePortail;     // NOUVELLE TEXTURE

};

#endif // SCENEGL_H
