#pragma once

#include "Geometry.h"
#include "RTreeNode.h"
#include <iostream>
#include <vector>

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
	void Insert(const Rect<Dimensions>&);
	void Delete(const Rect<Dimensions>&);
private:
	friend class RTreeNode<Dimensions>;
	RTreeNode<Dimensions>* root;
	int M, leaves;
};

template<int Dimensions = 2>
void RTree<Dimensions>::search(const Rect<Dimensions>& rec, std::vector<Rect<Dimensions>>& res) {
	root->search(rec, res);
}

template<int Dimensions = 2>
void RTree<Dimensions>::Insert(const Rect<Dimensions>& rec) {
	root->Insert(rec);
	++leaves;
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

}
