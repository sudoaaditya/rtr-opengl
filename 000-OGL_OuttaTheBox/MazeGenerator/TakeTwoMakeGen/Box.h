#ifndef BOX_H
#define BOX_H

class Box{

    public:
        int x;
        int y;
        bool visitedBox;
        bool walls[4];


        Box(int, int);
        ~Box();
        void Show();
        int CheckNeighbour() ;
        friend int operator!(Box&);
        void MakeBold();

};
#endif