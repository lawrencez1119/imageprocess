#include <iostream>
#include<opencv2\opencv.hpp>
#include<stdlib.h>
#include<time.h>

#define WIDTH 1000
#define HEIGHT 1000
#define MAXSIZE 50
using namespace cv;
using namespace std;

class MyVertex
{
public:
	int x;
	int y;
	bool isdeleted = false;

	MyVertex(int tmpx, int tmpy) { x = tmpx; y = tmpy; isdeleted = false; }
};

MyVertex minu(MyVertex a, MyVertex b)
{
	MyVertex r(a.x - b.x, a.y - b.y);
	return r;
}

bool isCW(MyVertex i, MyVertex j, MyVertex k)
{
	if (((j.x - i.x)*(k.y - i.y) - (k.x - i.x)*(j.y - i.y)) < 0)
		return true;
	else
		return false;
}

int main()
{
	vector<MyVertex> convex;
	vector<MyVertex> points;
	MyVertex startponit1(223, 11);
	MyVertex startponit2(125, 342);
	MyVertex startponit3(340, 100);
	points.push_back(startponit1);
	points.push_back(startponit2);
	points.push_back(startponit3);
	srand((unsigned)time(NULL));
	for (int i = 0; i < MAXSIZE-3; i++)
	{
		int x=(rand()%(500-0+1))+0;
		int y = (rand()%(500-0+1))+0;
		MyVertex tmpVertex(x,y);
		points.push_back(tmpVertex);
		//cout << points[i].x << " "<<points[i].y << endl;
	}

	Mat background(WIDTH, HEIGHT, CV_8UC3, Scalar(255, 255, 255));
	//namedWindow("img");
	for (int i = 0; i < MAXSIZE; i++)
	{
		Point center = Point(points[i].x, points[i].y);
		circle(background, center, 3, Scalar(0, 0, 0), -1);
	}
	convex.push_back(startponit1);
	convex.push_back(startponit2);
	convex.push_back(startponit3);

	vector<MyVertex> ::iterator piter;
	vector<MyVertex> ::iterator citer1;
	vector<MyVertex> ::iterator citer2;
	vector<MyVertex> ::iterator it;
	int delt = 0;
	vector<MyVertex> ::iterator startpos;
	bool isfirst = true;
	for (piter = points.begin()+3; piter != points.end(); piter++)
	{
		for (citer1 = convex.begin(); citer1 != convex.end(); citer1++)
		{
			if (citer1 != convex.end() - 1)
			{
				citer2 = citer1+1;
			}
			else
				citer2 = convex.begin();
			
			if ( citer1!=convex.end()&& citer2!=convex.end()&& isCW(*citer1, *citer2, *piter))//minu(*citer2,*citer1),minu(*piter, *citer2), minu(*citer1, *piter))
			{
				/*if (isfirst)
				{
					startpos = citer1;
					isfirst = false;
				}*/
				delt++;
				(*citer2).isdeleted = true;
				
			}

		}
		if (delt == 1)
		{
			for (it = convex.begin(); it != convex.end();it++)
			{
				if ((*it).isdeleted)
				{
					(*it).isdeleted = false;
					convex.insert(it, *piter);
					break;
				}
			}

			delt = 0;
		}
		else if (delt > 1)
		{
			for (it = convex.begin(); it != convex.end(); it++)
			{
				if ((*it).isdeleted && it!=convex.end()-1 &&!(*(it+1)).isdeleted)
				{
					(*it).isdeleted = false;
				}
				else if ((*it).isdeleted && it== convex.end()-1 &&!(*convex.begin()).isdeleted)
				{
					(*it).isdeleted = false;
				}
				if (it == convex.end())
				{
					break;
				}
			}
			for (it = convex.begin(); it != convex.end();)
			{
				if ((*it).isdeleted)
				{
					it = convex.erase(it);
					citer1 = it;
				}
				else
					it++;
				if (it == convex.end())
				{
					break;
				}
			}
			//citer1=convex.erase(startpos + 1, startpos+delt);
			if(citer1==convex.end())
			    convex.insert(convex.begin(), *piter);
			else
				convex.insert(citer1, *piter);
			
			delt = 0;
			//isfirst = true;
		}

	}
	for (citer1 = convex.begin(); citer1 != convex.end(); citer1++)
	{
		if (citer1 != convex.end() - 1)
		{
			citer2 = citer1 + 1;
		}
		else
			citer2 = convex.begin();
		Point a=Point((*citer1).x, (*citer1).y);
		/*if (citer1 != convex.end() - 1)
			citer1++;
		else 
			citer1 = convex.begin();*/
		Point b= Point((*citer2).x, (*citer2).y);
		line(background, a, b, Scalar(0, 0, 255));
		/*if (citer1 != convex.begin())
			citer1--;
		else
			citer1 = convex.end() - 1;*/
	}
	

	imshow("img",background);
	waitKey(0);


	
}
