#include "Data.h"
#include <iostream>
#include <stack>
#include <string>
#include <vector>
using namespace std;

#define hash_len 6
// max size : 1023
#define max_len 14 // 0~16383

Data::Data() {};
Data::Data(int a, int b, int c) {
	x = a, y = b, id = c;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	geohash = int_to_string(x, y);
}

bool Data::operator<(const Data& i) const {
	if (geohash == i.geohash) return id < i.id;
	return geohash < i.geohash;
}

vector<int> Data::int_to_two(int num) {
	int temp, idx, i, cnt;
	stack<int> st;
	vector<int> ret;

	temp = num;
	while (temp) {
		st.push(temp % 2);
		temp /= 2;
	}
	idx = 0, cnt = 0;
	for (i = st.size(); i < max_len; i++) {
		ret.push_back(0);
		cnt++;
	}

	while (!st.empty()) {
		if (cnt == hash_len) st.pop();
		ret.push_back(st.top());
		st.pop();
		cnt++;
	}

	return ret;
}

string Data::int_to_string(int a, int b) {
	vector<int> r, c, mix;
	string ret;
	stack<int> st;
	char ch;
	int i;

	r = int_to_two(a);
	c = int_to_two(b);

	for (i = 0; i < hash_len; i++) {
		mix.push_back(r[i]);
		mix.push_back(c[i]);
	}
	for (i = 0; i < 2 * hash_len; i++) {
		ch = mix[i] + '0';
		ret.push_back(ch);
	}

	return ret;
}

double Data::dist_to_point(int q_x, int q_y) {
	double ret;
	ret = sqrt(pow(x - q_x, 2) + pow(y - q_y, 2));

	return ret;
}

bool Data::compare(Data& i) {
	if (geohash > i.geohash) return true;
	else if (this->geohash == i.geohash) {
		if (id >= i.id) return true;
		else return false;
	}
	else
		return false;
}

