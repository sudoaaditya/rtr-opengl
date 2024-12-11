#include<iostream>
#include<gl/GLU.h>
#include<gl/GL.h>

class Planet {

    public:
        const char* m_PlanetName;
        GLUquadric *m_quadPlnetObj;
        GLfloat m_fPlanetRad;
        GLfloat m_fTransFromSun;
        GLfloat m_fRotationAlongSun;
        GLfloat m_fRevolutions;
        GLfloat m_RotUpVal;
        GLfloat m_RevUpVal;
        GLuint  m_iPlanetTex;
        GLfloat m_fTransX;
        GLfloat m_fTransY;
        GLfloat m_fTransZ;
        GLfloat m_RingRadArr[2];
        class Planet *m_NextPlanet;
};


class PlanetQueue {

    public:
        PlanetQueue();
        ~PlanetQueue();

        void EnqueuePlanet(const char*);
        void DequeuePlanet();
        void Render();
        void RenderFour();
        void UpdatePlanetPos();
        void InsertData(class Planet*&, const char*);
};