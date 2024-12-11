#ifndef BOX_H
#define BOX_H

class Box{

    public:
        int x;
        int y;
        bool visitedBox;
        bool walls[4];

        Box();
        ~Box();
        void PutVal(int,int);
        void Show();
       
        Box CheckNeighbour();
/*
        void MakeBold();
*/
};
#endif