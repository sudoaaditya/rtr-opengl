#include<gl/GLU.h>

class Bubble {

    private:
        GLfloat fxTrans;
        GLfloat fyTrans;
        GLUquadric *sphere;

    public:
        Bubble(GLfloat, GLfloat);
        ~Bubble();
        void Show(void);
        void CalculateTrans(void);
               
};