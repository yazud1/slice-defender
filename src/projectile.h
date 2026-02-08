#ifndef PROJECTILE_H
#define PROJECTILE_H
#ifdef _WIN32
#include <windows.h>  // DOIT venir avant GL/gl.h et GL/glu.h
#endif

#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QVector3D>
#include <GL/gl.h>
#include <GL/glu.h>



enum class ProjectileType {
    Sphere,
    Cone,
    Cylindre,
    Cube
};

class Projectile
{
public:
    ~Projectile();
    Projectile(const QVector3D& position, const QVector3D& velocity, ProjectileType type, GLuint textureId);
    ProjectileType getType() const;
    void update(float deltaTime);
    void draw() const;
    QVector3D getVelocity() const { return m_velocity; }
    void drawCube() const;
    QVector3D getPosition() const;

private:
    QVector3D m_position;
    QVector3D m_velocity;
    QVector3D m_rotationAxis;
    float m_rotationAngle = 0.0f; // en degrés
    ProjectileType m_type;
    GLuint m_textureId = 0;

};





#endif // PROJECTILE_H
