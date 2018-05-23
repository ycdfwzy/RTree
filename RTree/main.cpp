#include "Geometry.h"
#include "RTree.h"
#include "RTreeNode.h"
#include <iostream>
#include <algorithm>
#include <ctime>

using namespace MyRTree;
using namespace std;

const int N = 10000;

Point<2> p[N];
bool flag[N];

int check(const Rect<2>& rec) {
	int ret = 0;
	for (int i = 0; i < N; ++i)
	if (flag[i]){
		if (p[i].x[0] >= rec.LeftBottom.x[0] && p[i].x[0] <= rec.RightTop.x[0] &&
			p[i].x[1] >= rec.LeftBottom.x[1] && p[i].x[1] <= rec.RightTop.x[1]) {
			ret++;
			//p[i].write();
		}
	}
	return ret;
}

int main() {
	srand(time(0));

	bool wrong = true;
	while (wrong) {
		RTree<> rtree(4);
		//cout << "NEW" << endl;
		memset(flag, true, sizeof(flag));
		for (int i = 0; i < N; ++i) {
			p[i].x[0] = rand() % 10000;
			p[i].x[1] = rand() % 1000;
			Rect<> rec;
			rec.LeftBottom = p[i];
			rec.RightTop = p[i];
			rtree.Insert(rec);
		}
		for (int w = 0; w < 100; ++w) {
			int x0 = rand() % 10000;
			int x1 = rand() % 10000;
			int x2 = rand() % 10000;
			int x3 = rand() % 10000;
			if (x0 > x1) swap(x0, x1);
			if (x2 > x3) swap(x2, x3);

			Rect<> rec;
			Point<> q;
			q.x[0] = x0; q.x[1] = x2;
			rec.LeftBottom = q;
			q.x[0] = x1; q.x[1] = x3;
			rec.RightTop = q;

			if (rand() % 3 < 2) {
				//cout << "Insert:" << endl;
				//rec.write();
				std::vector<Rect<>> res;
				rtree.search(rec, res);
				int t = check(rec);
				if (t == res.size()) {
					cout << "Right!" << endl;
					cout << endl;
					continue;
				}
				/*
				cout << endl;
				for (int i = 0; i < res.size(); ++i)
					res[i].LeftBottom.write();
				cout << endl;

				for (int i = 0; i < N; ++i)
					p[i].write();
				cout << res.size() << endl;
				cout << t << endl;

				rec.write();

				cout << w << endl;
				*/
				cout << "Wrong!" << endl;
				cout << endl;
				wrong = false;
				break;
			}
			else
			{
				//cout << "Delete:" << endl;
				//rec.write();
				//cout << endl;
				rtree.Delete(rec);

				for (int i = 0; i < N; ++i) {
					if (p[i].x[0] < rec.LeftBottom.x[0] || p[i].x[0] > rec.RightTop.x[0])
						continue;
					if (p[i].x[1] < rec.LeftBottom.x[1] || p[i].x[1] > rec.RightTop.x[1])
						continue;
					flag[i] = false;
				}
			}
		}
	}
	/*
	for (int i = 0; i < 10; ++i) {
		Point<> p;
		p.x[0] = p.x[1] = i;
		Rect<> rec;
		rec.LeftBottom = p;
		rec.RightTop = p;
		rtree.Insert(rec);
	}
	
	Rect<> rec;
	Point<> p;
	p.x[0] = 1; p.x[1] = 1;
	rec.LeftBottom = p;
	p.x[0] = 4; p.x[1] = 4;
	rec.RightTop = p;
	std::vector<Rect<>> res;
	rtree.search(rec, res);
	std::cout << res.size() << std::endl;
	res.clear();

	p.x[0] = 2; p.x[1] = 2;
	rec.LeftBottom = p;
	p.x[0] = 6; p.x[1] = 6;
	rec.RightTop = p;
	rtree.Delete(rec);
	
	p.x[0] = 1; p.x[1] = 1;
	rec.LeftBottom = p;
	p.x[0] = 4; p.x[1] = 4;
	rec.RightTop = p;
	rtree.search(rec, res);
	std::cout << res.size() << std::endl;
	res.clear();
	*/
	return 0;
}