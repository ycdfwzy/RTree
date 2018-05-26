#include "Geometry.h"
#include "RTree.h"
#include "RTreeNode.h"
#include "windows.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <unordered_map>

using namespace MyRTree;
using namespace std;

const int querys = 5000;

unordered_map<string, Point<350>> string2point_gradient;
unordered_map<string, Point<9>> string2point_moment;
unordered_map<string, Point<256>> string2point_color;
unordered_map<string, Point<256>> string2point_gray;

unordered_map<string, string> point2string_gradient;
unordered_map<string, string> point2string_moment;
unordered_map<string, string> point2string_color;
unordered_map<string, string> point2strng_gray;

RTree<350> rtree_gradient(10);
RTree<9> rtree_moment(10);
RTree<256> rtree_color(10);
RTree<256> rtree_gray(10);

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

template<int Dimensions = 2>
void test_corectness_ratio(RTree<Dimensions>& rtree,
	unordered_map<string, Point<Dimensions>>& string2point,
	unordered_map<string, string>& point2string,
	int threshold) {
	int hit = 0;
	int miss = 0;
	for (auto p : string2point) {
		vector<Point<Dimensions>> res;
		string S(p.first);
		while (S.back() != '_')
			S.pop_back();

		rtree.KNN(threshold, p.second, res);
		for (auto s : res) {
			string pat(point2string[s.toString()]);
			while (pat.back() != '_')
				pat.pop_back();
			if (S.compare(pat) == 0) {
				hit++;
			}
			else miss++;
		}
		hit--;
	}
	cout << "total hit: " << hit << endl;
	cout << "total miss: " << miss << endl;
	cout << (double)hit / (double)(hit + miss) << endl;
}

void test_correctness_main() {
	/*
	cout << "Gradient: " << endl;
	cout << "threshold = 10" << endl;
	test_corectness_ratio<350>(rtree_gradient, string2point_gradient, point2string_gradient, 10);
	cout << endl;
	*/
	cout << "Moment: " << endl;
	cout << "threshold = 10" << endl;
	test_corectness_ratio<9>(rtree_moment, string2point_moment, point2string_moment, 10);
	cout << endl;
	/*
	cout << "Color: " << endl;
	cout << "threshold = 10" << endl;
	test_corectness_ratio<256>(rtree_color, string2point_color, point2string_color, 10);
	cout << endl;

	cout << "Moment: " << endl;
	cout << "threshold = 10" << endl;
	test_corectness_ratio<256>(rtree_gray, string2point_gray, point2strng_gray, 10);
	cout << endl;


	cout << "Gradient: " << endl;
	cout << "threshold = 20" << endl;
	test_corectness_ratio<350>(rtree_gradient, string2point_gradient, point2string_gradient, 20);
	cout << endl;
	*/
	cout << "Moment: " << endl;
	cout << "threshold = 20" << endl;
	test_corectness_ratio<9>(rtree_moment, string2point_moment, point2string_moment, 20);
	cout << endl;
	/*
	cout << "Color: " << endl;
	cout << "threshold = 20" << endl;
	test_corectness_ratio<256>(rtree_color, string2point_color, point2string_color, 20);
	cout << endl;

	cout << "Moment: " << endl;
	cout << "threshold = 20" << endl;
	test_corectness_ratio<256>(rtree_gray, string2point_gray, point2strng_gray, 20);
	cout << endl;
	*/
}

int N[6] = { 1000,2000,3000,4000,5000,100000 };
double t[6][5];

string name;

int main() {
	srand(time(0));
	/*
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
	*/
	/*
	RTree<2> rtree(4);
	Point<2> p;
	Rect<2> rect;
	for (int i = 0; i < 10; ++i) {
		p.x[0] = i; p.x[1] = i;
		rect.LeftBottom = p;
		rect.RightTop = p;
		rtree.Insert(rect);
	}
	vector<Point<2>> res;
	p.x[0] = 2.5;
	p.x[1] = 2.5;
	rtree.KNN(4, p, res);
	for (int i = 0; i < res.size(); ++i)
		cout << res[i].x[0] << " " << res[i].x[1] << endl;
	*/
	ifstream input("allfeatures.txt");
	/*
	std::stringstream buffer;
	buffer << input.rdbuf();
	std::string contents(buffer.str());
	cout << contents << endl;
	*/
	for (int j = 0; j < 5613; ++j) {
		getline(input, name);
		//input >> name;
		cout << name << endl;
		
		Point<350> gradient;
		Point<9> moment;
		Point<256> color;
		Point<256> gray;

		string svec;
		getline(input, svec);
		gradient.fromString(svec);
		//gradient.write();
	
		getline(input, svec);
		moment.fromString(svec);
		//moment.write();
	
		getline(input, svec);
		color.fromString(svec);
		//color.write();
	
		getline(input, svec);
		gray.fromString(svec);
		//color.write();
		
		string2point_gradient.insert(make_pair(name, gradient));
		string2point_moment.insert(make_pair(name, moment));
		string2point_color.insert(make_pair(name, color));
		string2point_gray.insert(make_pair(name, gray));
		
		
		point2string_gradient.insert(make_pair(gradient.toString(), name));
		point2string_moment.insert(make_pair(moment.toString(), name));
		point2string_color.insert(make_pair(color.toString(), name));
		point2strng_gray.insert(make_pair(gray.toString(), name));
		
		rtree_gradient.Insert(gradient);
		rtree_moment.Insert(moment);
		rtree_color.Insert(color);
		rtree_gray.Insert(gray);
		
	}
	input.close();
	cout << "insert finished!" << endl;

	test_correctness_main();

	
	string2point_gradient.clear();
	string2point_moment.clear();
	string2point_color.clear();
	string2point_gray.clear();
	
	point2string_gradient.clear();
	point2string_moment.clear();
	point2string_color.clear();
	point2strng_gray.clear();
	
	return 0;
}