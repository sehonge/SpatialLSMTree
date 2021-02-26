#include "Component.h"
#include "Data.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <queue>
#include <functional>
#include <cmath>
using namespace std;

#define max_len 14
#define COMPONENTLIMIT 512 // 128MB = 32768, 16MB = 4096, 2MB = 512 노드개수
#define MAX_N 135 // bplus tree의 node의 key값 size

bool comp(Data &a, Data &b) {
	return a.geohash < b.geohash;
}

Component::Component(){}

Component::Component(int lev) {
	elementNum = 0, root = NULL, blockaddress = "";
	level = lev, limit_size = COMPONENTLIMIT, x_min = 0, x_max = 16383, y_min = 0, y_max = 16384, nodeNum = 0;
	filter.resize(64);
	for (int i = 0; i < 64; i++) filter[i].resize(64, false);
	common_range = 2 * (level);

	return;
}

bool Component::operator<(const Component& c) const {
	return root->key[0].geohash < c.root->key[0].geohash;
}

void Component::makefilter(int x, int y) {
	int i_x, i_y;

	i_x = x / 256, i_y = y / 256;
	if (i_x > 63) i_x = 63;
	if (i_y > 63) i_y = 63;
	filter[i_x][i_y] = true;

	return;
}

int Component::two_to_int(string str) {
	int ret, i, len;
	ret = 0, len = str.length();

	for (i = max_len; i >= 0; i--) {
		if (i <= len - 1) {
			if (str[i] == '1') {
				ret += (int)pow(2, max_len - i - 1);
			}
		}
	}
	return ret;
}

vector<int> Component::find_range(string str, int range) {
	vector<int> ret;
	int i;
	string x_min, x_max, y_min, y_max;

	for (i = 0; i < range + 2; i++) { // 다음 level에 집어넣을 component이기 때문에 common_range + 2
		if (i % 2 == 0) x_min.push_back(str[i]), x_max.push_back(str[i]);
		else if (i % 2 == 1) y_min.push_back(str[i]), y_max.push_back(str[i]);
	}
	for (i = range + 1; i < max_len; i++) { // 각 x,y좌표의 geohash는 1자리씩 common ragne에 추가되었으므로 common_range + 1 부터 추가.
		x_min.push_back('0'), x_max.push_back('1'), y_min.push_back('0'), y_max.push_back('1');
	}
	ret.push_back(two_to_int(x_min));
	ret.push_back(two_to_int(x_max));
	ret.push_back(two_to_int(y_min));
	ret.push_back(two_to_int(y_max));

	return ret;
}

pair<pair<vector<Component>, vector<int>>, int> Component::flush() {
	Component::Bplus_Tree* node;
	int i, cnt;
	vector<Component> t, ret;
	vector<int> arr, range;
	string geohashnum;
	char dx[4] = { '0', '0', '1', '1' };
	char dy[4] = { '0', '1', '0', '1' };

	node = root, cnt = 0;
	while (node->leaf == false) {
		node = node->child_ptr[0];
		if (node->leaf == false) cnt++;
	}
	t.resize(4), arr.resize(4, 0), range.resize(4);

	for (i = 0; i < 4; i++) {
		t[i] = Component(level + 1);
		geohashnum = node->key[0].geohash.substr(0, common_range);
		geohashnum.push_back(dx[i]), geohashnum.push_back(dy[i]);
		range = find_range(geohashnum, common_range);
		t[i].x_min = range[0], t[i].x_max = range[1], t[i].y_min = range[2], t[i].y_max = range[3];
		range.clear();
	}
	
	while (1) {
		for (i = 0; i < node->n; i++) {
			geohashnum = node->key[i].geohash.substr(common_range, 2);
			if (geohashnum == "00") {
				t[0].insert(node->key[i]);
			}
			else if (geohashnum == "01") {
				t[1].insert(node->key[i]);
			}
			else if (geohashnum == "10") {
				t[2].insert(node->key[i]);
			}
			else if (geohashnum == "11") {
				t[3].insert(node->key[i]);
			}
		}
		cnt++;
		if (node->child_ptr[node->n] == NULL) break;
		else {
			node = node->child_ptr[node->n];
		}
	}
	
	for (i = 0; i < 4; i++) {
		if (t[i].elementNum > 0) {
			ret.push_back(t[i]);
			arr[i] = t[i].nodeNum;
			cnt += t[i].nodeNum;
		}
	}

	return make_pair(make_pair(ret, arr), cnt);
}

int Component::merge(Component c) {
	Bplus_Tree* cur;
	int i, ionum;

	cur = c.root, ionum = 0;
	while (cur->leaf == false) {
		cur = cur->child_ptr[0];
	}
	while (1) {
		for (i = 0; i < cur->n; i++) {
			insert(cur->key[i]);
		}
		if (cur->child_ptr[cur->n] == NULL) break;
		cur = cur->child_ptr[cur->n];
	}

	ionum = c.nodeNum;

	return ionum;
}


void Component::deletetree(Bplus_Tree* cur) {
	int i;
	Bplus_Tree* node;
	if (cur->leaf == true) {
		node = cur;
		delete(node);
		return;
	}
	for (i = 0; i <= cur->n; i++) {
		deletetree(cur->child_ptr[i]);
	}
	node = cur;
	delete(node);

	return;
}

void Component::make_empty() {
	deletetree(root);
	elementNum = 0;
	root = NULL;

	return;
}

void Component::show() {
	int i;
	Bplus_Tree* cur = root;

	if (cur == NULL) {
		cout << "empty\n";
		return;
	}
	while (cur->leaf == false) {
		cur = cur->child_ptr[0];
	}
	while (1) {
		for (i = 0; i < cur->n; i++) {
			cout << "x : " << cur->key[i].x << ", y : " << cur->key[i].y << ", geohash : " << cur->key[i].geohash << "\n";
		}
		if (cur->child_ptr[cur->n] == NULL) break;
		cur = cur->child_ptr[cur->n];
	}
	return;
}

double Component::min_dist(int x, int y) {
	double ret;
	int dist_x, dist_y;
	if (x_min <= x && x_max >= x) {
		dist_x = 0;
	}
	else if(x < x_min) {
		dist_x = x_min - x;
	}
	else if (x > x_max) {
		dist_x = x - x_max;
	}

	if (y_min <= y && y_max >= y) {
		dist_y = 0;
	}
	else if (y < y_min) {
		dist_y = y_min - y;
	}
	else if (y > y_max) {
		dist_y = y - y_max;
	}

	ret = sqrt(dist_x * dist_x + dist_y * dist_y);
	return ret;
}

priority_queue<pair<double, Data>> Component::brute(int x, int y, int k, priority_queue<pair<double, Data>> pq) {
	priority_queue<pair<double, Data>> ret;
	Bplus_Tree* node = root;
	int i;
	double dist;
	ret = pq;

	while (node->leaf == false) {
		node = node->child_ptr[0];
	}

	while (1) {
		for (i = 0; i < node->n; i++) {
			dist = sqrt(pow(node->key[i].x - x, 2) + pow(node->key[i].y - y, 2));
			if (ret.size() < k) {
				ret.push(make_pair(dist, node->key[i]));
			}
			else if (ret.size() == k && ret.top().first > dist) {
				ret.pop();
				ret.push(make_pair(dist, node->key[i]));
			}
		}
		if (node->child_ptr[node->n] == NULL) break;
		node = node->child_ptr[node->n];
	}

	return ret;
}

pair<priority_queue<pair<double, Data>>, int> Component::memory_search(int x, int y, int k, pair<priority_queue<pair<double, Data>>, int> pq) {
	pair<priority_queue<pair<double, Data>>, int> ret;
	int i;
	double dist;
	Bplus_Tree* cur = root;

	while (cur->leaf == false) {
		cur = cur->child_ptr[0];
	}
	ret = pq;
	while (1) {
		ret.second++;
		for (i = 0; i < cur->n; i++) {
			if (ret.first.size() < k) {
				dist = cur->key[i].dist_to_point(x, y);
				ret.first.push(make_pair(dist, cur->key[i]));
			}
			else if (ret.first.size() == k) {
				dist = cur->key[i].dist_to_point(x, y);
				if (dist < ret.first.top().first) {
					ret.first.pop();
					ret.first.push(make_pair(dist, cur->key[i]));
				}
			}
		}
		if (cur->child_ptr[cur->n] == NULL) break;
		cur = cur->child_ptr[cur->n];		
	}

	return ret;
}

bool Component::filtering(int x, int y, double r) {
	int i, j, sz, x_s, x_e, y_s, y_e, temp_xs, temp_xe, temp_ys, temp_ye;
	double dist, dist_x, dist_y;

	x_s = x_min / 256, x_e = x_max / 256, y_s = y_min / 256, y_e = y_max / 256;

	sz = filter.size();
	for (i = x_s; i <= x_e; i++) {
		for (j = y_s; j <= y_e; j++) {
			if (filter[i][j] == true) {
				temp_xs = i * 256, temp_xe = (i + 1) * 256 - 1, temp_ys = j * 256, temp_ye = (j + 1) * 256 - 1;
				if (temp_xs <= x && temp_xe >= x) {
					dist_x = 0;
				}
				else if (x < temp_xs) {
					dist_x = temp_xs - x;
				}
				else if (x > temp_xe) {
					dist_x = x - temp_xe;
				}

				if (temp_ys <= y && temp_ye >= y) {
					dist_y = 0;
				}
				else if (y < temp_ys) {
					dist_y = temp_ys - y;
				}
				else if (y > temp_ye) {
					dist_y = y - temp_ye;
				}
				dist = sqrt(dist_x * dist_x + dist_y * dist_y);

				if (dist <= r) {
					return true;
				}
			}
		}		
	}

	return false;
}

double Component::range_query(int x, int y) {
	int i;
	double dist;

	dist = min_dist(x, y);
	return dist;
}

Component::Bplus_Tree* Component::init() {
	int i;
	Component::Bplus_Tree* node;
	node = new Component::Bplus_Tree;
	node->key.resize(MAX_N);
	node->child_ptr.resize(MAX_N + 1);
	node->leaf = true;
	node->n = 0;
	for (i = 0; i < MAX_N + 1; i++) {
		node->child_ptr[i] = NULL;
	}
	node->xrange[0] = x_min, node->xrange[1] = x_max;
	node->yrange[0] = y_min, node->yrange[1] = y_max;

	nodeNum++;

	return node;
}

void Component::insert(Data a) {
	int i, j;

	elementNum++, makefilter(a.x, a.y);
	if (root == NULL) {
		root = init();
		root->key[0] = a;
		root->n++;
	}
	else {
		Bplus_Tree* cur = root, * parent;
		parent = cur;
		while (cur->leaf == false) { // leaf 노드까지 추적.
			parent = cur;
			for (i = 0; i < cur->n; i++) {
				if (a.compare(cur->key[i]) == false) { // a가 cur->key[i]보다 작으면 false 크면 true
					cur = cur->child_ptr[i];
					break;
				}
				if (i == cur->n - 1) {
					cur = cur->child_ptr[i + 1];
					break;
				}
			}
		}
		if (cur->n < MAX_N) {
			i = 0;
			while (a.compare(cur->key[i]) == true && i < cur->n) { //traverse to find where the new node is to be inserted
				i++;
			}
			for (j = cur->n; j > i; j--) {
				cur->key[j] = cur->key[j - 1];
			}
			cur->key[i] = a;
			cur->n++;
			cur->child_ptr[cur->n] = cur->child_ptr[cur->n - 1];
			cur->child_ptr[cur->n - 1] = NULL;
		}
		else { // leaf node 가 overflow 날 경우
			Bplus_Tree* newLeaf = init();
			Data virtualnode[MAX_N + 1];

			for (i = 0; i < MAX_N; i++) {
				virtualnode[i] = cur->key[i];
			}
			i = 0;
			while (a.compare(virtualnode[i]) == true && i < MAX_N) { //traverse to find where the new node is to be inserted
				i++;
			}
			for (j = MAX_N; j > i; j--) {
				virtualnode[j] = virtualnode[j - 1];
			}
			virtualnode[i] = a;

			cur->n = (MAX_N + 1) / 2;
			newLeaf->n = MAX_N + 1 - (MAX_N + 1) / 2;
			cur->child_ptr[cur->n] = newLeaf;
			newLeaf->child_ptr[newLeaf->n] = cur->child_ptr[MAX_N];			
			for (i = cur->n + 1; i <= MAX_N; i++) {
				cur->child_ptr[i] = NULL;
			}

			for (i = 0; i < cur->n; i++) {
				cur->key[i] = virtualnode[i];
			}
			for (i = cur->n; i < MAX_N; i++) {
				cur->key[i] = Data(-1, -1, -1);
			}
			for (i = 0, j = cur->n; i < newLeaf->n; i++, j++) {
				newLeaf->key[i] = virtualnode[j];
			}

			if (cur == root) {
				Bplus_Tree* newRoot = init();

				newRoot->key[0] = newLeaf->key[0];
				newRoot->child_ptr[0] = cur;
				newRoot->child_ptr[1] = newLeaf;
				newRoot->leaf = false;
				newRoot->n = 1;
				root = newRoot;
			}
			else {
				insertInternal(newLeaf->key[0], parent, newLeaf);
			}
		}
	}

	return;
}

void Component::insertInternal(Data a, Bplus_Tree* cur, Bplus_Tree* child) {
	if (cur->n < MAX_N) {
		int i, j;
		i = 0;
		while (a.compare(cur->key[i]) && i < cur->n) { // traverse the parent node to find where child node is to be inserted
			i++;
		}
		for (j = cur->n; j > i; j--) { // update parent key
			cur->key[j] = cur->key[j - 1];
		}
		for (j = cur->n + 1; j > i + 1; j--) { // update parent pointer
			cur->child_ptr[j] = cur->child_ptr[j - 1];
		}
		cur->key[i] = a;
		cur->n++;
		cur->child_ptr[i + 1] = child;
	}
	else { // for overflow, split the node
		Bplus_Tree* newInternal = init();
		Data virtualKey[MAX_N + 1];
		Bplus_Tree* virtualPtr[MAX_N + 2];
		int i, j;

		for (i = 0; i < MAX_N; i++) {
			virtualKey[i] = cur->key[i];
		}
		for (i = 0; i < MAX_N + 1; i++) {
			virtualPtr[i] = cur->child_ptr[i];
		}

		i = 0;
		while (a.compare(virtualKey[i]) && i < MAX_N) { // traverse to find where the new node is to be inserted
			i++;
		}
		for (j = MAX_N; j > i; j--) { // update parent key
			virtualKey[j] = virtualKey[j - 1];
		}
		virtualKey[i] = a;
		for (j = MAX_N + 1; j > i + 1; j--) { // update parent pointer
			virtualPtr[j] = virtualPtr[j - 1];
		}
		virtualPtr[i + 1] = child;

		newInternal->leaf = false;
		cur->n = (MAX_N + 1) / 2;
		newInternal->n = MAX_N - (MAX_N + 1) / 2;

		for (i = 0; i < MAX_N; i++) {
			if (i < cur->n) 
				cur->key[i] = virtualKey[i];
			else if (i >= cur->n) 
				cur->key[i] = Data(-1, -1, -1);
		}
		for (i = 0; i < MAX_N + 1; i++) {
			if (i < cur->n + 1)
				cur->child_ptr[i] = virtualPtr[i];
			else if (i >= cur->n + 1)
				cur->child_ptr[i] = NULL;
		}

		for (i = 0, j = cur->n + 1; i < newInternal->n; i++, j++) {
			newInternal->key[i] = virtualKey[j];
		}
		for (i = 0, j = cur->n + 1; i < newInternal->n + 1; i++, j++) {
			newInternal->child_ptr[i] = virtualPtr[j];
		}

		if (cur == root) {
			Bplus_Tree* newRoot = init();

			newRoot->key[0] = newInternal->key[0];
			newRoot->child_ptr[0] = cur;
			newRoot->child_ptr[1] = newInternal;
			newRoot->leaf = false;
			newRoot->n = 1;
			root = newRoot;
		}
		else {
			insertInternal(newInternal->key[0], findParent(root, cur), newInternal);
		}
	}

	return;
}

Component::Bplus_Tree* Component::findParent(Bplus_Tree* cur, Bplus_Tree* child) { // Function to find the parent node from root node 그런데 너무 오래걸릴것같다....
	Component::Bplus_Tree* parent = NULL;
	int i;
	if (cur->leaf || cur->child_ptr[0]->leaf) 
		return NULL;
	
	for (i = 0; i < cur->n + 1; i++) {
		if (cur->child_ptr[i] == child) {
			parent = cur;
			return parent;
		}
	}

	for (i = 0; i < cur->n + 1; i++) {
		if (i < cur->n && child->key[0].compare(cur->key[i])) continue; // cur->key[i] <= child->key[0] 이면 true
		parent = findParent(cur->child_ptr[i], child);

		if (parent != NULL)
			return parent;
	}

	return parent;
}


