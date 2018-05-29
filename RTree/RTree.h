#pragma once

#include "Geometry.h"
#include "RTreeNode.h"
#include <iostream>
#include <vector>
#include <algorithm>

namespace MyRTree {

enum Dis_Func {
	Euclidean,
	Manhattan,
	Chebyshev
};

template<int Dimensions = 2>
class RTree {
public:
	RTree(int M_) : M(M_) {
		root = new RTreeNode<Dimensions>(NULL, this);
		leaves = 0;
	}
	RTree(Point<Dimensions>* point, int N, int M_) : M(M_) {
		leaves = N;
		int depth = 0, NN = N;
		while (NN > M) {
			NN /= M;
			depth++;
		}
		root = BuildFromArray(point, N, NULL, 0, depth);
	}

	~RTree() {
		delete root;
	}

	void search(const Rect<Dimensions>&, std::vector<Rect<Dimensions>>&);
	void search(const Rect<Dimensions>&, std::vector<Point<Dimensions>>&);
	void search(const Rect<Dimensions>&, std::vector<Rect<Dimensions>>&, int&);
	void search(const Rect<Dimensions>&, std::vector<Point<Dimensions>>&, int&);
	void Insert(const Rect<Dimensions>&);
	void Insert(const Point<Dimensions>&);
	void Delete(const Rect<Dimensions>&);
	void KNN(int K, const Point<Dimensions>& p, std::vector<Point<Dimensions>>&);
	void BuildFromArray(Point<Dimensions>* point, int N) {
		delete root;
		leaves = N;
		int depth = 0, NN = N;
		while (NN > M) {
			NN /= M;
			depth++;
		}
		root = BuildFromArray(point, N, NULL, 0, depth);
	}
	RTreeNode<Dimensions>* BuildFromArray(Point<Dimensions>* point, int N, RTreeNode<Dimensions>* par, int d, int depth);

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
void RTree<Dimensions>::search(const Rect<Dimensions>& rec, std::vector<Rect<Dimensions>>& res, int& access_times) {
	access_times++;
	root->search(rec, res, access_times);
}

template<int Dimensions = 2>
void RTree<Dimensions>::search(const Rect<Dimensions>& rec, std::vector<Point<Dimensions>>& res, int& access_times) {
	std::vector<Rect<Dimensions>> tmp;
	tmp.clear();
	search(rec, tmp, access_times);
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
	double L = 0, R = 2;
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
		if (res.size() > 10 * K) {
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

template<int Dimensions>
RTreeNode<Dimensions>* RTree<Dimensions>::BuildFromArray(Point<Dimensions>* point, int N, RTreeNode<Dimensions>* par, int d, int depth) {
	RTreeNode<Dimensions>* p = new RTreeNode<Dimensions>(par, this);
	if (depth == 0) {
		for (int i = 0; i < N; ++i) {
			Rect<Dimensions> rec;
			rec.LeftBottom = point[i];
			rec.RightTop = point[i];
			RTreeNode<Dimensions> *newnode = new RTreeNode<Dimensions>(rec, p, this);
			p->addnewnode(newnode);
		}
		return p;
	}

	sort(point, point + N, [&](const Point<Dimensions> p1, const Point<Dimensions> p2) {
		return p1.x[d] < p2.x[d];
	});
	for (int i = 0, j = 0; i < M; ++i) {
		int k = (N - j) / (M - i);
		RTreeNode<Dimensions> *newnode = BuildFromArray(point + j, k, p, d + 1 == Dimensions ? 0 : d + 1, depth-1);
		j += k;
		p->addnewnode(newnode);
	}
	return p;
}

}
