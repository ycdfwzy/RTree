#include "Geometry.h"
#include "RTree.h"
#include "RTreeNode.h"
#include "windows.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>

using namespace MyRTree;
using namespace std;

const int querys = 5000;

template<int features>
double Search_test_time(RTree<features>& rtree, const Rect<features>& rect, vector<Rect<features>>& res) {


	LARGE_INTEGER winFreq;
	LARGE_INTEGER winStart, winNow;
	if (!QueryPerformanceFrequency(&winFreq))
		cout << "QueryPerformanceFrequency failed" << endl;

	if (!QueryPerformanceCounter(&winStart))
		cout << "QueryPerformanceCounter failed" << endl;
	rtree.search(rect, res);
	if (!QueryPerformanceCounter(&winNow))
		cout << "QueryPerformanceCounter failed" << endl;
	double time_cnt = (double)(winNow.QuadPart - winStart.QuadPart) / (double)winFreq.QuadPart;
	return time_cnt;
}

template<int features>
double user_test(int N, int M) {
	Point<features> p;
	Rect<features> rect;
	RTree<features> rtree(M);
	for (int i = 0; i < N; ++i) {
		p.rand();
		rect.LeftBottom = p;
		rect.RightTop = p;
		rtree.Insert(rect);
	}

	double tot = 0;
	vector<Rect<features>> res;
	for (int i = 0; i < querys; ++i) {
		rect.rand();
		res.clear();
		tot += Search_test_time<features>(rtree, rect, res);
	}
	//tot /= querys;
	return tot;
}

int N[6] = { 1000,2000,3000,4000,5000,100000 };
double t[6][5];

int main() {
	srand(time(0));
	for (int i = 0; i < 6; ++i) {
		t[i][0] = user_test<4>(N[i], 10);
		t[i][1] = user_test<8>(N[i], 10);
		t[i][2] = user_test<16>(N[i], 10);
		t[i][3] = user_test<32>(N[i], 10);
		t[i][4] = user_test<64>(N[i], 10);
	}
	for (int i = 0; i < 6; ++i)
		for (int j = 0; j < 5; ++j)
			cout << N[i] << " " << (1<<(j+2)) << " " << t[i][j] << " sec" << endl;
	return 0;
}