#include "Level.h"
#include "Component.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <queue>
#include <functional>
#include <math.h>
#include <stack>
using namespace std;

bool comp(Component& a, Component& b) {
	int len;
	len = a.common_range;
	if (a.root->key[0].geohash.substr(0, len) == b.root->key[0].geohash.substr(0, len))
		return a.elementNum > b.elementNum;
	return a.root->key[0].geohash.substr(0, len) < b.root->key[0].geohash.substr(0, len);
}

Level::Level(int lev) {
	int size_arr;
	level = lev;

	if (level > 0) {
		size_arr = (int)pow(4, lev);
		quadrant_num.resize(size_arr, 0);
	}
	else if (level == 0) {
		size_arr = 1;
		quadrant_num.resize(size_arr, 0);
		Component_lists.push_back(Component(level));
	}
	return;
}

void Level::insert_component(Component c) {
	Component_lists.push_back(c);
	return;
}

int Level::merge_component() { // 2개의 컴포넌트 모든 node개수 더하고 merge해서 나온 총 노드의 개수를 세자
	Component s, t;
	vector<Component> temp;
	vector<int> delete_check;
	int s_idx, t_idx, cnt, len, ionum;
	string s_prefix, t_prefix;

	delete_check.resize(Component_lists.size(), 0), ionum = 0;
	sort(Component_lists.begin(), Component_lists.end(), comp); // geohash를 기준으로 오름차순으로 정렬, 만약 geohash가 같다면 elenum을 기준으로 내림차순정렬 O(nlogn)
	len = Component_lists[0].common_range;
	s = Component_lists[0], s_idx = 0, cnt = 0, s_prefix = Component_lists[0].root->key[0].geohash.substr(0, len);
	for (int i = 1; i < Component_lists.size(); i++) {
		t = Component_lists[i], t_idx = i;
		t_prefix = Component_lists[i].root->key[0].geohash.substr(0, len);
		
		if (s_prefix == t_prefix) {
			if (cnt == 0) ionum += s.nodeNum;
			ionum += s.merge(t);
			delete_check[s_idx] = 1, delete_check[t_idx] = 1;
			cnt++;
		}
		else {
			if (cnt != 0) {
				temp.push_back(s);
				cnt = 0;
			}
			s = t, s_idx = t_idx, s_prefix = t_prefix;
		}
	}
	if (cnt != 0) {
		temp.push_back(s);
		cnt = 0;
	}

	for (int i = 0; i < Component_lists.size(); i++) {
		if (delete_check[i] == 0) temp.push_back(Component_lists[i]);
	}
	Component_lists.clear();
	Component_lists = temp;
	temp.clear();

	return ionum;
}

int Level::two_to_int(string str) {
	int ret, i, len;
	ret = 0, len = str.length();

	for (i = len - 1; i >= 0; i--) {
		if (str[i] == '1') {
			if (len - i - 1 == 0)
				ret += 1;
			else
				ret += pow(2, len - i - 1);
		}
	}
	return ret;
}

pair<pair<vector<Component>, vector<int>>, int> Level::flush_component() {
	pair<pair<vector<Component>, vector<int>>, int> get_ret;
	vector<Component> N, temp, t;
	vector<int> arr, arr_two;
	string g_hash;
	int index, i, j, num, ionum;
	
	arr.resize(quadrant_num.size() * 4, 0), ionum = 0;
	for (i = 0; i < Component_lists.size(); i++) {
		if (Component_lists[i].nodeNum > Component_lists[i].limit_size) { // flush 될 조건
			get_ret = Component_lists[i].flush();
			t = get_ret.first.first, arr_two = get_ret.first.second, ionum += get_ret.second;
			for (j = 0; j < t.size(); j++) {
				N.push_back(t[j]);
			}
			g_hash = t[0].root->key[0].geohash.substr(0, 2 * level);
			g_hash += "00";
			index = two_to_int(g_hash);
			num = 0;
			for (j = 0; j < 4; j++) {
				arr[index + j] += arr_two[j];
				num += arr_two[j];
			}
			quadrant_num[index / 4] -= num;
		}
		else {
			temp.push_back(Component_lists[i]);
		}
	}
	Component_lists.clear();
	Component_lists = temp;
	temp.clear();

	return make_pair(make_pair(N, arr), ionum);
}

void Level::show() {
	int i, j, len;
	if (level != 0) {
		cout << "quadrant status\n";
		for (j = 0; j < quadrant_num.size(); j++) {
			cout << j << " : " << quadrant_num[j] << ", ";
		}
	}
	for (i = 0; i < Component_lists.size(); i++) {
		len = Component_lists[i].common_range;

		if (len > 0) {
			cout << "\nComponent " << i << "'s common geohash " << Component_lists[i].root->key[0].geohash.substr(0, len) << ", data object number : " << Component_lists[i].elementNum << "\n";
		}
		cout << "x range : " << Component_lists[i].x_min << "~" << Component_lists[i].x_max << ", y range : " << Component_lists[i].y_min << "~" << Component_lists[i].y_max << "\n";
		Component_lists[i].show();
	}

	return;
}

vector<pair<double, Component::Bplus_Tree*>> Level::range_query(int x, int y, double r, int startnum) {
	vector<pair<double, Component::Bplus_Tree*>> ret;
	int i;
	double dist;

	for (i = 0; i < Component_lists.size(); i++) {
		if (i <= startnum) continue;

		dist = Component_lists[i].range_query(x, y);
		if (r < dist) continue;
		if (Component_lists[i].filtering(x, y, r)) {
			ret.push_back(make_pair(dist, Component_lists[i].root));
		}
	}

	return ret;
}

priority_queue<pair<double, Data>> Level::brute(int x, int y, int k, priority_queue<pair<double, Data>> pq, int startnum) {
	priority_queue<pair<double, Data>> ret, t;
	int i;
	ret = pq;
	for (i = 0; i < Component_lists.size(); i++) {
		if (i <= startnum) continue;
		if(Component_lists[i].root != NULL)
			ret = Component_lists[i].brute(x, y, k, ret);
	}

	return ret;
}

