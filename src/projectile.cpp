#include "projectile.h"
#include <GL/glu.h>


Projectile::~Projectile() {}

void Projectile::update(float deltaTime)
{

    // Gravité
    m_velocity.setY(m_velocity.y() - 9.81f * deltaTime);
    m_position += m_velocity * deltaTime;

    // Incrémente la rotation
    m_rotationAngle += 180.0f * deltaTime; // 180°/s (ajuste si tu veux)
    if (m_rotationAngle > 360.0f) m_rotationAngle -= 360.0f;
}

Projectile::Projectile(const QVector3D& position, const QVector3D& velocity, ProjectileType type, GLuint textureId)
    : m_position(position), m_velocity(velocity), m_type(type), m_textureId(textureId)
{
    m_rotationAxis = QVector3D(
    (float)(rand() % 100) / 100.0f,
    (float)(rand() % 100) / 100.0f,
    (float)(rand() % 100) / 100.0f
).normalized();
}

ProjectileType Projectile::getType() const
{
    return m_type;
}
void Projectile::drawCube() const
{
    glDisable(GL_CULL_FACE); // Désactive le masquage des faces, cube toujours plein
    float s = 0.28f; // Taille du cube (ajuste à ton goût)
    glColor3f(1.0f, 0.3f, 0.3f); // Rouge, par défaut

    glBegin(GL_QUADS);

    // Face avant
    glVertex3f(-s, -s,  s);
    glVertex3f( s, -s,  s);
    glVertex3f( s,  s,  s);
    glVertex3f(-s,  s,  s);

    // Face arrière
    glVertex3f(-s, -s, -s);
    glVertex3f(-s,  s, -s);
    glVertex3f( s,  s, -s);
    glVertex3f( s, -s, -s);

    // Face gauche
    glVertex3f(-s, -s, -s);
    glVertex3f(-s, -s,  s);
    glVertex3f(-s,  s,  s);
    glVertex3f(-s,  s, -s);

    // Face droite
    glVertex3f( s, -s, -s);
    glVertex3f( s,  s, -s);
    glVertex3f( s,  s,  s);
    glVertex3f( s, -s,  s);

    // Face haut
    glVertex3f(-s,  s, -s);
    glVertex3f(-s,  s,  s);
    glVertex3f( s,  s,  s);
    glVertex3f( s,  s, -s);

    // Face bas
    glVertex3f(-s, -s, -s);
    glVertex3f( s, -s, -s);
    glVertex3f( s, -s,  s);
    glVertex3f(-s, -s,  s);

    glEnd();
}


void Projectile::draw() const
{
    glPushMatrix();
    glTranslatef(m_position.x(), m_position.y(), m_position.z());

    // Mise à l'échelle dynamique basée sur la profondeur
    float scale = std::clamp(6.0f / std::abs(m_position.z() + 0.1f), 0.8f, 4.0f);
    glScalef(scale, scale, scale);

    glRotatef(m_rotationAngle, m_rotationAxis.x(), m_rotationAxis.y(), m_rotationAxis.z());

    // Affichage selon le type
    switch (m_type)
    {
case ProjectileType::Sphere:
    if (m_textureId != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor3f(1.0f, 1.0f, 1.0f);
        glDisable(GL_CULL_FACE);

        // IMPORTANT : Activer la répétition de texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        float radius = 0.4f;
        int stacks = 30;
        int slices = 42;

        // --- Sphère avec texture répétée CORRECTEMENT ---
        for (int i = 0; i < stacks; ++i) {
            float phi1 = M_PI * ((float)i / stacks);
            float phi2 = M_PI * ((float)(i + 1) / stacks);

            glBegin(GL_QUAD_STRIP);
            for (int j = 0; j <= slices; ++j) {
                float theta = 2.0f * M_PI * ((float)j / slices);

                float x1 = radius * sin(phi1) * cos(theta);
                float y1 = radius * cos(phi1);
                float z1 = radius * sin(phi1) * sin(theta);

                float x2 = radius * sin(phi2) * cos(theta);
                float y2 = radius * cos(phi2);
                float z2 = radius * sin(phi2) * sin(theta);

                // Texture coordinates (note: u va de 0 à 1 sur un tour, v de 0 à 1 du bas vers le haut)
                float repeatU = 2.0f;
                float repeatV = 1.0f;
                float u = repeatU * (float)j / slices;
                float v1 = repeatV * (float)i / stacks;
                float v2 = repeatV * (float)(i + 1) / stacks;


                glTexCoord2f(u, v1); glVertex3f(x1, y1, z1);
                glTexCoord2f(u, v2); glVertex3f(x2, y2, z2);
            }
            glEnd();

        }

        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
    } else {
        // Fallback: sphère jaune
        GLUquadric* quad = gluNewQuadric();
        glColor3f(1.0f, 1.0f, 0.0f);
        gluSphere(quad, 0.4f, 16, 16);
        gluDeleteQuadric(quad);
    }
    break;
    case ProjectileType::Cone:   // --- FLECHE (BILLBOARD) ---
    case ProjectileType::Cylindre:
        if (m_textureId != 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, m_textureId);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // --- Ajoute CETTE PARTIE avant glBegin(GL_QUADS) ---
            QVector3D dir = m_velocity.normalized();
            float angle = atan2(dir.y(), dir.x()) * 180.0f / M_PI;
            glRotatef(angle, 0.0f, 0.0f, 1.0f);
            // ----------------------------------------------------

            float size = 0.3f;
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -size, 0.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex3f( size, -size, 0.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex3f( size,  size, 0.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(-size,  size, 0.0f);
            glEnd();

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
        } else {
            // Fallback: cône/cylindre bleu
            GLUquadric* quad = gluNewQuadric();
            glColor3f(0.5f, 0.5f, 1.0f);
            if (m_type == ProjectileType::Cone)
                gluCylinder(quad, 0.0f, 0.2f, 0.4f, 16, 16);
            else
                gluCylinder(quad, 0.2f, 0.2f, 0.4f, 16, 16);
            gluDeleteQuadric(quad);
        }
        break;

    case ProjectileType::Cube:   // --- TNT (CUBE TEXTURÉ) ---
        if (m_textureId != 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, m_textureId);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            float s = 0.28f;
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

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
        } else {
            // Fallback: cube rouge
            float s = 0.28f;
            glColor3f(1.0f, 0.3f, 0.3f);
            glBegin(GL_QUADS);
            // (mêmes 6 faces, sans texture)
            glEnd();
        }
        break;
    }

    glPopMatrix();
}




QVector3D Projectile::getPosition() const
{
    return m_position;
}
