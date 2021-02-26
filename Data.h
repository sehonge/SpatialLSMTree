#pragma once
#include <string>
#include <vector>
#include <queue>
using namespace std;

class Data
{
public:
	int x, y, id;
	string geohash;
	Data();
	Data(int a, int b, int c);
	string int_to_string(int a, int b);
	vector<int> int_to_two(int num);
	bool operator<(const Data& i) const;
	bool compare(Data& i);
	double dist_to_point(int x, int y);
};

