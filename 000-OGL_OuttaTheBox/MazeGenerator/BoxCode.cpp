#include<iostream>

class Box{

    public:
        int x;
        int y;
        bool visitedBox;
        bool walls[4];

        Box();
        ~Box();
        void PutVal(int,int);       
        Box& CheckNeighbour();
        void PrintBox();

};

Box Grid[25];
Box BoxRet;
int GetIndx(int, int);
int Random(int, int);
int cnt =1;

void Box::PutVal(int i, int j) {
    this->x = i;
    this->y = j;
    
    for(int i =0; i<4; i++) {
        walls[i] = true;
    }
    visitedBox = false;

}
Box::Box() {
    
    std::cout<<"Here"<<cnt<<std::endl;
    cnt++;
}

Box::~Box() {
    if(this) {
        delete(this);
    }
}

Box& Box::CheckNeighbour() {
    
    std::cout<<" Entered Termination"<<std::endl;

    int Indx =0;
    Box Negh[4];

    Box top = Grid[GetIndx(this->x,this->y-1)];
    Box right = Grid[GetIndx(this->x+1,this->y)];
    Box bottom = Grid[GetIndx(this->x,this->y+1)];
    Box left = Grid[GetIndx(this->x-1,this->y)];

    if(!top.visitedBox) {
        Negh[Indx] = top;
        Indx++;
    }
    if(!right.visitedBox) {
        Negh[Indx] = right;
        Indx++;
    }
    if(!bottom.visitedBox) {
        Negh[Indx] = bottom;
        Indx++;
    }
    if(!left.visitedBox) {
        Negh[Indx] = left;
        Indx++;
    }

    int pIndx = Random(0,Indx);

    return(Negh[pIndx]);
    
}

void Box::PrintBox() {

    std::cout<<this->x<<std::endl;
    std::cout<<this->y<<std::endl;
    std::cout<<this->visitedBox<<std::endl;

}

int main(void) {

    for(int i=0; i<10; i++) {
        for(int j=0; j<5; j++) {
          //  printf("%d & %d & %d\n",i,j,i+(j*5));
            Grid[j+(i*5)].PutVal(i,j);
        }
    }
    
    std::cout<<"Termination1"<<std::endl;
    Box Curr = Grid[0]; 
     std::cout<<"Termination"<<std::endl;
    Curr.visitedBox = true;

   Curr.PrintBox();   
/*
    while(1) {
        std::cout<<"Termination"<<std::endl;

        Box next = Curr.CheckNeighbour();
        
        std::cout<<"Termination"<<std::endl;

        if(next.visitedBox == false){
            next.visitedBox = true;
        }
        else {
            PrintBox(next);
            std::cout<<"Termination"<<std::endl;
            break;
        }
        
        PrintBox(next);

        Curr = next;

    }
*/
//    for(int i=0; i<50; i++) {
       // printf("Reached 3\n");
  //      printf("Grid{%d} == (%d, %d)\n",i,Grid[i].x, Grid[i].y);
//    }

    return(0);
}

int GetIndx(int i, int j) {
    if(i < 0 || j < 0 || i > 9 || j > 4) {
        return(-1);
    }
    else {
        return(j+(i*5));
    }
}

int Random(int min, int max) {

    return( min + rand() / (RAND_MAX / (max - min + 1) + 1));
}

