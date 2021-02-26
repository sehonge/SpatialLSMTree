#pragma once
#include "Component.h"
#include <vector>

class Level
{
public:
	vector<Component> Component_lists;
	vector<int> quadrant_num;
	int level;

	Level(int lev);
	void insert_component(Component c);
	int merge_component();
	pair<pair<vector<Component>, vector<int>>, int> flush_component();
	vector<pair<double, Component::Bplus_Tree*>> range_query(int x, int y, double r, int startnum);
	priority_queue<pair<double, Data>> brute(int x, int y, int k, priority_queue<pair<double, Data>> pq, int startnum);
	int two_to_int(string str);

	void show();
};
