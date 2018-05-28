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
const int D_lbp = 36;

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

unordered_map<string, int> cat;

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
	Point<features> *point = new Point<features>[N];
	ifstream input("lbp.txt");
	for (int i = 0; i < N; ++i) {
		string name;
		getline(input, name);

		string svec;
		getline(input, svec);
		p.fromString(svec);

		point[i] = p;
		rect.LeftBottom = p;
		rect.RightTop = p;
		rtree.Insert(rect);
	}
	input.close();

	int tot = 0;
	int tot_res_size = 0;
	vector<Rect<features>> res;
	for (int i = 0; i < querys; ++i) {
		rect.LeftBottom = point[rand() % N];
		rect.RightTop = rect.LeftBottom;
		rect.LeftBottom.move(-0.02);
		rect.RightTop.move(0.02);
		//rect.RightTop = point[rand() % N];
		//for (int j = 0; j < features; ++j) {
			//if (rect.LeftBottom.x[j] > rect.RightTop.x[j])
			//	swap(rect.LeftBottom.x[j], rect.RightTop.x[j]);
			//if (rand() & 1)
			//	rect.LeftBottom.x[j] /= 4;
			//if (rand() & 1)
			//	rect.RightTop.x[j] *= 4;
		//}

		res.clear();
		rtree.search(rect, res, tot);
		tot_res_size += res.size();
		//tot += Search_test_time<features>(rtree, rect, res);
	}
	//tot /= querys;
	delete[] point;
	return tot / (tot_res_size / 100);
}

template<int Dimensions = 2>
void test_corectness_ratio(RTree<Dimensions>& rtree,
	unordered_map<string, Point<Dimensions>>& string2point,
	unordered_map<string, string>& point2string,
	int threshold) {
	int cnt = 0;
	double correct = 0;
	double recall = 0;
	for (auto p : string2point) {
		//cout << " " << p.first << endl;
		if ((++cnt) % 100 == 0)
			cout << cnt << endl;
		
		vector<Point<Dimensions>> res;
		string S(p.first);
		while (S.back() != '_')
			S.pop_back();

		//rtree.KNN(threshold, p.second, res);
		Rect<Dimensions> rect;
		rect.LeftBottom = p.second;
		rect.LeftBottom.move(-0.1);
		rect.RightTop = p.second;
		rect.RightTop.move(0.1);
		rtree.search(rect, res);

		int N = res.size();
		for (Point<Dimensions>& rp : res)
			rp.decrease(p.second);
		std::sort(res.begin(), res.end(), [](const Point<Dimensions>& p1, const Point<Dimensions>& p2) {
			return p1.rank2() < p2.rank2();
		});
		for (Point<Dimensions>& rp : res)
			rp.increase(p.second);

		int hit = 0;
		int miss = 0;
		for (int i = 0; i < 200 && i < N; ++i) {
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
		if (hit + miss > 0) {
			correct += (double)hit / (hit + miss);
			recall += (double)hit / cat[S];
		}
	}
	//cout << "total hit: " << hit << endl;
	//cout << "total miss: " << miss << endl;
	//cout << (double)hit / (double)(hit + miss) << endl;
	cout << "correct ratio: " << correct / string2point.size() << endl;
	cout << "recall ratio: " << recall / string2point.size() << endl;
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

int N[11] = { 500,1000,1500,2000,2500,3000,3500,4000,4500,5000,5500 };
int t[11][9];
bool v[10000];
/*
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
*/
template<int Dimensions = 2>
void problem_3(const string& filename, unordered_map<string, Point<Dimensions>>& string2point,
	unordered_map<string, string>& point2string, RTree<Dimensions>& rtree) {
	Point<Dimensions> p;
	Point<Dimensions>* point = new Point<Dimensions>[10000];
	string name;
	ifstream input(filename);
	cat.clear();
	if (input.is_open()) {
		for (int j = 0; j < 5613; ++j) {
			getline(input, name);
			cout << name << endl;

			string svec;
			getline(input, svec);
			p.fromString(svec);
			point[j].copy(p);

			point2string.insert(make_pair(p.toString(), name));
			string2point.insert(make_pair(name, p));
			rtree.Insert(p);

			while (!name.empty() && name.back() != '_')
				name.pop_back();
			cat[name]++;
		}
		input.close();
	}

	for (auto c : filename) {
		if (c == '.') break;
		cout << c;
	}
	cout << endl;
	test_corectness_ratio<Dimensions>(rtree, string2point, point2string, 10);
	cout << endl;

	delete[] point;
	system("pause");
}

void problem_1() {
	ofstream out("prob1.txt");
	for (int i = 0; i < 11; ++i) {
		t[i][0] = user_test<4>(N[i], 10);
		t[i][1] = user_test<6>(N[i], 10);
		t[i][2] = user_test<8>(N[i], 10);
		t[i][3] = user_test<10>(N[i], 10);
		t[i][4] = user_test<12>(N[i], 10);
		t[i][5] = user_test<14>(N[i], 10);
		t[i][6] = user_test<16>(N[i], 10);
		t[i][7] = user_test<18>(N[i], 10);
		t[i][8] = user_test<20>(N[i], 10);
	}
	for (int i = 0; i < 11; ++i)
		for (int j = 0; j < 9; ++j)
			out << N[i] << " " << (2*j+4) << " " << t[i][j] << endl;
	out.close();
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
	problem_1();

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

	
	//problem_3<D_gradient>(file_gradient, string2point_gradient, point2string_gradient, rtree_gradient);
	//problem_3<D_moment>(file_moment, string2point_moment, point2string_moment, rtree_moment);
	//problem_3<D_color>(file_color, string2point_color, point2string_color, rtree_color);
	//problem_3<D_gray>(file_gray, string2point_gray, point2string_gray, rtree_gray);
	//problem_3<D_lbp>(file_lbp, string2point_lbp, point2string_lbp, rtree_lbp);
	
	cout << "All finished!" << endl;
	//test_correctness_main();

	
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