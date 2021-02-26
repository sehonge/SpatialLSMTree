#define _CRT_SECURE_NO_WARNINGS
#include "Tree.h"
#include "Level.h"
#include "Component.h"
#include "Data.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <queue>
#include <time.h>
using namespace std;

Tree LSM;
Data query_point_to_Data();
int KNN1(Data p);
int KNN2(Data p);
int KNN3(Data p);
void brute_force();
int Range(Data d, double r);

int main() {
	FILE* fp = fopen("data_clustered.txt", "r"); // input file name
	int r, c, cnt, ord, ord2, n1, n2 , n3, n0, numbercheck, i, kk, j;
	vector<int> iocheck, rr;
	vector<pair<int, int>> dataset;

	LSM = Tree();
	cnt = 1;
	if (fp == NULL) {
		cout << "No file\m";
		return 0;
	}
	while (1) {
		ord = fscanf(fp, "%d %d", &r, &c);
		if (ord == EOF) break;
		if (r < 0) r = 0;
		if (c < 0) c = 0;
		LSM.insert_data(Data(r, c, cnt++));
		if (cnt % 2000000 == 0) cout << cnt / 10000 << "만 ";
		if (cnt == 10'000'000) iocheck.push_back(LSM.IOnum), cout<<"\n"; // 2천만으로 올려야함
		else if (cnt == 20'000'000) iocheck.push_back(LSM.IOnum), cout << "\n";
		else if (cnt == 30'000'000) iocheck.push_back(LSM.IOnum), cout << "\n";
		else if (cnt == 40'000'000) iocheck.push_back(LSM.IOnum), cout << "\n";
		else if (cnt == 50'000'000) { 
			iocheck.push_back(LSM.IOnum), cout << "\n";
			break;
		}
	}
	fclose(fp);	
	if (LSM.max_level >= 1) LSM.maintain(1);
	cout << "insertion disk I/O :" << LSM.IOnum << "\n";

	FILE* fp2 = fopen("output2.txt", "w");

	
	kk = 10'000'000; // 2천만으로 올려야함
	for (i = 0; i < 5; i++) {
		fprintf(fp2, "%d insertion I/O %d\n", kk, iocheck[i]);
		kk += 10'000'000;
	}
	
	// random point 10개
	dataset.push_back({ 6140,12330 }), dataset.push_back({ 10434,11934 }), dataset.push_back({ 5459,4875 }), dataset.push_back({ 12067,4014 });
	dataset.push_back({ 4185,8556 }), dataset.push_back({ 8564,9902 }), dataset.push_back({ 13602,14075 }), dataset.push_back({ 3185,3054 });
	dataset.push_back({ 9987,7221 }), dataset.push_back({ 2007,9510 });
	
	rr.push_back(10), rr.push_back(50), rr.push_back(100), rr.push_back(500); // r 범위 (range query)
	fprintf(fp2, "data_clustered disk i/o\n");
	
	
	Data point;
	for (i = 0; i < 10; i++) {
		fprintf(fp2, "Range Query\n");
		point.x = dataset[i].first, point.y = dataset[i].second, point.id = -1;
		for (j = 0; j < 4; j++) {
			fprintf(fp2, "x = %d, y = %d, r = %d ", dataset[i].first, dataset[i].second, rr[j]);
			n0 = Range(point, rr[j]);
			fprintf(fp2, "I/O : %d\n", n0);
		}

		fprintf(fp2, "\nKNN Query\n");
		kk = 5;
		point.id = kk;
		for (j = 0; j < 4; j++) {
			fprintf(fp2, "k = %d\n", kk);
			n1 = KNN1(point);
			fprintf(fp2, "KNN1 : %d\n", n1);

			n2 = KNN2(point);
			fprintf(fp2, "KNN2 : %d\n", n2);

			n3 = KNN3(point);
			fprintf(fp2, "KNN3 : %d\n\n", n3);
			point.id += 5;
			kk += 5;
		}

		fprintf(fp2, "\n");
	}

	fclose(fp2);
	/*
	while (1) {
		cout << "\n----------------OUTPUT------------------\n";
		cout << "Option\nshow data : 1\nKNN query : 2\nRnage query : 3\nterminate : 4\norder : ";
		cin >> ord;
		if (ord == 1) {			
			cout << "which level?(0 is Memory level) ";
			cin >> ord2;
			LSM.show(ord2);
		}
		else if (ord == 2) {		// 전체 component의 개수, Queue에 집어넣은 component개수, 실제 파일에서 가져온 component의 개수(파일 I/O) 를 비교해야 한다.
			cout << "---------------------KNN---------------------\n";
			cout << "Option\nKNN algorithm1 : 1\nKNN algorithm2 : 2\nKNN algorithm3 : 3\nbrute force : 4\n which one ? : ";
			cin >> ord2;
			
			if (ord2 == 1) {
				start_t = clock();
				KNN1();
				end_t = clock();
				cout << "\ntime : " << double(end_t - start_t) << "ms\n";
			}
			else if (ord2 == 2) {
				start_t = clock();
				KNN2();
				end_t = clock();
				cout << "\ntime : " << double(end_t - start_t) << "ms\n";
			}
			else if (ord2 == 3) {
				start_t = clock();
				KNN3();
				end_t = clock();
				cout << "\ntime : " << double(end_t - start_t) << "ms\n";
			}
			else if (ord2 == 4) {
				start_t = clock();
				brute_force();
				end_t = clock();
				cout << "\ntime : " << double(end_t - start_t) << "ms\n";
			}
			else {
				cout << "Error\n";
			}		
		}
		else if (ord == 3) {
			Range();
		}
		else if(ord == 4) {
			break;
		}
		else {
			cout << "Wrong order\n";
		}
	}	
	*/

	return 0;
}

int Range(Data d, double r) {
	int row, col, i, ret;
	pair<priority_queue<pair<double, Data>>, int> temp;
	priority_queue<pair<double, Data>> Q;
	Data point = d;

	temp = LSM.RangeQuery(point, r);
	Q = temp.first, ret = temp.second;

	i = 1;
	/*
	while (!Q.empty()) {
		cout << i++ << ". x : " << Q.top().second.x << ", y : " << Q.top().second.y << ", distance from query point : " << Q.top().first << "\n";
		Q.pop();
	}
	*/
	return ret;
}

int KNN1(Data p) {
	int i, ret;
	pair<priority_queue<pair<double, Data>>, int> temp;
	priority_queue<pair<double, Data>> Q;
	//Data point= query_point_to_Data();

	temp = LSM.KNN1(p, p.id);
	Q = temp.first, ret = temp.second;
	/*
	i = 1;
	while (!Q.empty()) {
		cout << i++ << ". x : " << Q.top().second.x << ", y : " << Q.top().second.y << ", distance from query point : " << Q.top().first << "\n";
		Q.pop();
	}
	*/
	return ret;
}

int KNN2(Data p) {
	int i, ret;
	pair<priority_queue<pair<double, Data>>, int> temp;
	priority_queue<pair<double, Data>> Q;
	//Data point = query_point_to_Data();

	temp = LSM.KNN2(p, p.id);
	Q = temp.first, ret = temp.second;
	/*
	i = 1;
	while (!Q.empty()) {
		cout << i++ << ". x : " << Q.top().second.x << ", y : " << Q.top().second.y << ", distance from query point : " << Q.top().first << "\n";
		Q.pop();
	}
	*/
	return ret;
}

int KNN3(Data p) {
	int i, ret;
	pair<priority_queue<pair<double, Data>>, int> temp;
	priority_queue<pair<double, Data>> Q;
	//Data point = query_point_to_Data();

	temp = LSM.KNN3(p, p.id);
	Q = temp.first, ret = temp.second;
	/*
	i = 1;
	while (!Q.empty()) {
		cout << i++ << ". x : " << Q.top().second.x << ", y : " << Q.top().second.y << ", distance from query point : " << Q.top().first << "\n";
		Q.pop();
	}
	*/
	return ret;
}


Data query_point_to_Data() {
	int r, c, k;

	cout << "insert x_coordinate : ";
	cin >> r;
	cout << "insert y_coordinate : ";
	cin >> c;
	cout << "insert size of K(or R) : ";
	cin >> k;

	return Data(r, c, k);
}

void brute_force() {
	priority_queue<pair<double, Data>> Q;
	Data point = query_point_to_Data();
	int cnt;

	Q = LSM.brute_force(point, point.id);
	cnt = 1;
	while (!Q.empty()) {
		cout << cnt++ << ". x : " << Q.top().second.x << ", y : " << Q.top().second.y << ", distance from query point : " << Q.top().first << "\n";
		Q.pop();
	}
	return;
}
