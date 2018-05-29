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

ofstream prob3out("prob3.txt");

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
int user_test(int N, int M, bool flag = true) { // true: build by insertion    false: build by array
	Point<features> p;
	Rect<features> rect;
	RTree<features> rtree(M);
	
	Point<features> *point = new Point<features>[5613];
	ifstream input("lbp.txt");
	for (int i = 0; i < 5613; ++i) {
		string name;
		getline(input, name);

		string svec;
		getline(input, svec);
		p.fromString(svec);

		point[i] = p;
	}
	random_shuffle(point, point+5613);

	if (flag) {
		for (int i = 0; i < N; ++i) {
			rect.LeftBottom = point[i];
			rect.RightTop = point[i];
			rtree.Insert(rect);
		}
	}
	else
	{
		rtree.BuildFromArray(point, N);
	}
	input.close();
	
	//cout << "OK" << endl;

	int tot = 0;
	int tot_res_size = 0;
	vector<Rect<features>> res;
	for (int i = 0; i < querys; ++i) {
		rect.LeftBottom = point[rand() % N];
		rect.RightTop = rect.LeftBottom;
		/*
		for (int j = 0; j < 10; ++j) {
			int id = rand() % N;
			for (int k = 0; k < features; ++k) {
				rect.LeftBottom.x[k] = rect.LeftBottom.x[k] < point[id].x[k] ? rect.LeftBottom.x[k] : point[id].x[k];
				rect.RightTop.x[k] = rect.RightTop.x[k] > point[id].x[k] ? rect.RightTop.x[k] : point[id].x[k];
			}
		}
		*/
		rect.LeftBottom.move(-0.02);
		rect.RightTop.move(0.02);

		res.clear();
		rtree.search(rect, res, tot);
		tot_res_size += res.size();
	}
	delete[] point;
	return tot / (tot_res_size / 100);
}

template<int Dimensions = 2>
void test_corectness_ratio(RTree<Dimensions>& rtree,
	unordered_map<string, Point<Dimensions>>& string2point,
	unordered_map<string, string>& point2string,
	Dis_Func dis_func) {
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

		for (Point<Dimensions>& rp : res)
			rp.decrease(p.second);
		if (dis_func == Dis_Func::Euclidean) {
			std::sort(res.begin(), res.end(), [](const Point<Dimensions>& p1, const Point<Dimensions>& p2) {
				return p1.rank2() < p2.rank2();
			});
		} else
		if (dis_func == Dis_Func::Manhattan) {
			std::sort(res.begin(), res.end(), [](const Point<Dimensions>& p1, const Point<Dimensions>& p2) {
				return p1.rank1() < p2.rank1();
			});
		} else
		if (dis_func == Dis_Func::Chebyshev) {
			std::sort(res.begin(), res.end(), [](const Point<Dimensions>& p1, const Point<Dimensions>& p2) {
				return p1.max_d() < p2.max_d();
			});
		}
		for (Point<Dimensions>& rp : res)
			rp.increase(p.second);

		int N = res.size();
		int hit = 0;
		int miss = 0;
		for (int i = 0; i < 200 && i < N; ++i) {
		//for (auto s : res) {
			Point<Dimensions>& s = res[i];
			string pat(point2string[s.toString()]);
			while (!pat.empty() && pat.back() != '_')
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
	prob3out << "correct ratio: " << correct / string2point.size() << endl;
	prob3out << "recall ratio: " << recall / string2point.size() << endl;
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
	unordered_map<string, string>& point2string,
	RTree<Dimensions>& rtree,
	Dis_Func dis_func = Dis_Func::Euclidean) {
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
		//rtree.BuildFromArray(point, 5613);
		input.close();
	}

	for (auto c : filename) {
		if (c == '.') break;
		prob3out << c;
	}
	if (dis_func == Dis_Func::Euclidean) {
		prob3out << endl << "Euclidean" << endl;
		test_corectness_ratio<Dimensions>(rtree, string2point, point2string, Dis_Func::Euclidean);
	} else
	if (dis_func == Dis_Func::Manhattan) {
		prob3out << endl << "Manhattan" << endl;
		test_corectness_ratio<Dimensions>(rtree, string2point, point2string, Dis_Func::Manhattan);
	} else
	if (dis_func == Dis_Func::Chebyshev) {
		prob3out << endl << "Chebyshev" << endl;
		test_corectness_ratio<Dimensions>(rtree, string2point, point2string, Dis_Func::Chebyshev);
	}
	prob3out << endl;

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
	for (int j = 0; j < 9; ++j) {
		for (int i = 1; i < 11; ++i)
			out << N[i] << " " << (2*j+4) << " " << t[i][j] << endl;
			//out << t[i][j] << endl;
		out << endl;
	}
	out.close();
}

void buildfromarray() {
	ofstream out("buildfromarray.txt");
	for (int i = 0; i < 11; ++i) {
		t[i][0] = user_test<4>(N[i], 10, false);
		t[i][1] = user_test<6>(N[i], 10, false);
		t[i][2] = user_test<8>(N[i], 10, false);
		t[i][3] = user_test<10>(N[i], 10, false);
		t[i][4] = user_test<12>(N[i], 10, false);
		t[i][5] = user_test<14>(N[i], 10, false);
		t[i][6] = user_test<16>(N[i], 10, false);
		t[i][7] = user_test<18>(N[i], 10, false);
		t[i][8] = user_test<20>(N[i], 10, false);
	}
	for (int j = 0; j < 9; ++j) {
		for (int i = 1; i < 11; ++i)
			out << N[i] << " " << (2 * j + 4) << " " << t[i][j] << endl;
			//out << t[i][j] << endl;
		out << endl;
	}
	out.close();
}

template<int Dimensions = 2>
void query(const string& datafile, const string& queryfile,
	unordered_map<string, Point<Dimensions>>& string2point,
	unordered_map<string, string>& point2string,
	RTree<Dimensions>& rtree,
	Dis_Func dis_func = Dis_Func::Euclidean) {

	Point<Dimensions> p;
	Point<Dimensions>* point = new Point<Dimensions>[10000];
	string name;
	ifstream input(datafile);
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
	else
	{
		cout << "Can't open " << datafile << endl;
		delete[] point;
		return;
	}

	ofstream out("result.txt");
	input.open(queryfile);
	if (input.is_open()) {
		while (!input.eof()) {
			getline(input, name);
			if (name.empty()) break;
			cout << name << endl;
			out << name << endl;
			string svec;
			getline(input, svec);
			p.fromString(svec);

			vector<Point<Dimensions>> res;
			Rect<Dimensions> rect;
			rect.LeftBottom = p;
			rect.LeftBottom.move(-0.02);
			rect.RightTop = p;
			rect.RightTop.move(0.02);
			rtree.search(rect, res);

			for (Point<Dimensions>& rp : res)
				rp.decrease(p);
			if (dis_func == Dis_Func::Euclidean) {
				std::sort(res.begin(), res.end(), [](const Point<Dimensions>& p1, const Point<Dimensions>& p2) {
					return p1.rank2() < p2.rank2();
				});
			}
			else
			if (dis_func == Dis_Func::Manhattan) {
				std::sort(res.begin(), res.end(), [](const Point<Dimensions>& p1, const Point<Dimensions>& p2) {
					return p1.rank1() < p2.rank1();
				});
			}
			else
			if (dis_func == Dis_Func::Chebyshev) {
				std::sort(res.begin(), res.end(), [](const Point<Dimensions>& p1, const Point<Dimensions>& p2) {
					return p1.max_d() < p2.max_d();
				});
			}
			for (Point<Dimensions>& rp : res)
				rp.increase(p);

			for (int i = 0; i < 10 && i < res.size(); ++i)
				out << "	"<< point2string[res[i].toString()] << endl;
		}
		input.close();
		out.close();
	}
	else
	{
		cout << "Can't open " << queryfile << endl;
		delete[] point;
		out.close();
		return;
	}

	delete[] point;
	system("pause");
}

int main(int argc, char** argv) {
	//srand(time(0));
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
	
	if (argc == 1) {
		cout << "too few arguments!" << endl;
		return 0;
	}

	if (strcmp(argv[1], "prob1") == 0) {
		if (argc > 2) {
			cout << "too many arguments!" << endl;
			return 0;
		}
		cout << "Please wait..." << endl;
		problem_1();
	} else
	if (strcmp(argv[1], "prob3") == 0) {
		if (argc < 3) {
			cout << "too few arguments!" << endl;
			return 0;
		}
		if (argc > 3) {
			cout << "too many arguments!" << endl;
			return 0;
		}

		if (strcmp(argv[2], "gradient") == 0)
			problem_3<D_gradient>(file_gradient, string2point_gradient, point2string_gradient, rtree_gradient);
		else
		if (strcmp(argv[2], "moment") == 0)
			problem_3<D_moment>(file_moment, string2point_moment, point2string_moment, rtree_moment);
		else
		if (strcmp(argv[2], "color") == 0)
			problem_3<D_color>(file_color, string2point_color, point2string_color, rtree_color);
		else
		if (strcmp(argv[2], "gray") == 0)
			problem_3<D_gray>(file_gray, string2point_gray, point2string_gray, rtree_gray);
		else
		if (strcmp(argv[2], "lbp") == 0)
			problem_3<D_lbp>(file_lbp, string2point_lbp, point2string_lbp, rtree_lbp);
		else
		{
			cout << "No found \"" << argv[2] << "\" " << endl;
			return 0;
		}
	}
	else
	if (strcmp(argv[1], "opt2") == 0) {
		if (argc > 1) {
			cout << "too many arguments!" << endl;
			return 0;
		}
		cout << "Please wait..." << endl;
		buildfromarray();
	}
	else
	if (strcmp(argv[1], "opt3") == 0) {
		if (argc < 4) {
			cout << "too few arguments!" << endl;
			return 0;
		}
		if (argc > 4) {
			cout << "too many arguments!" << endl;
			return 0;
		}

		Dis_Func foo;
		if (strcmp(argv[3], "Euclidean") == 0)
			foo = Dis_Func::Euclidean;
		else
		if (strcmp(argv[3], "Manhattan") == 0)
			foo = Dis_Func::Manhattan;
		else
		if (strcmp(argv[3], "Chebyshev") == 0)
			foo = Dis_Func::Chebyshev;
		else
		{
			cout << "Not found \"" << argv[3] << "\" " << endl;
			return 0;
		}

		if (strcmp(argv[2], "gradient") == 0)
			problem_3<D_gradient>(file_gradient, string2point_gradient, point2string_gradient, rtree_gradient, foo);
		else
			if (strcmp(argv[2], "moment") == 0)
				problem_3<D_moment>(file_moment, string2point_moment, point2string_moment, rtree_moment, foo);
			else
				if (strcmp(argv[2], "color") == 0)
					problem_3<D_color>(file_color, string2point_color, point2string_color, rtree_color, foo);
				else
					if (strcmp(argv[2], "gray") == 0)
						problem_3<D_gray>(file_gray, string2point_gray, point2string_gray, rtree_gray, foo);
					else
						if (strcmp(argv[2], "lbp") == 0)
							problem_3<D_lbp>(file_lbp, string2point_lbp, point2string_lbp, rtree_lbp, foo);
						else
						{
							cout << "No found \"" << argv[2] << "\" " << endl;
							return 0;
						}
	}
	else
	if (strcmp(argv[1], "query") == 0) {
		//cout << argc << endl;
		if (argc < 5) {
			cout << "too few arguments!" << endl;
			return 0;
		}
		if (argc > 5) {
			cout << "too many arguments!" << endl;
			return 0;
		}

		Dis_Func foo;
		if (strcmp(argv[3], "Euclidean") == 0)
			foo = Dis_Func::Euclidean;
		else
		if (strcmp(argv[3], "Manhattan") == 0)
			foo = Dis_Func::Manhattan;
		else
		if (strcmp(argv[3], "Chebyshev") == 0)
			foo = Dis_Func::Chebyshev;
		else
		{
			cout << "Not found \"" << argv[3] << "\" " << endl;
			return 0;
		}

		if (strcmp(argv[2], "gradient") == 0)
			query<D_gradient>(file_gradient, string(argv[4]), string2point_gradient, point2string_gradient, rtree_gradient, foo);
		else
			if (strcmp(argv[2], "moment") == 0)
				query<D_moment>(file_moment, string(argv[4]), string2point_moment, point2string_moment, rtree_moment, foo);
			else
				if (strcmp(argv[2], "color") == 0)
					query<D_color>(file_color, string(argv[4]), string2point_color, point2string_color, rtree_color, foo);
				else
					if (strcmp(argv[2], "gray") == 0)
						query<D_gray>(file_gray, string(argv[4]), string2point_gray, point2string_gray, rtree_gray, foo);
					else
						if (strcmp(argv[2], "lbp") == 0)
							query<D_lbp>(file_lbp, string(argv[4]), string2point_lbp, point2string_lbp, rtree_lbp, foo);
						else
						{
							cout << "No found \"" << argv[2] << "\" " << endl;
							return 0;
						}

	}
	else
	{
		cout << "No found \"" << argv[1] << "\" " << endl;
		return 0;
	}

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
	problem_3<D_gradient>(file_gradient, string2point_gradient, point2string_gradient, rtree_gradient);
	problem_3<D_moment>(file_moment, string2point_moment, point2string_moment, rtree_moment);
	problem_3<D_color>(file_color, string2point_color, point2string_color, rtree_color);
	problem_3<D_gray>(file_gray, string2point_gray, point2string_gray, rtree_gray);
	problem_3<D_lbp>(file_lbp, string2point_lbp, point2string_lbp, rtree_lbp);
	*/
	
	//buildfromarray();
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

	prob3out.close();
	return 0;
}