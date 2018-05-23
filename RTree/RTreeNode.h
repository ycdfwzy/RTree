#pragma once

#include "Geometry.h"
#include <vector>
#include <iostream>

namespace MyRTree {

template<int Dimensions = 2>
class RTree;

template<int Dimensions = 2>
class RTreeNode {
public:
	RTreeNode(RTreeNode<Dimensions>* p_, RTree<Dimensions>* rt_)
		: p(p_), rt(rt_) {
		sons.clear();
		for (int i = 0; i < Dimensions; ++i) {
			MBR.LeftBottom.x[i] = Inf;
			MBR.RightTop.x[i] = -Inf;
		}
	}
	RTreeNode(const Rect<Dimensions>& MBR_, RTreeNode<Dimensions>* p_, RTree<Dimensions>* rt_)
		: MBR(MBR_), p(p_), rt(rt_) {
		sons.clear();
	}
	~RTreeNode() {
		while (!sons.empty()) {
			delete sons.back();
			sons.pop_back();
		}
	}

	void search(const Rect<Dimensions>&, std::vector<Rect<Dimensions>>&);
	void Insert(const Rect<Dimensions>&);
	void Delete(const Rect<Dimensions>&, std::vector<Rect<Dimensions>>&);
private:
	friend class RTree<Dimensions>;
	RTree<Dimensions>* rt;
	RTreeNode<Dimensions>* p;
	std::vector< RTreeNode<Dimensions>* > sons;
	Rect<Dimensions> MBR;

	int ChooseLeaf(const Rect<Dimensions>&);
	std::pair<int, int> find_seeds();
	void addnewnode(RTreeNode<Dimensions>* newnode);
	void SplitNode();
	bool find_allndoestodelete(const Rect<Dimensions>&, std::vector<RTreeNode<Dimensions>*>&);
	void UpdateMBR();
};

template<int Dimensions = 2>
void RTreeNode<Dimensions>::search(const Rect<Dimensions>& rec, std::vector<Rect<Dimensions>>& res) {
	if (sons.size() == 0) {
		if (Cover(rec, MBR))
			res.push_back(MBR);
		return;
	}
	for (RTreeNode<Dimensions>* son : sons) {
		if (Cross(son->MBR, rec)) {
			son->search(rec, res);
		}
	}
}

template<int Dimensions = 2>
void RTreeNode<Dimensions>::Insert(const Rect<Dimensions>& rec) {
	if (sons.size() == 0 || sons[0]->sons.size() == 0) {
		RTreeNode<Dimensions> *newnode = new RTreeNode<Dimensions>(rec, this, rt);
		addnewnode(newnode);

		if (sons.size() > rt->M)
			SplitNode();
		return;
	}
	int id = ChooseLeaf(rec);
	sons[id]->Insert(rec);
	
	//update MBR
	for (int i = 0; i < Dimensions; ++i) {
		MBR.LeftBottom.x[i] = MBR.LeftBottom.x[i] < rec.LeftBottom.x[i] ? MBR.LeftBottom.x[i] : rec.LeftBottom.x[i];
		MBR.RightTop.x[i] = MBR.RightTop.x[i] > rec.RightTop.x[i] ? MBR.RightTop.x[i] : rec.RightTop.x[i];
	}
	if (sons.size() > rt->M)
		SplitNode();
}

template<int Dimensions = 2>
void RTreeNode<Dimensions>::Delete(const Rect<Dimensions>& rec, std::vector<Rect<Dimensions>>& remains) {
	//delete from father
	int pN = p->sons.size();
	for (int i = 0; i < pN; ++i) {
		if (p->sons[i] == this) {
			p->sons.erase(p->sons.begin()+i);
			break;
		}
	}
	p->UpdateMBR();

	//find all remains
	std::vector<RTreeNode<Dimensions>*> que;
	que.clear();
	que.push_back(this);
	int l = 0, r = 0;
	while (l <= r) {
		RTreeNode<Dimensions>* x = que[l++];
		if (x->sons.empty()) {
			--rt->leaves;
			if (!Cover(rec, x->MBR))
				remains.push_back(x->MBR);
		}
		else
		{
			int xN = x->sons.size();
			for (int i = 0; i < xN; ++i) {
				que.push_back(x->sons[i]);
				++r;
			}
		}
	}
	que.clear();
}

template<int Dimensions = 2>
int RTreeNode<Dimensions>::ChooseLeaf(const Rect<Dimensions>& rec) {
	if (sons.size() == 0) return 0;
	int N = sons.size();
	
	double min_delta_size = Inf;
	int id = 0;

	for (int i = 0; i < N; ++i) {
		double d = 1;
		double origin_size = 1;
		for (int j = 0; j < Dimensions; ++j) {
			origin_size *= (sons[i]->MBR.RightTop.x[j] - sons[i]->MBR.LeftBottom.x[j]);

			int xjmin = (rec.LeftBottom.x[j] < sons[i]->MBR.LeftBottom.x[j] ? rec.LeftBottom.x[j] : sons[i]->MBR.LeftBottom.x[j]);
			int xjmax = (rec.RightTop.x[j] > sons[i]->MBR.RightTop.x[j] ? rec.RightTop.x[j] : sons[i]->MBR.RightTop.x[j]);
			d *= (xjmax - xjmin);
		}
		if (d - origin_size < min_delta_size) {
			id = i;
			min_delta_size = d - origin_size;
		}
	}
	return id;
}

template<int Dimensions = 2>
std::pair<int, int> RTreeNode<Dimensions>::find_seeds() {
	int N = sons.size();
	//double MBR_d = 1;
	//for (int i = 0; i < Dimensions; ++i)
	//	MBR_d *= (MBR.RightTop.x[i] - MBR.LeftBottom.x[i]);

	//naive method
	double max_delta = -Inf;
	std::pair<int, int> ret;
	for (int i = 0; i < N; ++i)
		for (int j = i + 1; j < N; ++j) {
			double d = 1;
			for (int l = 0; l < Dimensions; ++l) {
				double minx = Inf;
				double maxx = -Inf;
				for (int k = 0; k < N; ++k)
					if (i != k && j != k) {
						minx = minx > sons[k]->MBR.LeftBottom.x[l] ? sons[k]->MBR.LeftBottom.x[l] : minx;
						maxx = maxx > sons[k]->MBR.RightTop.x[l] ? maxx : sons[k]->MBR.RightTop.x[l];
					}
				d *= (maxx - minx);
			}
			if (d > max_delta) {
				max_delta = d;
				ret = std::make_pair(i, j);
			}
		}

	return ret;
}

template<int Dimensions = 2>
void RTreeNode<Dimensions>::addnewnode(RTreeNode<Dimensions>* newnode) {
	sons.push_back(newnode);
	newnode->p = this;
	// update MBR
	/*
	for (int i = 0; i < Dimensions; ++i) {
		MBR.LeftBottom.x[i] = Inf;
		MBR.RightTop.x[i] = -Inf;
	}
	int k = sons.size();
	for (int i = 0; i < k; ++i) {
		for (int j = 0; j < Dimensions; ++j) {
			MBR.LeftBottom.x[j] = MBR.LeftBottom.x[j] < sons[i]->MBR.LeftBottom.x[j] ? MBR.LeftBottom.x[j] : sons[i]->MBR.LeftBottom.x[j];
			MBR.RightTop.x[j] = MBR.RightTop.x[j] > sons[i]->MBR.RightTop.x[j] ? MBR.RightTop.x[j] : sons[i]->MBR.RightTop.x[j];
		}
	}
	*/
	for (int i = 0; i < Dimensions; ++i) {
		MBR.LeftBottom.x[i] = MBR.LeftBottom.x[i] < newnode->MBR.LeftBottom.x[i] ? MBR.LeftBottom.x[i] : newnode->MBR.LeftBottom.x[i];
		MBR.RightTop.x[i] = MBR.RightTop.x[i] > newnode->MBR.RightTop.x[i] ? MBR.RightTop.x[i] : newnode->MBR.RightTop.x[i];
	}
}

template<int Dimensions = 2>
void RTreeNode<Dimensions>::SplitNode() {
	RTreeNode<Dimensions> *newnode = new RTreeNode<Dimensions>(this->p, rt);

	std::pair<int, int> seeds = find_seeds();
	std::vector<RTreeNode<Dimensions>*> sons_tmp(this->sons);
	this->sons.clear();

	for (int i = 0; i < Dimensions; ++i) {
		this->MBR.LeftBottom.x[i] = Inf;
		this->MBR.RightTop.x[i] = -Inf;
	}

	int k = sons_tmp.size();
	for (int i = 0; i < k; ++i) {
		double d1 = 0, d2 = 0;
		for (int j = 0; j < Dimensions; ++j) {
			double t1 = (sons_tmp[seeds.first]->MBR.LeftBottom.x[j] + sons_tmp[seeds.first]->MBR.RightTop.x[j]) / 2;
			double t2 = (sons_tmp[seeds.second]->MBR.LeftBottom.x[j] + sons_tmp[seeds.second]->MBR.RightTop.x[j]) / 2;
			double t3 = (sons_tmp[i]->MBR.LeftBottom.x[j] + sons_tmp[i]->MBR.RightTop.x[j]) / 2;
			d1 += (t1 - t3)*(t1 - t3);
			d2 += (t2 - t3)*(t2 - t3);
		}

		if (i != seeds.second && (i == seeds.first || d1 < d2)) {
			this->addnewnode(sons_tmp[i]);
		}
		else
		{
			newnode->addnewnode(sons_tmp[i]);
		}
	}
	sons_tmp.clear();

	if (this->p == NULL) {
		RTreeNode<Dimensions> *newroot = new RTreeNode<Dimensions>(NULL, rt);
		this->rt->root = newroot;
		newroot->addnewnode(this);
		newroot->addnewnode(newnode);
		this->p = newnode->p = newroot;
	}
	else
	{
		
		this->p->addnewnode(newnode);
	}

}

template<int Dimensions = 2>
bool RTreeNode<Dimensions>::find_allndoestodelete(const Rect<Dimensions>& rec, std::vector<RTreeNode<Dimensions>*>& toDelete) {
	int tim = toDelete.size();
	if (sons.empty()) {
		if (Cover(rec, MBR)) {
			toDelete.push_back(this);
			return true;
		}
		return false;
	}
	int N = sons.size();
	int loss = 0;
	for (int i = 0; i < N; ++i) {
		bool v = sons[i]->find_allndoestodelete(rec, toDelete);
		if (v) ++loss;
	}

	if (this->p != NULL && loss > 0 && sons.size() - loss < rt->M / 2) {
		while (toDelete.size() > tim)
			toDelete.pop_back();
		toDelete.push_back(this);
		return true;
	}
	return false;
}

template<int Dimensions = 2>
void RTreeNode<Dimensions>::UpdateMBR() {
	for (int i = 0; i < Dimensions; ++i) {
		MBR.LeftBottom.x[i] = Inf;
		MBR.RightTop.x[i] = -Inf;
	}
	int k = sons.size();
	for (int i = 0; i < k; ++i) {
		for (int j = 0; j < Dimensions; ++j) {
			MBR.LeftBottom.x[j] = MBR.LeftBottom.x[j] < sons[i]->MBR.LeftBottom.x[j] ? MBR.LeftBottom.x[j] : sons[i]->MBR.LeftBottom.x[j];
			MBR.RightTop.x[j] = MBR.RightTop.x[j] > sons[i]->MBR.RightTop.x[j] ? MBR.RightTop.x[j] : sons[i]->MBR.RightTop.x[j];
		}
	}
}

}