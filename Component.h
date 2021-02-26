#pragma once
#include "Data.h"
#include <vector>
#include <queue>
#include <functional>
#include <string>
using namespace std;

class Component
{
public:
	struct Bplus_Tree {
		vector<Data> key;
		vector<Bplus_Tree*> child_ptr;
		bool leaf;
		int n;
		int xrange[2], yrange[2];
	};
	Bplus_Tree* root;
	int common_range, level, limit_size, x_min, x_max, y_min, y_max, elementNum, nodeNum;
	vector<vector<bool>> filter;
	string blockaddress;

	Component();
	Component(int lev);
	int two_to_int(string str);
	void makefilter(int x, int y);

	vector<int> find_range(string str, int range);
	pair<pair<vector<Component>, vector<int>>, int> flush();
	int merge(Component c);
	void make_empty();
	void show();
	bool operator<(const Component& c) const;
	double min_dist(int x, int y);
	priority_queue<pair<double, Data>> brute(int x, int y, int k, priority_queue<pair<double, Data>> pq);
	pair<priority_queue<pair<double, Data>>, int> memory_search(int x, int y, int k, pair<priority_queue<pair<double, Data>>, int> pq);
	double range_query(int x, int y);
	bool filtering(int x, int y, double r);

	Bplus_Tree* init();
	void deletetree(Bplus_Tree* cur);
	void insert(Data a);
	void insertInternal(Data a, Bplus_Tree* cur, Bplus_Tree* child);
	Bplus_Tree* findParent(Bplus_Tree* cur, Bplus_Tree* child);

};

