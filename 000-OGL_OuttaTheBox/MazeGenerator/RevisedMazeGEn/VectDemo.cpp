#include<iostream>
#include<vector>
using namespace std;

class Point {

private:
	int x,y;

public:
	
	Point(int,int);
	~Point();
	void Show();
};

Point::Point(int x_, int y_) : x(x_), y(y_){
	cout<<"Const Called!!.."<<endl;
}	

Point::~Point() {
	cout<<"Dest Called!!.."<<endl;
}

void Point::Show() {
	cout<<"( X:"<<this->x<<", Y:"<<this->y<<" )\n";
}

int main() {
	vector<Point> v1;

	for(int i=0; i<10;i++) {
		for(int j=0;j<10;j++) {
			v1.push_back(Point(i,j));
		}
	}

	for (int  i=0;i<100;i++) {
		v1[i].Show();
		//v1 = v1.begin()+i;
	}
	return(0);
}

int GetIndx(int i, int j) {
	
}