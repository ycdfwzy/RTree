#pragma once

#include "Geometry.h"
#include "RTreeNode.h"
#include <iostream>
#include <vector>
#include <algorithm>

namespace MyRTree {

template<int Dimensions = 2>
class RTree {
public:
	RTree(int M_) : M(M_) {
		root = new RTreeNode<Dimensions>(NULL, this);
		leaves = 0;
		//std::cout << M << std::endl;
	}

	~RTree() {
		delete root;
	}

	void search(const Rect<Dimensions>&, std::vector<Rect<Dimensions>>&);
	void search(const Rect<Dimensions>&, std::vector<Point<Dimensions>>&);
	void Insert(const Rect<Dimensions>&);
	void Insert(const Point<Dimensions>&);
	void Delete(const Rect<Dimensions>&);
	void KNN(int K, const Point<Dimensions>& p, std::vector<Point<Dimensions>>&);

	int leaves;
private:
	friend class RTreeNode<Dimensions>;
	RTreeNode<Dimensions>* root;
	int M;
};

template<int Dimensions = 2>
void RTree<Dimensions>::search(const Rect<Dimensions>& rec, std::vector<Rect<Dimensions>>& res) {
	root->search(rec, res);
}

template<int Dimensions = 2>
void RTree<Dimensions>::search(const Rect<Dimensions>& rec, std::vector<Point<Dimensions>>& res) {
	std::vector<Rect<Dimensions>> tmp;
	tmp.clear();
	search(rec, tmp);
	for (Rect<Dimensions>& r : tmp) {
		res.push_back(r.LeftBottom);
	}
	tmp.clear();
}

template<int Dimensions = 2>
void RTree<Dimensions>::Insert(const Rect<Dimensions>& rec) {
	root->Insert(rec);
	++leaves;
}

template<int Dimensions = 2>
void RTree<Dimensions>::Insert(const Point<Dimensions>& p) {
	Rect<Dimensions> rec;
	rec.LeftBottom = p;
	rec.RightTop = p;
	this->Insert(rec);
}

template<int Dimensions = 2>
void RTree<Dimensions>::Delete(const Rect<Dimensions>& rec) {
	std::vector<RTreeNode<Dimensions>*> toDelete;
	root->find_allndoestodelete(rec, toDelete);
	std::vector<Rect<Dimensions>> remains;
	while (!toDelete.empty()) {
		RTreeNode<Dimensions>* q = toDelete.back();
		toDelete.pop_back();
		q->Delete(rec, remains);
		delete q;
	}

	while (!remains.empty()) {
		Rect<Dimensions> rect = remains.back();
		remains.pop_back();
		this->Insert(rect);
	}

	while (root->sons.size() == 1) {
		RTreeNode<Dimensions>* newroot = root->sons[0];
		newroot->p = NULL;
		root->sons.clear();
		delete root;
		root = newroot;
	}
	//toDelete.clear();
}

template<int Dimensions = 2>
void RTree<Dimensions>::KNN(int K, const Point<Dimensions>& p, std::vector<Point<Dimensions>>& res) {
	double L = 0, R = Inf;
	double mid = (L + R) / 2;
	Rect<Dimensions> rec;
	while (L < R) {
		rec.LeftBottom.copy(p);
		rec.LeftBottom.move(-mid);
		rec.RightTop.copy(p);
		rec.RightTop.move(mid);
		this->search(rec, res);
		if (res.size() < K) {
			L = mid;
		} else
		if (res.size() > 5 * K) {
			R = mid;
		}
		else
			break;
		mid = (L + R) / 2;
		res.clear();
	}
	for (Point<Dimensions>& rp : res)
		rp.decrease(p);
	std::sort(res.begin(), res.end(), [](const Point<Dimensions>& p1, const Point<Dimensions>& p2) {
		return p1.rank2() < p2.rank2();
	});
	while (res.size() > K)
		res.pop_back();
	for (Point<Dimensions>& rp : res)
		rp.increase(p);
}

}
