#include "fragment.h"
#include <GL/glu.h>

Fragment::Fragment(const QVector3D& position, const QVector3D& velocity, float rotationSpeed, ProjectileType type, GLuint textureId)
    : m_position(position), m_velocity(velocity), m_rotationSpeed(rotationSpeed), m_type(type), m_textureId(textureId)
{
    m_rotation = 0.0f;
    m_lifetime = 3.0f; // 3 secondes de vie
}

void Fragment::update(float deltaTime)
{
    // Gravité plus forte pour les fragments
    m_velocity.setY(m_velocity.y() - 15.0f * deltaTime);
    m_position += m_velocity * deltaTime;
    
    // Rotation
    m_rotation += m_rotationSpeed * deltaTime;
    
    // Diminuer la durée de vie
    m_lifetime -= deltaTime;
}

bool Fragment::isAlive() const
{
    return m_lifetime > 0.0f && m_position.y() > -5.0f; // Disparaît si trop bas
}

void Fragment::draw() const
{
    glPushMatrix();
    glTranslatef(m_position.x(), m_position.y(), m_position.z());
    
    // TAILLE : moitié de la taille originale (0.5x)
    glScalef(0.5f, 0.5f, 0.5f);
    
    glRotatef(m_rotation, 1.0f, 1.0f, 0.0f); // Rotation aléatoire
    
    // Appliquer la texture si disponible
    if (m_textureId != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    
    // Dessiner selon le type
    switch(m_type) {
    case ProjectileType::Cube:
        drawCubeFragment();
        break;
    case ProjectileType::Sphere:
        drawSphereFragment();
        break;
    case ProjectileType::Cone:
        drawConeFragment();
        break;
    case ProjectileType::Cylindre:
        drawCylinderFragment();
        break;
    }
    
    if (m_textureId != 0) {
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
    }
    
    glPopMatrix();
}

void Fragment::drawCubeFragment() const
{
    float s = 0.28f; // AUGMENTÉ : moitié de la taille du projectile original (était 0.14f)
    
    glBegin(GL_QUADS);
    
    // Face avant
    glTexCoord2f(0,0); glVertex3f(-s, -s,  s);
    glTexCoord2f(1,0); glVertex3f( s, -s,  s);
    glTexCoord2f(1,1); glVertex3f( s,  s,  s);
    glTexCoord2f(0,1); glVertex3f(-s,  s,  s);

    // Face arrière
    glTexCoord2f(1,0); glVertex3f(-s, -s, -s);
    glTexCoord2f(1,1); glVertex3f(-s,  s, -s);
    glTexCoord2f(0,1); glVertex3f( s,  s, -s);
    glTexCoord2f(0,0); glVertex3f( s, -s, -s);

    // Face gauche
    glTexCoord2f(0,0); glVertex3f(-s, -s, -s);
    glTexCoord2f(1,0); glVertex3f(-s, -s,  s);
    glTexCoord2f(1,1); glVertex3f(-s,  s,  s);
    glTexCoord2f(0,1); glVertex3f(-s,  s, -s);

    // Face droite
    glTexCoord2f(0,0); glVertex3f( s, -s, -s);
    glTexCoord2f(1,0); glVertex3f( s,  s, -s);
    glTexCoord2f(1,1); glVertex3f( s,  s,  s);
    glTexCoord2f(0,1); glVertex3f( s, -s,  s);

    // Face haut
    glTexCoord2f(0,0); glVertex3f(-s,  s, -s);
    glTexCoord2f(1,0); glVertex3f(-s,  s,  s);
    glTexCoord2f(1,1); glVertex3f( s,  s,  s);
    glTexCoord2f(0,1); glVertex3f( s,  s, -s);

    // Face bas
    glTexCoord2f(0,0); glVertex3f(-s, -s, -s);
    glTexCoord2f(1,0); glVertex3f( s, -s, -s);
    glTexCoord2f(1,1); glVertex3f( s, -s,  s);
    glTexCoord2f(0,1); glVertex3f(-s, -s,  s);
    
    glEnd();
}

void Fragment::drawSphereFragment() const
{
    // Petite sphère avec texture
    GLUquadric* quadric = gluNewQuadric();
    if (m_textureId != 0) {
        gluQuadricTexture(quadric, GL_TRUE);
        gluQuadricNormals(quadric, GLU_SMOOTH);
    }
    gluSphere(quadric, 0.2f, 12, 12); // AUGMENTÉ : rayon 0.2f (moitié de 0.4f)
    gluDeleteQuadric(quadric);
}

void Fragment::drawConeFragment() const
{
    // Petit cône avec texture
    GLUquadric* quadric = gluNewQuadric();
    if (m_textureId != 0) {
        gluQuadricTexture(quadric, GL_TRUE);
        gluQuadricNormals(quadric, GLU_SMOOTH);
    }
    gluCylinder(quadric, 0.05f, 0.0f, 0.2f, 8, 4); // AUGMENTÉ : moitié de la taille
    gluDeleteQuadric(quadric);
}

void Fragment::drawCylinderFragment() const
{
    // Petit cylindre avec texture
    GLUquadric* quadric = gluNewQuadric();
    if (m_textureId != 0) {
        gluQuadricTexture(quadric, GL_TRUE);
        gluQuadricNormals(quadric, GLU_SMOOTH);
    }
    gluCylinder(quadric, 0.04f, 0.04f, 0.2f, 8, 4); // AUGMENTÉ : moitié de la taille
    gluDeleteQuadric(quadric);
}