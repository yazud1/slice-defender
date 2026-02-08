#ifndef FRAGMENT_H
#define FRAGMENT_H

#include <QVector3D>
#include "projectile.h"

class Fragment
{
public:
    Fragment(const QVector3D& position, const QVector3D& velocity, float rotationSpeed, ProjectileType type, GLuint textureId = 0);  // AJOUTÉ textureId
    
    void update(float deltaTime);
    void draw() const;
    bool isAlive() const;

private:
    QVector3D m_position;
    QVector3D m_velocity;
    float m_rotationSpeed;
    float m_rotation;
    float m_lifetime;
    ProjectileType m_type;
    GLuint m_textureId;  // AJOUTÉ : stockage de la texture
    
    void drawCubeFragment() const;
    void drawSphereFragment() const;   // AJOUTÉ
    void drawConeFragment() const;     // AJOUTÉ
    void drawCylinderFragment() const; // AJOUTÉ
};


#endif // FRAGMENT_H
