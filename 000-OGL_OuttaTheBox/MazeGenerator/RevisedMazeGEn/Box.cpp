#include<iostream>
#include<vector>
#include"Box.h"
using namespace std;

vector<Box> Grid;
vector<Box**> temp;
Box *b;

int GetIndx(int, int);
int Random(int, int);
int icount =0;

Box::Box(int x_ , int y_) {
    x = x_;
    y = y_;

    for(int i=0; i<4; i++) {
        walls[i] = true;
    }

    visitedBox = false;
}

Box::~Box() {

}

void Box::Show() {

    cout<<"( X:"<<this->x<<", Y:"<<this->y<<" )\n";
    cout<<"Vsisted"<<this->visitedBox<<endl;
}

Box** Box::CheckNeighbour(){

    int retI,indx=0;
    

    retI= GetIndx(this->x, this->y-1);
    if(retI != -1) {
        //top
         Box *top = &Grid.at(retI);
        
        if(!top->visitedBox && top != nullptr ){
            temp.push_back(&top);
            indx++;
        }
        
    }

    retI = GetIndx(this->x+1, this->y);
    if(retI != -1) {
        Box *right = &Grid.at(retI);
        if(!right->visitedBox && right != nullptr ){
            temp.push_back(&right);
            indx++;
        }   
    }

    retI =GetIndx(this->x, this->y+1);
    if(retI != -1) {
        Box *bottom = &Grid.at(retI);
        if(!bottom->visitedBox && bottom != nullptr ){
            temp.push_back(&bottom);
            indx++;
        }
    }

    retI = GetIndx(this->x-1, this->y);
    if(retI != -1) {
        Box *left = &Grid.at(retI);
        if(!left->visitedBox && left != nullptr ){
            temp.push_back(&left);
            indx++;
        }
    }
    
    if(indx == 0) {
        cout<<"Here AT Zero!!.."<<endl;
        b = new Box(-1, -2);
        return(&b);
    }

   /* for(int i=0; i<temp.size(); i++) {
        temp[i].Show();
    }
    */   
    int rIndx = Random(0, indx-1);

    return(temp[rIndx]);
}

int operator!(Box &b) {

    if(b.x>=0 && b.y>=0) {
        return(1);
    }
    else {
        return(0);
    }
}

int GetIndx(int i, int j) {
    if(i < 0 || j < 0 || i > 10-1 || j > 5-1) {
        return(-1);
    }
    else {
        int ret = (i*5)+j;
        cout<<"Ind:"<<ret<<endl;
        return(ret);
    }
}

int Random(int min, int max) {

    //cout<<"Max:"<<max<<endl;
    int ret = min + rand() % (max - min + 1);
    cout<<"Rand:"<<ret<<endl;
    return( ret);
}


int main() {

    for(int i=0; i<10; i++) {
        for(int j=0; j<5; j++) {

            Grid.push_back(Box(i,j));
        }
    }

    for(int i=0; i != Grid.size(); i++) {
        Grid[i].Show();
    }

    cout<<"_________________________________________________"<<endl;
    Box *Current = &Grid.at(0);
    Current->visitedBox = true;

    while(!(*Current)) {

        Current->Show();
        icount++;
        Box **Next = Current->CheckNeighbour();

        if(!(*Next)->visitedBox)  {
            (*Next)->visitedBox = true;

            
        }
           // Next.Show();

    }
    Current->Show();
    cout<<"Itr : "<<icount<<endl;

        for(int i=0; i != Grid.size(); i++) {
        Grid[i].Show();
    }

    cout<<"_________________________________________________"<<endl;
    return(0);

}