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

const int querys = 1000;

const string file_gradient = "gradient.txt";
const string file_moment = "moment.txt";
const string file_color = "color.txt";
const string file_gray = "gray.txt";
const string file_lbp = "lbp.txt";

const int D_gradient = 64;
const int D_moment = 9;
const int D_color = 64;
const int D_gray = 64;
const int D_lbp = 24;

unordered_map<string, Point<D_gradient>> string2point_gradient;
unordered_map<string, Point<D_moment>> string2point_moment;
unordered_map<string, Point<D_color>> string2point_color;
unordered_map<string, Point<D_gray>> string2point_gray;
unordered_map<string, Point<D_lbp>> string2point_lbp;

unordered_map<string, string> point2string_gradient;
unordered_map<string, string> point2string_moment;
unordered_map<string, string> point2string_color;
unordered_map<string, string> point2string_gray;
unordered_map<string, string> point2string_lbp;

RTree<D_gradient> rtree_gradient(10);
RTree<D_moment> rtree_moment(10);
RTree<D_color> rtree_color(10);
RTree<D_gray> rtree_gray(10);
RTree<D_lbp> rtree_lbp(10);

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
int user_test(int N, int M) {
	Point<features> p;
	Rect<features> rect;
	RTree<features> rtree(M);
	for (int i = 0; i < N; ++i) {
		p.rand();
		rect.LeftBottom = p;
		rect.RightTop = p;
		rtree.Insert(rect);
	}

	int tot = 0;
	int tot_res_size = 0;
	vector<Rect<features>> res;
	for (int i = 0; i < querys; ++i) {
		//rect.rand();
		for (int j = 0; j < features; ++j)
			//p.x[j] = rand() % 1000;
			p.x[j] = 0;
		rect.LeftBottom = p;
		for (int j = 0; j < features; ++j)
			//p.x[j] = rand() % 1000 + 20000;
			p.x[j] = 30000;
		rect.RightTop = p;
		res.clear();
		rtree.search(rect, res, tot);
		tot_res_size += res.size();
		//tot += Search_test_time<features>(rtree, rect, res);
	}
	//tot /= querys;
	return tot / (tot_res_size / 100);
}

template<int Dimensions = 2>
void test_corectness_ratio(RTree<Dimensions>& rtree,
	unordered_map<string, Point<Dimensions>>& string2point,
	unordered_map<string, string>& point2string,
	int threshold) {
	int hit = 0;
	int miss = 0;
	for (auto p : string2point) {
		//cout << p.first << endl;
		vector<Point<Dimensions>> res;
		string S(p.first);
		while (S.back() != '_')
			S.pop_back();

		//rtree.KNN(threshold, p.second, res);
		Rect<Dimensions> rect;
		rect.LeftBottom = p.second;
		rect.LeftBottom.move(-0.05);
		rect.RightTop = p.second;
		rect.RightTop.move(0.05);
		rtree.search(rect, res);

		int N = res.size();
		for (Point<Dimensions>& rp : res)
			rp.decrease(p.second);
		std::sort(res.begin(), res.end(), [](const Point<Dimensions>& p1, const Point<Dimensions>& p2) {
			return p1.rank2() < p2.rank2();
		});
		for (Point<Dimensions>& rp : res)
			rp.increase(p.second);

		for (int i = 0; i < 100 && i < N; ++i) {
		//for (auto s : res) {
			Point<Dimensions>& s = res[i];
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
	test_corectness_ratio<D_gradient>(rtree_gradient, string2point_gradient, point2string_gradient, 10);
	cout << endl;

	cout << "Moment: " << endl;
	test_corectness_ratio<D_moment>(rtree_moment, string2point_moment, point2string_moment, 10);
	cout << endl;
	
	cout << "Color: " << endl;
	test_corectness_ratio<D_color>(rtree_color, string2point_color, point2string_color, 10);
	cout << endl;

	cout << "Gray: " << endl;
	test_corectness_ratio<D_gray>(rtree_gray, string2point_gray, point2string_gray, 10);
	cout << endl;
	*/
	cout << "LBP: " << endl;
	test_corectness_ratio<D_lbp>(rtree_lbp, string2point_lbp, point2string_lbp, 10);
	cout << endl;
}

int N[6] = { 1000,2000,3000,4000,5000,10000 };
int t[6][5];
bool v[10000];
Point<> p[10000];
string name;

int check(const Rect<>& rect) {
	int ret = 0;
	for (int i = 0; i < 1000; ++i)
	if (v[i]) {
		if (p[i].x[0] < rect.LeftBottom.x[0] || p[i].x[0] > rect.RightTop.x[0] ||
			p[i].x[1] < rect.LeftBottom.x[1] || p[i].x[1] > rect.RightTop.x[1])
			continue;
		++ret;
	}
	return ret;
}

template<int Dimensions = 2>
void problem_3(const string& filename, unordered_map<string, Point<Dimensions>>& string2point,
	unordered_map<string, string>& point2string, RTree<Dimensions>& rtree) {
	Point<Dimensions> p;
	ifstream input(filename);
	for (int j = 0; j < 5613; ++j) {
		getline(input, name);
		cout << name << endl;

		string svec;
		getline(input, svec);
		p.fromString(svec);

		point2string.insert(make_pair(p.toString(), name));
		string2point.insert(make_pair(name, p));
		rtree.Insert(p);
	}
	input.close();
}

int main() {
	srand(time(0));
	/*
	RTree<> rtree(4);
	memset(v, true, sizeof(v));
	for (int i = 0; i < 1000; ++i) {
		p[i].rand();
		rtree.Insert(p[i]);
	}
	while (true) {
		Rect<> rec;
		rec.rand();
		if (rec.LeftBottom.x[0] > rec.RightTop.x[0])
			swap(rec.LeftBottom.x[0], rec.RightTop.x[0]);
		if (rec.LeftBottom.x[1] > rec.RightTop.x[1])
			swap(rec.LeftBottom.x[1], rec.RightTop.x[1]);
		
		if (rand() % 3 < 2) {
			vector<Point<>> res;
			rtree.search(rec, res);
			int t = check(rec);
			if (t == res.size()) {
				cout << "Right!" << endl;
				continue;
			}
			cout << "Wrong!" << endl;

			rec.write();

			cout << res.size() << endl;
			for (int i = 0; i < res.size(); ++i)
				res[i].write();

			cout << t << endl;
			for (int i = 0; i < 1000; ++i)
			if (v[i]) {
				if (p[i].x[0] < rec.LeftBottom.x[0] || p[i].x[0] > rec.RightTop.x[0] ||
					p[i].x[1] < rec.LeftBottom.x[1] || p[i].x[1] > rec.RightTop.x[1])
					continue;
				p[i].write();
			}
			break;
		}
		else
		{
			rtree.Delete(rec);
			for (int i = 0; i < 1000; ++i) {
				if (p[i].x[0] < rec.LeftBottom.x[0] || p[i].x[0] > rec.RightTop.x[0] ||
					p[i].x[1] < rec.LeftBottom.x[1] || p[i].x[1] > rec.RightTop.x[1])
					continue;
				v[i] = false;
			}
		}
	*/
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
			cout << N[i] << " " << (1<<(j+2)) << " " << t[i][j] << endl;
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
	
	/*
	ifstream input("allfeatures.txt");
	for (int j = 0; j < 5613; ++j) {
		getline(input, name);
		cout << name << endl;
		
		Point<D_gradient> gradient;
		Point<D_moment> moment;
		Point<D_color> color;
		Point<D_gray> gray;

		string svec;
		getline(input, svec);
		gradient.fromString(svec);
	
		getline(input, svec);
		moment.fromString(svec);
	
		getline(input, svec);
		color.fromString(svec);
	
		getline(input, svec);
		gray.fromString(svec);
		
		string2point_gradient.insert(make_pair(name, gradient));
		string2point_moment.insert(make_pair(name, moment));
		string2point_color.insert(make_pair(name, color));
		string2point_gray.insert(make_pair(name, gray));
		
		
		point2string_gradient.insert(make_pair(gradient.toString(), name));
		point2string_moment.insert(make_pair(moment.toString(), name));
		point2string_color.insert(make_pair(color.toString(), name));
		point2string_gray.insert(make_pair(gray.toString(), name));
		
		rtree_gradient.Insert(gradient);
		rtree_moment.Insert(moment);
		rtree_color.Insert(color);
		rtree_gray.Insert(gray);
	}
	input.close();
	*/
/*
	ifstream input("lbp.txt");
	for (int j = 0; j < 5613; ++j) {
		getline(input, name);
		cout << name << endl;
		Point<D_lbp> lbp;

		string svec;
		getline(input, svec);
		lbp.fromString(svec);

		point2string_lbp.insert(make_pair(lbp.toString(), name));
		string2point_lbp.insert(make_pair(name, lbp));
		rtree_lbp.Insert(lbp);
	}
	input.close();
*/
	
	problem_3<D_gradient>(file_gradient, string2point_gradient, point2string_gradient, rtree_gradient);
	problem_3<D_moment>(file_moment, string2point_moment, point2string_moment, rtree_moment);
	problem_3<D_color>(file_color, string2point_color, point2string_color, rtree_color);
	problem_3<D_gray>(file_gray, string2point_gray, point2string_gray, rtree_gray);
	problem_3<D_lbp>(file_lbp, string2point_lbp, point2string_lbp, rtree_lbp);
	
	cout << "insert finished!" << endl;
	test_correctness_main();

	
	string2point_gradient.clear();
	string2point_moment.clear();
	string2point_color.clear();
	string2point_gray.clear();
	string2point_lbp.clear();
	
	point2string_gradient.clear();
	point2string_moment.clear();
	point2string_color.clear();
	point2string_gray.clear();
	point2string_lbp.clear();
	
	return 0;
}