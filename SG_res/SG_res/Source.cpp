#define _USE_MATH_DEFINES
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <omp.h>
#include <time.h>
#include <intrin.h>
#include <windows.h>
#include <functional>
#include <limits>
#include <cstddef>
#include <map>
#include <numeric>
#include <string>
#include <chrono>

using namespace std;

class Timer { // ���������� �����. ����� ��� �������� ������ � ���������.
public:
	string name; //   �������� ���� �����
	long long time, freq, del; // ����� � �������

							   // �������� �����
	virtual void Start() = 0;

	// �������� ��������� ����� ����� ��������� �������
	virtual long long Stop() = 0;
};

class Clock : public Timer { // ���� �� ������ ������� clock()
public:
	long long start;

	Clock() {
		freq = CLOCKS_PER_SEC;
		name = "clock";
	}

	void Start() override {
		start = clock();
	}

	long long Stop() override {
		start = clock() - start;
		return start;
	}
};

class QPC : public Timer { // ���� �� ������ QPC
public:
	LONGLONG t_start, t_end;

	QPC() {
		name = "QPC";
		QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
	}

	void Start() override {
		QueryPerformanceCounter((LARGE_INTEGER *)&t_start);
	}

	long long Stop() override {
		QueryPerformanceCounter((LARGE_INTEGER *)&t_end);
		time = ((t_end - t_start) / (double)freq) * 1e9;
		return time;
	}
};

class TSC : public Timer { // ���� �� ������ Time Stamp Counter
public:
	long long start;

	TSC() {
		name = "TSC";
		del = 1e9;
	}

	void Start() override {
		freq = hz_cpu();
		start = __rdtsc();
	}

	long long Stop() override {
		start = __rdtsc() - start; // ����� �����������������
		return (1.0 * start / freq) * del;
	}

	long long hz_cpu() { // ����� �������
		clock_t t_clock;
		long long t_tsc;
		t_clock = clock() + CLOCKS_PER_SEC;
		t_tsc = __rdtsc(); // ����� TSC
		while (clock() < t_clock); // ������ ����� �������
		return (__rdtsc() - t_tsc); // ������� � ������
	}
};

class Chrono : public Timer {
public:
	chrono::time_point<chrono::high_resolution_clock> start;

	Chrono() {
		freq = CLOCKS_PER_SEC;
		name = "Chrono";
	}

	void Start() override {
		start = chrono::high_resolution_clock::now();
	}

	long long Stop() override {
		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double, nano> time = end - start;
		return time.count();
	}
};

const int N = 5e6;  // limit for array size
int n = N;  // array size
long long tn[2 * N];

void build() {  // build the tree
	for (int i = n - 1; i > 0; --i)
		tn[i] = tn[i << 1] + tn[i << 1 | 1];
}

void modify(int p, int value) {  // set value at position p
	for (tn[p += n] = value; p > 1; p >>= 1)
		tn[p >> 1] = tn[p] + tn[p ^ 1];
}

long long query(int l, int r) {  // sum on interval [l, r)
	long long res = 0;
	for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
		if (l & 1) res += tn[l++];
		if (r & 1) res += tn[--r];
	}
	return res;
}

long long t[4 * N];

void init(vector<long long>& arr, int v, int l, int r) {
	if (l == r) {
		t[v] = arr[l];
	}
	else {
		int mid = (l + r) / 2;
		init(arr, v * 2, l, mid);
		init(arr, v * 2 + 1, mid + 1, r);
		t[v] = t[v * 2] + t[v * 2 + 1];
	}
}

void update(int v, int l, int r, int ind, int val) {
	if (l == r) {
		t[v] = val;
	}
	else {
		int mid = (l + r) / 2;
		if (ind > mid) {
			update(v * 2 + 1, mid + 1, r, ind, val);
		}
		else {
			update(v * 2, l, mid, ind, val);
		}
		t[v] = t[v * 2] + t[v * 2 + 1];
	}
}

long long query(int v, int L, int R, int l, int r) {
	if (l > r)
		return 0;
	if (l == L && r == R)
		return t[v];
	int mid = (L + R) / 2;
	return query(v * 2, L, mid, l, min(r, mid)) + query(v * 2 + 1, mid + 1, R, max(l, mid + 1), r);
}

struct Operation {
	int type;
	int l;
	int r;

	Operation(int _type, int _l, int _r) {
		type = type;
		l = _l;
		r = _r;
	}

	Operation() : Operation(0, 0, 0) {

	}

};

int main() {
	freopen("input.txt", "r", stdin);
	freopen("output.txt", "w", stdout);

	vector<long long> base_arr(n);
	for (int i = 0; i < n; i++) {
		int rnd_val = rand();
		base_arr[i] = rnd_val;
		tn[n + i] = rnd_val;
	}

	build();
	init(base_arr, 1, 0, n - 1);

	int q = 6e6;
	vector<Operation> ops(q);
	for (int i = 0; i < q; i++) {
		int type = rand() % 2,
			l = rand() % n,
			r = rand() % n;
		if (l > r)
			swap(l, r);
		ops[i] = { type, l, r };
	}

	// Checking query
	auto timer = Clock();
	long long sum = 0;
	timer.Start();
	for (auto op : ops) {
		auto val = query(1, 0, n, op.l, op.r);
		sum = (sum + val) % 1000000007;
	}
	long long classic_time = timer.Stop();

	long long sum2 = 0;
	timer.Start();
	for (auto op : ops) {
		auto val = query(op.l, op.r + 1);
		sum2 = (sum2 + val) % 1000000007;
	}
	long long eff_time = timer.Stop();
	cout << sum << endl << sum2 << endl;
	cout << "classic sg: " << classic_time << " msec" << endl
	   	 << "efficient: " << eff_time << " msec" << endl;

	// Checking update
	timer.Start();
	for (auto op : ops) {
		update(1, 0, n, op.l, op.r);
	}
	classic_time = timer.Stop();

	timer.Start();
	for (auto op : ops) {
		modify(op.l, op.r);
	}
	eff_time = timer.Stop();
	cout << "classic sg: " << classic_time << " msec" << endl
		<< "efficient: " << eff_time << " msec" << endl;
}