#define NOMINMAX
#include <windows.h>
#include "scenegl.h"
#include <GL/glu.h>
#include <QTimer>
#include <QDebug>
#include <QPainter>
#include <QFont>


SceneGL::SceneGL(QWidget *parent)
    : QOpenGLWidget(parent), m_useMouseInput(false)

{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    m_gameStarted = false;
    m_score = 0;
    m_pv = 3;
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SceneGL::updateScene);
    m_timer->start(16); // ~60 FPS
    QString path = QCoreApplication::applicationDirPath() + "/images/sabre_mc.png";
    QPixmap sabrePixmap(path);
    if (sabrePixmap.isNull()) {
        qDebug() << "❌ Curseur : image introuvable à" << path;
    } else {
        QCursor sabreCursor(sabrePixmap.scaled(64, 64), 0, 0);

    }





    m_elapsed.start();
}

void SceneGL::setGamePaused(bool paused) {
    m_gamePaused = paused;
}

SceneGL::~SceneGL() {}

void SceneGL::setMainVirtuellePos2D(const QPoint& pos)
{
    const int widthWebcam = 640;
    const int heightWebcam = 480;

    float xRatio = float(pos.x()) / float(widthWebcam);
    float yRatio = float(pos.y()) / float(heightWebcam);

    // Range [-scale, +scale]
    float scaleX = 5.0f;
    float scaleY = 5.0f;

    float x = (xRatio - 0.5f) * 2.0f * scaleX;
    float y = (0.5f - yRatio) * 2.0f * scaleY;

    m_mainVirtuellePos3D = QVector3D(x, y, 0.0f);
    update();
}




void SceneGL::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    loadTexture(); // pour le sabre

    // Charger toutes les textures
    m_textureFireball = chargerTexture(QCoreApplication::applicationDirPath() + "/images/fireball.png");
    m_textureFleche = chargerTexture(QCoreApplication::applicationDirPath() + "/images/fleche.png");
    m_textureTNT = chargerTexture(QCoreApplication::applicationDirPath() + "/images/tnt.png");
    
    // Textures pour l'environnement
    m_textureSol = chargerTexture(QCoreApplication::applicationDirPath() + "/images/grass_block_top.png");
    m_textureMur = chargerTexture(QCoreApplication::applicationDirPath() + "/images/stone_bricks.jpg");
    
    // NOUVELLE TEXTURE pour le ciel
    m_textureCiel = chargerTexture(QCoreApplication::applicationDirPath() + "/images/minecraft_sky.jpg");

    // NOUVELLES TEXTURES pour le portail du Nether
    m_textureObsidienne = chargerTexture(QCoreApplication::applicationDirPath() + "/images/obsidienne.jpg");
    m_texturePortail = chargerTexture(QCoreApplication::applicationDirPath() + "/images/portail.jpg");
    
    
    qDebug() << "Fireball:" << m_textureFireball;
    qDebug() << "Fleche:" << m_textureFleche;
    qDebug() << "TNT:" << m_textureTNT;
    qDebug() << "Sol:" << m_textureSol;
    qDebug() << "Mur:" << m_textureMur;
    qDebug() << "Ciel:" << m_textureCiel;
    qDebug() << "Obsidienne:" << m_textureObsidienne;
    qDebug() << "Portail:" << m_texturePortail;

    glClearColor(0.5f, 0.8f, 1.0f, 1.0f); // Bleu ciel par défaut au cas où

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat lightPos[] = { 5.0f, 5.0f, 5.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glEnable(GL_COLOR_MATERIAL);
}

void SceneGL::drawPortailNether()
{
    glPushMatrix();
    
    // Position du portail : décalé vers la DROITE et bien positionné
    glTranslatef(0.5f, -1.0f, -15.0f); // CORRIGÉ : décalé vers la droite (+1.5) au lieu de gauche (-3)
    
    // === CADRE EN OBSIDIENNE AVEC COINS (RECTANGULAIRE VERTICAL) ===
    if (m_textureObsidienne != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_textureObsidienne);
        glColor3f(0.2f, 0.1f, 0.3f); // Couleur sombre pour l'obsidienne
        
        // === RANGÉE INFÉRIEURE (Y = -2) - 4 BLOCS AVEC COINS ===
        // Coin inférieur gauche
        glPushMatrix();
        glTranslatef(-2.0f, -2.0f, 0.0f);
        drawCube(1.0f);
        glPopMatrix();
        
        // Blocs inférieur centre
        glPushMatrix();
        glTranslatef(-1.0f, -2.0f, 0.0f);
        drawCube(1.0f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(0.0f, -2.0f, 0.0f);
        drawCube(1.0f);
        glPopMatrix();
        
        // Coin inférieur droit
        glPushMatrix();
        glTranslatef(1.0f, -2.0f, 0.0f);
        drawCube(1.0f);
        glPopMatrix();
        
        // === COLONNES VERTICALES (Y = -1, 0, 1) - 3 BLOCS CHAQUE CÔTÉ ===
        for (int y = -1; y <= 1; y++) {
            // Colonne gauche
            glPushMatrix();
            glTranslatef(-2.0f, (float)y, 0.0f);
            drawCube(1.0f);
            glPopMatrix();
            
            // Colonne droite
            glPushMatrix();
            glTranslatef(1.0f, (float)y, 0.0f);
            drawCube(1.0f);
            glPopMatrix();
        }
        
        // === RANGÉE SUPÉRIEURE (Y = 2) - 4 BLOCS AVEC COINS ===
        // Coin supérieur gauche
        glPushMatrix();
        glTranslatef(-2.0f, 2.0f, 0.0f);
        drawCube(1.0f);
        glPopMatrix();
        
        // Blocs supérieur centre
        glPushMatrix();
        glTranslatef(-1.0f, 2.0f, 0.0f);
        drawCube(1.0f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(0.0f, 2.0f, 0.0f);
        drawCube(1.0f);
        glPopMatrix();
        
        // Coin supérieur droit
        glPushMatrix();
        glTranslatef(1.0f, 2.0f, 0.0f);
        drawCube(1.0f);
        glPopMatrix();
        
        glDisable(GL_TEXTURE_2D);
    }
    
    // === EFFET DU PORTAIL AU CENTRE (RECTANGULAIRE VERTICAL) ===
    if (m_texturePortail != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_texturePortail);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Effet d'animation
        static float animationTime = 0.0f;
        animationTime += 0.01f;
        
        glColor4f(0.8f, 0.3f, 1.0f, 0.8f); // Couleur violette translucide
        
        // CARRELAGE DU PORTAIL RECTANGULAIRE (2x3 dalles)
        int tilesX = 2; // 2 dalles en largeur
        int tilesY = 3; // 3 dalles en hauteur
        float tileWidth = 1.0f;  // Largeur complète d'une dalle
        float tileHeight = 1.0f; // Hauteur d'une dalle
        
        // Position de départ corrigée et décalée vers la gauche
        float startX = -1.5f;  // Position relative dans le portail (reste inchangée)
        float startY = -1.5f;  // CORRIGÉ : commence à -1.5 pour centrer
        
        for (int x = 0; x < tilesX; ++x) {
            for (int y = 0; y < tilesY; ++y) {
                float xPos = startX + x * tileWidth;
                float yPos = startY + y * tileHeight;
                
                glBegin(GL_QUADS);
                // Animation de texture pour chaque dalle
                float animOffset = sin(animationTime + x * 0.3f + y * 0.2f) * 0.1f;
                
                glTexCoord2f(0.0f + animOffset, 0.0f + animOffset); 
                glVertex3f(xPos, yPos, 0.1f);
                
                glTexCoord2f(1.0f + animOffset, 0.0f + animOffset); 
                glVertex3f(xPos + tileWidth, yPos, 0.1f);
                
                glTexCoord2f(1.0f + animOffset, 1.0f + animOffset); 
                glVertex3f(xPos + tileWidth, yPos + tileHeight, 0.1f);
                
                glTexCoord2f(0.0f + animOffset, 1.0f + animOffset); 
                glVertex3f(xPos, yPos + tileHeight, 0.1f);
                glEnd();
            }
        }
        
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
    }
    
    glPopMatrix();
}

void SceneGL::drawCube(float size)
{
    float s = size / 2.0f;
    
    // Face AVANT
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-s, -s,  s);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( s, -s,  s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( s,  s,  s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-s,  s,  s);
    glEnd();
    
    // Face ARRIÈRE
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-s, -s, -s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-s,  s, -s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( s,  s, -s);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( s, -s, -s);
    glEnd();
    
    // Face HAUT
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-s,  s, -s);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-s,  s,  s);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( s,  s,  s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( s,  s, -s);
    glEnd();
    
    // Face BAS
    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-s, -s, -s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( s, -s, -s);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( s, -s,  s);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-s, -s,  s);
    glEnd();
    
    // Face DROITE
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( s, -s, -s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( s,  s, -s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( s,  s,  s);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( s, -s,  s);
    glEnd();
    
    // Face GAUCHE
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-s, -s, -s);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-s, -s,  s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-s,  s,  s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-s,  s, -s);
    glEnd();
}
void SceneGL::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, GLfloat(w) / GLfloat(h), 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void SceneGL::drawSkybox()
{
    if (m_textureCiel == 0) return; // Pas de texture disponible
    
    // Sauvegarder les états OpenGL
    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Désactiver l'écriture dans le depth buffer pour la skybox
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING); // Pas d'éclairage pour le ciel
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureCiel);
    glColor3f(1.0f, 1.0f, 1.0f); // Couleur blanche pour ne pas altérer la texture
    
    float skySize = 50.0f; // Taille de la skybox (très grande)
    
    // Face ARRIÈRE (derrière nous)
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-skySize, -skySize, -skySize);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( skySize, -skySize, -skySize);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( skySize,  skySize, -skySize);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-skySize,  skySize, -skySize);
    glEnd();
    
    // Face AVANT (devant nous) - plus loin que les projectiles
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-skySize, -skySize, skySize);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( skySize, -skySize, skySize);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( skySize,  skySize, skySize);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-skySize,  skySize, skySize);
    glEnd();
    
    // Face GAUCHE
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-skySize, -skySize, -skySize);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-skySize, -skySize,  skySize);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-skySize,  skySize,  skySize);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-skySize,  skySize, -skySize);
    glEnd();
    
    // Face DROITE
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(skySize, -skySize, -skySize);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(skySize, -skySize,  skySize);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(skySize,  skySize,  skySize);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(skySize,  skySize, -skySize);
    glEnd();
    
    // Face HAUT (plafond)
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-skySize, skySize, -skySize);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-skySize, skySize,  skySize);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( skySize, skySize,  skySize);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( skySize, skySize, -skySize);
    glEnd();
    
    // Face BAS (pas nécessaire car on a le sol)
    
    glDisable(GL_TEXTURE_2D);
    
    // Restaurer les états OpenGL
    glPopAttrib();
}

void SceneGL::drawSol()
{
    if (m_textureSol == 0) return; // Pas de texture disponible
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureSol);
    glColor3f(1.0f, 1.0f, 1.0f); // Couleur blanche pour ne pas altérer la texture
    
    // Sol large centré sur l'origine
    float tileSize = 2.0f;   // Taille d'une dalle
    int tilesX = 15;         // 15 dalles en largeur
    int tilesZ = 20;         // 20 dalles en profondeur
    
    float startX = -(tilesX * tileSize) / 2.0f;
    float startZ = -(tilesZ * tileSize) / 2.0f;
    
    for (int x = 0; x < tilesX; ++x) {
        for (int z = 0; z < tilesZ; ++z) {
            float xPos = startX + x * tileSize;
            float zPos = startZ + z * tileSize;
            
            glBegin(GL_QUADS);
            glNormal3f(0.0f, 1.0f, 0.0f); // Normale vers le haut
            glTexCoord2f(0.0f, 0.0f); glVertex3f(xPos, -3.0f, zPos);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(xPos + tileSize, -3.0f, zPos);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(xPos + tileSize, -3.0f, zPos + tileSize);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(xPos, -3.0f, zPos + tileSize);
            glEnd();
        }
    }
    
    glDisable(GL_TEXTURE_2D);
}

void SceneGL::drawMurs()
{
    if (m_textureMur == 0) return; // Pas de texture disponible
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureMur);
    glColor3f(0.8f, 0.8f, 0.8f); // Légèrement assombri pour différencier du sol
    
    float murHauteur = 8.0f;
    float murLargeur = 12.0f;    // ÉTENDU : correspond à la largeur du sol (15 dalles x 2)
    float murProfondeur = 40.0f; // ÉTENDU : correspond à la profondeur du sol (20 dalles x 2)
    float textureRepeat = 8.0f;  // Plus de répétitions pour les murs plus grands
    
    // MUR GAUCHE - ÉTENDU JUSQU'AU BOUT DU SOL
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f); // Normale vers la droite
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-murLargeur/2, -3.0f, -murProfondeur/2);
    glTexCoord2f(textureRepeat, 0.0f); glVertex3f(-murLargeur/2, -3.0f, murProfondeur/2);
    glTexCoord2f(textureRepeat, textureRepeat); glVertex3f(-murLargeur/2, murHauteur, murProfondeur/2);
    glTexCoord2f(0.0f, textureRepeat); glVertex3f(-murLargeur/2, murHauteur, -murProfondeur/2);
    glEnd();
    
    // MUR DROIT - ÉTENDU JUSQU'AU BOUT DU SOL
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f); // Normale vers la gauche
    glTexCoord2f(0.0f, 0.0f); glVertex3f(murLargeur/2, -3.0f, murProfondeur/2);
    glTexCoord2f(textureRepeat, 0.0f); glVertex3f(murLargeur/2, -3.0f, -murProfondeur/2);
    glTexCoord2f(textureRepeat, textureRepeat); glVertex3f(murLargeur/2, murHauteur, -murProfondeur/2);
    glTexCoord2f(0.0f, textureRepeat); glVertex3f(murLargeur/2, murHauteur, murProfondeur/2);
    glEnd();
    
    // SUPPRESSION DU MUR ARRIÈRE - plus de mur devant nous !
    
    glDisable(GL_TEXTURE_2D);
}

void SceneGL::paintGL()
{
    glDisable(GL_CULL_FACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(0.0, 0.0, 8.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // DESSINER L'ENVIRONNEMENT EN PREMIER
    drawSkybox();
    drawSol();
    drawMurs();
    
    // DESSINER LE PORTAIL DU NETHER (source des projectiles)
    drawPortailNether();
    
    // Puis le reste de la scène
    drawZoneDecoupe();
    drawMainVirtuelle();

    for (const auto& p : m_projectiles)
        p.draw();

    for (const auto& f : m_fragments)
        f.draw();
    
    // Interface utilisateur
    QString scoreText = QString("Score: %1").arg(m_score);
    QString pvText = QString("PV: %1").arg(m_pv);
    QString timeText = QString("Temps: %1s").arg(m_gameTimeSeconds);

    QPainter painter(this);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 14, QFont::Bold));
    painter.drawText(width() - 150, 30, scoreText);
    painter.drawText(width() - 150, 60, pvText);
    painter.drawText(width() - 150, 90, timeText);
    painter.end();
}


void SceneGL::drawZoneDecoupe()
{
    glPushMatrix();
    
    glTranslatef(0.0f, 0.0f, 2.0f); // Cylindre plus proche de l'origine (au lieu de 8.0f)
    
    float cylinderRadius = 4.5f;  // Rayon un peu plus petit aussi
    float cylinderHeight = 6.0f;  // Hauteur ajustée
    int segments = 24;             
    int rings = 8;                 
    
    glColor4f(0.0f, 1.0f, 0.0f, 0.5f); // Un peu plus transparent
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Désactiver le culling pour voir les faces de l'intérieur
    glDisable(GL_CULL_FACE);
    
    // Dessiner le quadrillage cylindrique en mode wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.0f); // Lignes un peu moins épaisses
    
    // Lignes verticales (génératrices du cylindre)
    glBegin(GL_LINES);
    for (int i = 0; i < segments; i++) {
        float angle = (float)i * 2.0f * M_PI / segments;
        float x = cylinderRadius * cos(angle);
        float z = cylinderRadius * sin(angle);
        
        glVertex3f(x, -cylinderHeight/2, z);
        glVertex3f(x, cylinderHeight/2, z);
    }
    glEnd();
    
    // Lignes horizontales (cercles)
    for (int ring = 0; ring <= rings; ring++) {
        float y = -cylinderHeight/2 + (float)ring * cylinderHeight / rings;
        
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; i++) {
            float angle = (float)i * 2.0f * M_PI / segments;
            float x = cylinderRadius * cos(angle);
            float z = cylinderRadius * sin(angle);
            glVertex3f(x, y, z);
        }
        glEnd();
    }
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glPopMatrix();
}


void SceneGL::drawMainVirtuelle()
{
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, m_sabreTexture);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    float w = 0.5f, h = 1.5f;
    glPushMatrix();
    glTranslatef(m_mainVirtuellePos3D.x(), m_mainVirtuellePos3D.y(), m_mainVirtuellePos3D.z());
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-w / 2, -h / 2, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( w / 2, -h / 2, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( w / 2,  h / 2, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-w / 2,  h / 2, 0.0f);
    glEnd();
    glPopMatrix();

    // --- RESTAURE BIEN L'ÉTAT ---
    glDisable(GL_TEXTURE_2D);     // Très important
    glDisable(GL_BLEND);          // (facultatif mais propre)
    glEnable(GL_LIGHTING);        // Pour que l'éclairage OpenGL fonctionne après
    // ----------------------------

}



GLuint SceneGL::chargerTexture(const QString& path)
{
    QImage img(path);
    if (img.isNull()) {
        qDebug() << "Erreur chargement image:" << path;
        return 0;
    }
    if (img.isNull()) return 0;
    QImage tex = img.convertToFormat(QImage::Format_RGBA8888).mirrored();
    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width(), tex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());
    return texId;

}


void SceneGL::lancerProjectile()
{
    // Position de départ avec dispersion dans l'ouverture
    float offsetX = ((rand() % 200) - 100) / 100.0f; // -1.0 à +1.0
    float offsetY = ((rand() % 300) - 150) / 100.0f; // -1.5 à +1.5
    QVector3D depart(0.0f + offsetX * 0.8f, -1.0f + offsetY * 0.8f, -14.9f);

    // Direction vers le joueur avec dispersion
    QVector3D directionVersJoueur = QVector3D(0.0f, 0.0f, 8.0f) - depart;
    directionVersJoueur.normalize();
    
    // Vitesse avec trajectoire parabolique et dispersion
    float vx = directionVersJoueur.x() * 8.0f + ((rand() % 3900)-800) / 1000.0f;
    float vy = 7.0f + (rand() % 120) / 100.0f; // Trajectoire parabolique
    float vz = directionVersJoueur.z() * 7.7f + ((rand() % 1000)-500) / 1000.0f;

    QVector3D vitesse(vx, vy, vz);

    ProjectileType type = static_cast<ProjectileType>(rand() % 4);
    GLuint texId = 0;
    switch(type) {
    case ProjectileType::Sphere:   texId = m_textureFireball; break;
    case ProjectileType::Cone:     
    case ProjectileType::Cylindre: texId = m_textureFleche; break;
    case ProjectileType::Cube:     texId = m_textureTNT; break;
    }
    m_projectiles.emplace_back(depart, vitesse, type, texId);
}  


void SceneGL::updateScene()
{
    // Calcul du temps de jeu en secondes selon état pause ou non
    m_gameTimeSeconds = (m_gamePaused ? m_elapsedPausedTime : (m_elapsedPausedTime + m_gameTimer.elapsed())) / 1000.0;

    if (!m_gameStarted) return;
    if (m_gamePaused) return;  // Stoppe toute mise à jour en pause

    // Calcul du delta temps (en secondes) entre deux frames
    float delta = m_elapsed.elapsed() / 1000.0f;
    m_elapsed.restart();

    // Mise à jour des fragments
    for (auto it = m_fragments.begin(); it != m_fragments.end(); ) {
        it->update(delta);
        if (!it->isAlive())
            it = m_fragments.erase(it);
        else
            ++it;
    }

    // Mise à jour des projectiles
    for (auto& p : m_projectiles)
        p.update(delta);

    m_tempsDepuisDernierTir += delta;
    if (m_tempsDepuisDernierTir >= 4.0f) {
        lancerProjectile();
        m_tempsDepuisDernierTir = 0.0f;
    }

    // Vérification des collisions
    for (auto it = m_projectiles.begin(); it != m_projectiles.end(); ) {
        QVector3D pos = it->getPosition();
        float dx = pos.x() - m_mainVirtuellePos3D.x();
        float dy = pos.y() - m_mainVirtuellePos3D.y();
        float distance = std::sqrt(dx * dx + dy * dy);

        if (fabs(pos.z()) < 2.8f) {
            if (distance < (0.3f + 2.0f)) {
                // === COLLISION DÉTECTÉE - GÉNÉRATION DE FRAGMENTS AVEC TEXTURE ===
                m_score += 10;

                QVector3D collisionPoint = pos;
                ProjectileType fragType = it->getType();
                GLuint fragTexture = 0;

                // Récupérer la texture correspondante au type
                switch(fragType) {
                case ProjectileType::Sphere:   fragTexture = m_textureFireball; break;
                case ProjectileType::Cone:     
                case ProjectileType::Cylindre: fragTexture = m_textureFleche; break;
                case ProjectileType::Cube:     fragTexture = m_textureTNT; break;
                }

                // Créer 4-6 fragments avec la bonne texture et taille appropriée
                for (int i = 0; i < 5; ++i) {
                    QVector3D fragVelocity(
                        ((rand() % 200) - 100) / 50.0f,  // -2 à +2
                        ((rand() % 150) + 50) / 50.0f,   // +1 à +4 (vers le haut)
                        ((rand() % 200) - 100) / 50.0f   // -2 à +2
                    );
                    
                    float fragRotSpeed = ((rand() % 400) + 100) / 10.0f; // 10-50 degrés/s
                    
                    // IMPORTANT : passer la texture au fragment
                    m_fragments.emplace_back(collisionPoint, fragVelocity, fragRotSpeed, fragType, fragTexture);
                }

                it = m_projectiles.erase(it);
                continue;
            } else if (distance > 1.5f && pos.z() > -0.5f) {
                m_pv -= 1;
                if (m_pv <= 0 && m_gameStarted) {
                    m_gameStarted = false;
                    emit gameOver();
                    return;
                }
                it = m_projectiles.erase(it);
                continue;
            }
        }

        ++it;
    }

    update();
}





void SceneGL::startGame()
{
    m_gameStarted = true;
    m_projectiles.clear();
    m_fragments.clear();
    m_tempsDepuisDernierTir = 0.0f;
    m_score = 0;
    m_pv = 3;

    m_elapsedPausedTime = 0;
    m_gamePaused = false;
    m_gameTimer.restart();
    m_elapsed.restart();
}





void SceneGL::resetGame()
{
    m_projectiles.clear();
    m_fragments.clear();
    m_tempsDepuisDernierTir = 0.0f;
    m_score = 0;
    m_pv = 3;
    update();
}

void SceneGL::stopGame()
{
    m_gamePaused = true;
    m_elapsedPausedTime += m_gameTimer.elapsed(); // Sauvegarde temps écoulé
}

void SceneGL::resumeGame()
{
    m_gamePaused = false;
    m_gameTimer.restart();       // Redémarre le chrono à 0
    m_elapsed.restart();         // Redémarre chrono delta pour update fluide
    // Le temps affiché sera calculé dans updateScene en ajoutant m_elapsedPausedTime + m_gameTimer.elapsed()
}




void SceneGL::loadTexture()
{
    QString path = QCoreApplication::applicationDirPath() + "/images/sabre_mc.png";
    QImage img(path);

    if (img.isNull()) {
        qDebug() << "❌ Image introuvable au chemin :" << path;
        return;
    }

    // Toujours convertir en RGBA (prend en charge l'alpha)
    QImage tex = img.convertToFormat(QImage::Format_RGBA8888).mirrored();

    // Sécurité supplémentaire : si certains pixels sont encore blancs, rends-les transparents
    for (int y = 0; y < tex.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(tex.scanLine(y));
        for (int x = 0; x < tex.width(); ++x) {
            QColor pixelColor(line[x]);
            if (pixelColor.red() > 245 && pixelColor.green() > 245 && pixelColor.blue() > 245 && pixelColor.alpha() > 200) {
                // Pixel quasiment blanc : rendre transparent
                line[x] = qRgba(255, 255, 255, 0);
            }
        }
    }

    glGenTextures(1, &m_sabreTexture);
    glBindTexture(GL_TEXTURE_2D, m_sabreTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width(), tex.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());

    qDebug() << "✅ Texture OpenGL chargée : " << tex.width() << "x" << tex.height();
}



void SceneGL::mouseMoveEvent(QMouseEvent* event)
{
}
