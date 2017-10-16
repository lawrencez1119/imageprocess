#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <opencv2\opencv.hpp>
#include <time.h>
using namespace std;
using namespace cv;
#define WIDTH 1000
#define HEIGHT 1000
//点
class vertex
{
public:
	double x;
	double y;
	vertex(){};
	vertex(double x, double y) { this->x = x; this->y = y; }
};
//边
class edge :public vertex
{
public:
	vertex v1;//起点
	vertex v2;//终点
	edge() {};
	edge(vertex v1, vertex v2) { this->v1 = v1; this->v2 = v2; }
};

//链表结点结构
class triangle
{
public:
	//三点的索引值
	int v1;
	int v2;
	int v3;
	triangle* pre;
	triangle* next;
	triangle() {};
	triangle(int a, int b, int c) { this->v1 = a; this-> v2 = b; this-> v3 = c; this->pre = NULL; this->next = NULL; }
};

//三角形的链表
class mesh:public vertex,public triangle
{
public:
	int v_num;//点数
	int tri_num;//三角形数
	vector<vertex> vArr;
	triangle* triArr;
	mesh() {};
};

//判断是否在圆内，为真则在圆内需要翻转
bool inCircle(vertex* i, vertex* k, vertex* j, vertex* l)
{
	double ixy = i->x*i->x + i->y*i->y;
	double kxy = k->x*k->x + k->y*k->y;
	double jxy= j->x*j->x + j->y*j->y;
	double lxy= l->x*l->x + l->y*l->y;
	double det1 = i->x*k->y - k->x*i->y;
	double det2= i->x*j->y - j->x*i->y;
	double det3 = i->x*l->y - l->x*i->y;
	double det4 = k->x*j->y - j->x*k->y;
	double det5 = k->x*l->y - l->x*k->y;
	double det6 = j->x*l->y - l->x*j->y;

	double det = det1*(jxy - lxy)
		- det2*(kxy - lxy)
		+ det3*(kxy - jxy)
		+ det4*(ixy - lxy)
		- det5*(ixy - jxy)
		+ det6*(ixy - kxy);
	return det > 0;
}

double isTri(vertex* a, vertex* b, vertex* c)
{
	double s = (b->x - a->x)*(c->y - b->y) - (c->x - b->x)*(b->y - a->y);
	return s;
}

triangle* add(mesh *m, triangle* pre, int i1, int i2, int i3) {
	//不构成三角形
	if (fabs(isTri(&(m->vArr[i1]), &(m->vArr[i2]), &(m->vArr[i3])))==0)
	{
		return pre;
	}
	triangle* tri = new triangle();
	tri->v1 = i1;
	tri->v2 = i2;
	tri->v3 = i3;
	if (pre==NULL)
	{
		m->triArr = tri;
		tri->next = NULL;
		tri->pre = NULL;
	}
	else {
		tri->next = pre->next;
		tri->pre = pre;
		if (pre->next!=NULL)
		{
			pre->next->pre = tri;
		}
		pre->next = tri;
	}
	return tri;
}

//删除三角形
void deleteTri(mesh* m, triangle* tri)
{
	if (tri==NULL)
	{
		return;
	}
	if (tri->pre!=NULL)
	{
		tri->pre->next = tri->next;
	}
	else {
		m->triArr = tri->next;//重新赋值
	}
	if (tri->next!=NULL)
	{
		tri->next->pre = tri->pre;
	}
	delete tri;
}

int LocatePoint(mesh* m, int index, triangle* tri)
{
	vertex* v = &(m->vArr[index]);
	vertex* vi = &(m->vArr[tri->v1]);
	vertex* vj = &(m->vArr[tri->v2]);
	vertex* vk = &(m->vArr[tri->v3]);
	double a0 = isTri(vi, vj, v);
	double a1 = isTri(vj, vk, v);
	double a2 = isTri(vk, vi, v);

	if (a0 > 0 && a1 > 0 && a2 > 0)
	{
		return 1;//在三角形内部
	}
	else if ((a0*a1 > 0 && a2 == 0) || (a0*a2 > 0 && a1 == 0) || (a1*a2 > 0 && a0 == 0))
		return 2;//在三角形边界上
	return 0;//不存在
}

bool swapEdge(mesh* m, triangle* tri)
{
	int a = tri->v1;
	int b = tri->v2;
	int c = tri->v3;
	int d = -1;

	int* pv;
	int index;
	bool flag = false;

	int tmp[3] = { 0,0,0 };
	triangle* p = m->triArr;
	while (p!=NULL)
	{
		tmp[0] = 0;
		tmp[1] = 0;
		tmp[2] = 0;

		pv = &(p->v1);
		for (int j = 0, k = 1; j < 3; j++, k *= 2) {
			index = *pv;
			pv++;
			if (index == a || index == b)
			{
				tmp[j] = k;
			}
		}

		switch (tmp[0] | tmp[1] | tmp[2])
		{
		case 3:
			if (isTri(&(m->vArr[a]), &(m->vArr[b]), &(m->vArr[p->v3])) < 0)
			{
				d = p->v3;
			}
			break;
		case 5:
			if (isTri(&(m->vArr[a]), &(m->vArr[b]), &(m->vArr[p->v2])) < 0)
			{
				d = p->v2;
			}
			break;
		case 6:
			if (isTri(&(m->vArr[a]), &(m->vArr[b]), &(m->vArr[p->v1])) < 0)
			{
				d = p->v1;
			}
			break;
		default:
			break;

		}
		if (d != -1)
		{
			vertex* va = &(m->vArr[a]);
			vertex* vb = &(m->vArr[b]);
			vertex* vc = &(m->vArr[c]);
			vertex* vd = &(m->vArr[d]);

			if (inCircle(va, vb, vc, vd)) {
				flag = true;
				triangle* t1 = add(m, tri, a, d, c);
				triangle* t2 = add(m, t1, d, b, c);
				deleteTri(m, tri);
				deleteTri(m, p);

				swapEdge(m, t1);
				swapEdge(m, t2);
				break;
			}
		}
		p = p->next;
	}
	return flag;

}
//点在三角形内部
void FaceSplitInside(mesh* m,triangle* tri,int index)
{
	int a = tri->v1, b = tri->v2, c = tri->v3;
	if (tri==NULL)
		return;
	triangle* newTri1 = NULL, *newTri2 = NULL,*newTri3 = NULL;
	newTri1 = add(m, tri, a, b, index);
	newTri2 = add(m, newTri1, b, c, index);
	newTri3 = add(m, newTri2, c, a, index);
	deleteTri(m, tri);

	swapEdge(m, newTri1);
	swapEdge(m, newTri2);
	swapEdge(m, newTri3);
}

//点在边界
void FaceSplitBoundary(mesh* m,triangle* tri,int index)
{
	int a = tri->v1;
	int b = tri->v2;
	int c = tri->v3;
	triangle* newTri1 = NULL, *newTri2 = NULL, *newTri3 = NULL;
	newTri1 = add(m, tri, a, b, index);
	newTri2 = add(m, newTri1, b, c, index);
	newTri3 = add(m, newTri2, c, a, index);
	deleteTri(m, tri);
	
	if (newTri1 == tri)
	{
		swapEdge(m, newTri2);
		swapEdge(m, newTri3);
	}
	else if (newTri2 == newTri1)
	{
		swapEdge(m, newTri1);
		swapEdge(m, newTri3);
	}
	else if (newTri3 == newTri2)
	{
		swapEdge(m, newTri1);
		swapEdge(m, newTri2);
	}
}

void insert(mesh* m, int index)
{
	triangle* tri = m->triArr;
	triangle* first = NULL;
	triangle* sec = NULL;
	int inEdge = 0;
	int inTri = -1;

	while (tri != NULL)
	{
		inTri = LocatePoint(m, index, tri);
		if (inTri == 1)
		{
			break;
		}
		else if (inTri == 2) {
			if (inEdge == 0) {
				first = tri;
				inEdge++;
			}
			else {
				sec = tri;
				break;
			}
		}
		tri = tri->next;
	}
	if (first != NULL&&sec != NULL)
	{
		FaceSplitBoundary(m, first, index);
		FaceSplitBoundary(m, sec, index);
	}
	else if (tri != NULL)
	{
		FaceSplitInside(m, tri, index);
	}
}

void deleteBox(mesh* m)
{
	int statify[3] = { 0,0,0 };
	int index;
	int* pv;

	triangle* pTri = m->triArr;
	triangle* pNext = NULL;
	while (pTri != NULL) {
		pNext = pTri->next;
		statify[0] = 0;
		statify[1] = 0;
		statify[2] = 0;

		pv = &(pTri->v1);
		for (int j = 0, k = 1; j < 3; j++, k*= 2)
		{
			index = *pv;
			pv++;

			if (index == 0 || index == 1 || index == 2)
			{
				statify[j] = k;
			}
	    }

		switch (statify[0] | statify[1] | statify[2])
		{
		   case 0:
			   break;
		   case 1:
		   case 2:
		   case 4: 
			   deleteTri(m, pTri);
			   break;
		   case 3:
		   case 5:
		   case 6:
			   deleteTri(m, pTri);
			   break;
		   case 7:
			   deleteTri(m, pTri);
			   break;
		   default:
			   break;
		}
		pTri = pNext;
	}
}


void delaunay()
{
	mesh* m = new mesh();
	int n;
	cin >> n;
	double max = -10;
	vertex v(0, 0);
	m->vArr.push_back(v);
	m->vArr.push_back(v);
	m->vArr.push_back(v);

	ofstream of("C:\\Users\\Lawrence\\Desktop\\delaunay.txt");
	srand((unsigned int)time(NULL));
	Mat background(WIDTH, HEIGHT, CV_8UC3, Scalar(255, 255, 255));
	for (int i = 0; i < n; i++)//画点
	{
		double x = (rand() % (500 - 0 + 1)) + 0;
		double y = (rand() % (500 - 0 + 1)) + 0;
		of << x << ' ' << y << endl;
		if (fabs(x) > max)
			max = fabs(x);
		if (fabs(y) > max)
			max = fabs(y);
		vertex tmp(x, y);
		m->vArr.push_back(tmp);
		Point center(x, y);
		circle(background, center, 3, Scalar(0, 0, 0), -1);
	}
	of.close();
	vertex a(0, 2 * max);
	vertex b(-3 * max, -max);
	vertex c(3 * max, -max);
	m->vArr[0] = a;
	m->vArr[1] = b;
	m->vArr[2] = c;
	add(m, NULL, 0, 1, 2);
	for (int i = 3; i < m->vArr.size(); i++) {
		insert(m, i);
	}
	deleteBox(m);
	triangle* tmp = m->triArr;
	int i = 0;
	while (tmp != NULL)
	{
		i++;
		Point a(m->vArr[tmp->v1].x, m->vArr[tmp->v1].y);
		Point b(m->vArr[tmp->v2].x, m->vArr[tmp->v2].y);
		Point c(m->vArr[tmp->v3].x, m->vArr[tmp->v3].y);
		line(background, a, b, Scalar(0, 0, 255));
		line(background, b, c, Scalar(0, 0, 255));
		line(background, c, a, Scalar(0, 0, 255));

		tmp = tmp->next;

	}
	imshow("img", background);
	waitKey(0);

}
int main(int argc, char *argv[])
{
	delaunay();
	return 0;
}