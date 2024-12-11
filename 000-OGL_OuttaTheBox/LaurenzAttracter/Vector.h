#ifndef VECTOR_H
#define VECTOR_H

class Vector {

    public:
        int x;
        int y;
        int z;

    Vector(int x_, int y_, int z_) {
        x = x_;
        y = y_;
        z = z_;
    }

    ~Vector() {
        
    }

};

#endif