#include "Tree.h"
#include "Level.h"
#include "Component.h"
#include "Data.h"
#include <iostream>
#include <vector>
#include <queue>
#include <math.h>
#include <string>
#include <functional>
#define map_row_size 64

Tree::Tree() {
	IOnum = 0, max_level = -1, limit_level = 5;
	spatial_filter.resize(map_row_size * map_row_size, false);
	return;
}

void Tree::maintain(int level) {
	int i;
	if (level == limit_level) {
		merge(level);
		return;
	}

	merge(level);
	//show(1);
	flush(level);
	//show(1);
	if (max_level == level) return;
	for (i = 0; i < ComponentTable[level + 1].quadrant_num.size(); i++) {
		if (ComponentTable[level + 1].quadrant_num[i] > ComponentTable[0].Component_lists[0].limit_size) {
			maintain(level + 1);
			break;
		}
	}
	
	return;
}

void Tree::insert_data(Data d) {
	if (max_level == -1) {
		ComponentTable.push_back(Level(++max_level));
	}
	ComponentTable[0].Component_lists[0].insert(d);

	if (ComponentTable[0].Component_lists[0].nodeNum >= ComponentTable[0].Component_lists[0].limit_size) {
		pair<pair<vector<Component>, vector<int>>, int> ret;
		vector<Component> temp;
		vector<int> arr;
		int i;

		ret = ComponentTable[0].Component_lists[0].flush();
		temp = ret.first.first, arr = ret.first.second, IOnum += ret.second;

		if (max_level == 0) ComponentTable.push_back(Level(++max_level));
		for (i = 0; i < temp.size(); i++) 
			ComponentTable[1].insert_component(temp[i]);
		for (i = 0; i < 4; i++) 
			ComponentTable[1].quadrant_num[i] += arr[i];
				
		int maxsize = ComponentTable[0].Component_lists[0].limit_size;
		for (i = 0; i < 4; i++) {
			if (ComponentTable[1].quadrant_num[i] >= maxsize) {
				maintain(1);
				break;
			}
		}		
		ComponentTable[0].Component_lists[0].make_empty();
		ComponentTable[0].Component_lists[0].nodeNum = 0;
		ComponentTable[0].Component_lists[0].elementNum = 0;
		for (i = 0; i < 64; i++) {
			for (int j = 0; j < 64; j++) {
				ComponentTable[0].Component_lists[0].filter[i][j] = false;
			}
		}
	}
	return;
}

void Tree::merge(int level) {
	if (level > max_level) {
		cout << "Error, level " << level << " does not exist\n";
		return;
	}
	IOnum += ComponentTable[level].merge_component();

	return;
}

void Tree::flush(int level) {
	if (level > max_level) {
		cout << "Error, level " << level << " does not exist\n";
		return;
	}
	pair<pair<vector<Component>, vector<int>>, int> ret;
	vector<Component> temp;
	vector<int> arr;

	ret = ComponentTable[level].flush_component();
	temp = ret.first.first, arr = ret.first.second, IOnum += ret.second;
	if (temp.size() == 0) {
		//cout << "There is nothing to flush\n";
		return;
	}
	for (int i = 0; i < temp.size(); i++) {
		if (max_level == level) ComponentTable.push_back(Level(++max_level));
		ComponentTable[level + 1].insert_component(temp[i]);
	}
	for (int i = 0; i < arr.size(); i++) {
		ComponentTable[level + 1].quadrant_num[i] += arr[i];
	}

	return;
}

void Tree::show(int level) {
	if (level > max_level) {
		cout << "level " << level << " does not exist!\n";
		return;
	}
	ComponentTable[level].show();
	return;
}

pair<priority_queue<pair<double, Data>>, pair<int, int>> Tree::memory_search(int x, int y, int k) {
	bool flag = false;
	int i, j;
	pair<int, int> ret_p;
	pair<priority_queue<pair<double, Data>>, int> ret;
	if(ComponentTable[0].Component_lists[0].root != NULL)
		ret = ComponentTable[0].Component_lists[0].memory_search(x, y, k, ret);
	ret_p.first = 0, ret_p.second = 0, ret.second = 0;

	if (ret.first.size() < k) {
		for (i = 1; i < ComponentTable.size(); i++) {
			for (j = 0; j < ComponentTable[i].Component_lists.size(); j++) {
				ret = ComponentTable[i].Component_lists[j].memory_search(x, y, k, ret);
				if (ret.first.size() >= k) {
					flag = true;
					ret_p.first = i, ret_p.second = j;
					break;
				}
			}
			if (flag == true) break;
		}
	}
	IOnum = ret.second;
	
	return make_pair(ret.first, ret_p);
}

pair<priority_queue<pair<double, Data>>, int> Tree:: RangeQuery(Data point, double r) {
	pair<priority_queue<pair<double, Data>>, int> ret;
	double dist;
	int q_x, q_y, i, j;

	IOnum = 0;
	q_x = point.x, q_y = point.y;
	vector<pair<double, Component::Bplus_Tree*>> temp;
	priority_queue<Component::Bplus_Tree*> Q;

	for (i = 0; i <= max_level; i++) {
		temp = ComponentTable[i].range_query(q_x, q_y, r, -1);
		if (temp.empty()) continue;
		for (j = 0; j < temp.size(); j++) {
			Q.push(temp[j].second);
		}
		while (!Q.empty()) {
			Component::Bplus_Tree* cur;

			cur = Q.top();
			Q.pop();

			while (cur->leaf == false) {
				if (i != 0) IOnum++;
				cur = cur->child_ptr[0];
			}
			while (1) {
				if(i != 0) IOnum++;
				for (j = 0; j < cur->n; j++) {
					dist = cur->key[j].dist_to_point(q_x, q_y);
					if (dist <= r)
						ret.first.push(make_pair(dist, cur->key[j]));
				}
				if (cur->child_ptr[cur->n] == NULL) break;
				cur = cur->child_ptr[cur->n];
			}
		}
		IOnum--; // root count한거 1 빼야함
	}
	ret.second = IOnum;

	return ret;
}

void Tree::IOcheck() {
	IOnum++;
	return;
}

pair<priority_queue<pair<double, Data>>, int> Tree::KNN1(Data point, int k) { // Data 대신에 x좌표, y좌표, k값을 파라미터로 보내자
	int q_x, q_y, i, j;
	double dist, r;
	string min_geohash, max_geohash;	
	priority_queue<pair<double, Component::Bplus_Tree*>, vector<pair<double, Component::Bplus_Tree*>>, greater<pair<double, Component::Bplus_Tree*>>> Q;
	pair< priority_queue<pair<double, Data>>, pair<int, int>> t_ret;
	pair<int, int> start_n;
	priority_queue<pair<double, Data>> ret; // memory search 하고 max heap구조, ret할 pq는 min heap구조를 가지고 있어야함........
	q_x = point.x, q_y = point.y;
	
	vector<pair<double, Component::Bplus_Tree*>> temp;
	
	IOnum = 0;
	t_ret = memory_search(q_x, q_y, k); // memory search 이후의 녀석들부터 검사해야함.... mem_comp가 비었을경우 disk까지 내려가서 knn후보군을 찾아왔을수있음
	ret = t_ret.first, start_n = t_ret.second;
	r = ret.top().first;

	for (i = 1; i <= max_level; i++) {
		if (i < start_n.first) continue;
		else if(i == start_n.first)
			temp = ComponentTable[i].range_query(q_x, q_y, r, start_n.second);
		else if(i > start_n.first)
			temp = ComponentTable[i].range_query(q_x, q_y, r, -1);
		
		for (j = 0; j < temp.size(); j++) {
			Q.push(temp[j]);
		}
	}
	
	while (!Q.empty()) {
		double now_dist;
		Component::Bplus_Tree* cur;
		now_dist = Q.top().first, cur = Q.top().second;
		Q.pop();

		if (r < now_dist) break;
		if (cur->leaf == true) {
			for (i = 0; i < cur->n; i++) {
				dist = cur->key[i].dist_to_point(q_x, q_y);
				if (ret.top().first > dist) {
					ret.pop();
					ret.push(make_pair(dist, cur->key[i]));
					r = ret.top().first;
				}
			}			
		}
		else {
			for (i = 0; i <= cur->n; i++) {
				Q.push(make_pair(now_dist, cur->child_ptr[i]));
				IOcheck();
			}
		}		
	}	
	cout << "I/O check : " << IOnum << "\n";

	return make_pair(ret, IOnum);
}

pair<priority_queue<pair<double, Data>>, int> Tree::KNN2(Data point, int k) {
	int q_x, q_y, i, j;
	double dist, r;
	string min_geohash, max_geohash;
	priority_queue<pair<double, Component::Bplus_Tree *>, vector<pair<double, Component::Bplus_Tree*>>, greater<pair<double, Component::Bplus_Tree*>>> Q;
	priority_queue<pair<double, Data>> ret;
	pair< priority_queue<pair<double, Data>>, pair<int, int>> t_ret;
	pair<int, int> start_n;
	q_x = point.x, q_y = point.y;

	vector<pair<double, Component::Bplus_Tree *>> temp;

	IOnum = 0;
	t_ret = memory_search(q_x, q_y, k);
	ret = t_ret.first, start_n = t_ret.second;
	r = ret.top().first;

	for (i = 1; i <= max_level; i++) {
		if (i < start_n.first) continue;
		else if (i == start_n.first)
			temp = ComponentTable[i].range_query(q_x, q_y, r, start_n.second);
		else if (i > start_n.first)
			temp = ComponentTable[i].range_query(q_x, q_y, r, -1);

		for (j = 0; j < temp.size(); j++) {
			Q.push(temp[j]);
		}

		while (!Q.empty()) {
			double now_dist;
			Component::Bplus_Tree* cur;
			now_dist = Q.top().first, cur = Q.top().second;
			Q.pop();

			if (r < now_dist) break;
			if (cur->leaf == true) {
				for(j = 0; j < cur->n; j++) {
					dist = cur->key[j].dist_to_point(q_x, q_y);
					if (ret.top().first > dist) {
						ret.pop();
						ret.push(make_pair(dist, cur->key[j]));
						r = ret.top().first;
					}
				}				
			}
			else {
				for (j = 0; j <= cur->n; j++) {
					Q.push(make_pair(now_dist, cur->child_ptr[j]));
					IOcheck();
				}
			}
		}
	}
	cout << "I/O check : " << IOnum << "\n";

	return make_pair(ret, IOnum);
}

pair<priority_queue<pair<double, Data>>, int> Tree::KNN3(Data point, int k) {
	int q_x, q_y, i, j;
	double dist, r;
	string min_geohash, max_geohash;
	priority_queue<pair<double, Component::Bplus_Tree *>, vector<pair<double, Component::Bplus_Tree *>>, greater<pair<double, Component::Bplus_Tree *>>> Q, N;
	priority_queue<pair<double, Data>> ret;
	pair< priority_queue<pair<double, Data>>, pair<int, int>> t_ret;
	pair<int, int> start_n;
	q_x = point.x, q_y = point.y;

	vector<pair<double, Component::Bplus_Tree *>> temp;

	IOnum = 0;
	t_ret = memory_search(q_x, q_y, k);
	ret = t_ret.first, start_n = t_ret.second;
	r = ret.top().first;

	for (i = 1; i <= max_level; i++) {
		if (i < start_n.first) continue;
		else if (i == start_n.first)
			temp = ComponentTable[i].range_query(q_x, q_y, r, start_n.second);
		else if (i > start_n.first)
			temp = ComponentTable[i].range_query(q_x, q_y, r, -1);

		for (j = 0; j < temp.size(); j++) {
			Q.push(temp[j]);
		}
	}

	while (!Q.empty()) {
		double now_dist;
		Component::Bplus_Tree* cur;
		now_dist = Q.top().first, cur = Q.top().second;
		Q.pop();

		if (r < now_dist) break;
		if (cur->leaf == true) {
			for (i = 0; i < cur->n; i++) {
				dist = cur->key[i].dist_to_point(q_x, q_y);
				if (ret.top().first > dist) {
					ret.pop();
					ret.push(make_pair(dist, cur->key[i]));
					r = ret.top().first;
				}
			}			
		}
		else {
			for (i = 0; i <= cur->n; i++) {
				N.push(make_pair(now_dist, cur->child_ptr[i]));
				IOcheck();
			}
		}
		while (!N.empty()) {
			double node_dist;
			Component::Bplus_Tree* node; 
			node_dist = N.top().first, node = N.top().second;
			N.pop();

			if (r < node_dist) break;
			if (node->leaf == true) {
				for (i = 0; i < node->n; i++) {
					dist = node->key[i].dist_to_point(q_x, q_y);
					if (ret.top().first > dist) {
						ret.pop();
						ret.push(make_pair(dist, node->key[i]));
					}
				}
				r = ret.top().first;
			}
			else {
				for (i = 0; i <= node->n; i++) {
					N.push(make_pair(node_dist, node->child_ptr[i]));
					IOcheck();
				}
			}
		}
	}
	cout << "I/O check : " << IOnum << "\n";
	
	return make_pair(ret, IOnum);
}

priority_queue<pair<double, Data>> Tree::brute_force(Data point, int k) {
	int q_x, q_y, i;
	priority_queue<pair<double, Data>> ret, t;
	pair< priority_queue<pair<double, Data>>, pair<int, int>> t_ret;
	pair<int, int> start_n;
	q_x = point.x, q_y = point.y;

	t_ret = memory_search(q_x, q_y, k);
	ret = t_ret.first, start_n = t_ret.second;

	for (i = 1; i <= max_level; i++) {
		if (i < start_n.first) continue;
		else if (i == start_n.first)
			ret = ComponentTable[i].brute(q_x, q_y, k, ret, start_n.second);
		else if (i > start_n.first)
			ret = ComponentTable[i].brute(q_x, q_y, k, ret, -1);
	}
	
	return ret;
}

