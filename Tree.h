#pragma once
#include "Component.h"
#include "Level.h"
#include "Data.h"
#include <vector>
#include <queue>
#include <string>

class Tree
{
public:
	int IOnum;
	vector<Level> ComponentTable;
	int max_level, limit_level;
	vector<bool> spatial_filter;

	Tree();
	void insert_data(Data d);
	void merge(int level);
	void flush(int level);
	void show(int level);
	void maintain(int level);
	pair<priority_queue<pair<double, Data>>, int> KNN1(Data point, int k);
	pair<priority_queue<pair<double, Data>>, int> KNN2(Data point, int k);
	pair<priority_queue<pair<double, Data>>, int> KNN3(Data point, int k);
	pair<priority_queue<pair<double, Data>>, pair<int, int>> memory_search(int x, int y, int k);
	priority_queue<pair<double, Data>> brute_force(Data point, int k);
	pair<priority_queue<pair<double, Data>>, int> RangeQuery(Data point, double r);

	void IOcheck();
};

