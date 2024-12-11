
#include<iostream>
using namespace std;

int Random(int min, int max) {

    //cout<<"Max:"<<max<<endl;
    int ret = min + rand() % (max - min + 1);
    cout<<"Rand:"<<ret<<endl;
    return( ret);
}

int main() {

    while(1) {
    Random(0, 4);
    }
    
    return(0);

}