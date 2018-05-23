#pragma once
#include <iostream>

namespace MyRTree {

const double Inf = 1e10;

template<int Dimensions = 2>
class Point {
public:
	double x[Dimensions];
	void write() {
		for (int i = 0; i < Dimensions; ++i)
			std::cout << x[i] << " ";
		std::cout << std::endl;
	}
};

template<int Dimensions = 2>
class Rect {
public:
	Point<Dimensions> LeftBottom;
	Point<Dimensions> RightTop;
	void write() {
		std::cout << "LeftBottom: ";
		LeftBottom.write();
		std::cout << "RightTop:   ";
		RightTop.write();
	}
};

template<int Dimensions = 2>
bool Cover(const Rect<Dimensions> &r1, const Rect<Dimensions> &r2) { // judge if r1 covers r2
	for (int i = 0; i < Dimensions; ++i) {
		if (r1.LeftBottom.x[i] > r2.LeftBottom.x[i]) return false;
		if (r1.RightTop.x[i] < r2.RightTop.x[i]) return false;
	}
	return true;
}

template<int Dimensions = 2>
bool Cross(const Rect<Dimensions> &r1, const Rect<Dimensions> &r2) {
	if (Cover(r1, r2) || Cover(r2, r1)) return true;
	for (int i = 0; i < Dimensions; ++i) {
		if (r1.LeftBottom.x[i] >= r2.LeftBottom.x[i] && r1.LeftBottom.x[i] <= r2.RightTop.x[i])
			continue;
		if (r1.RightTop.x[i] >= r2.LeftBottom.x[i] && r1.RightTop.x[i] <= r2.RightTop.x[i])
			continue;
		if (r2.LeftBottom.x[i] >= r1.LeftBottom.x[i] && r2.LeftBottom.x[i] <= r1.RightTop.x[i])
			continue;
		if (r2.RightTop.x[i] >= r1.LeftBottom.x[i] && r2.RightTop.x[i] <= r1.RightTop.x[i])
			continue;
		return false;
	}
	return true;
}
}