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
#include <set>

using namespace std;

class Timer { // Абстракный класс. Нужен для обощения работы с таймерами.
public:
	string name; //   название типа часов
	long long time, freq, del; // время и частота

							   // Засекает время
	virtual void Start() = 0;

	// Измеряет прошедшее время после последней засечки
	virtual long long Stop() = 0;
};

class Clock : public Timer { // часы на основе функции clock()
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

class QPC : public Timer { // часы на основе QPC
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

class TSC : public Timer { // часы на основе Time Stamp Counter
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
		start = __rdtsc() - start; // замер продолжительности
		return (1.0 * start / freq) * del;
	}

	long long hz_cpu() { // замер частоты
		clock_t t_clock;
		long long t_tsc;
		t_clock = clock() + CLOCKS_PER_SEC;
		t_tsc = __rdtsc(); // взять TSC
		while (clock() < t_clock); // отсчет одной секунды
		return (__rdtsc() - t_tsc); // частота в герцах
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
		if (l & 1) 
			res += tn[l++];
		if (r & 1)
			res += tn[--r];
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

pair<long long, long long> test_query(vector<Operation>& ops, int tests) {
	vector<long long> times_cl(tests);
	vector<long long> times_eff(tests);
	vector<long long> sum_cl(tests);
	vector<long long> sum_eff(tests);
	for (int i = 0; i < tests; i++) {
		auto timer = Clock();
		timer.Start();
		for (auto op : ops) {
			auto val = query(1, 0, n, op.l, op.r);
			sum_cl[i] = (sum_cl[i] + val) % 1000000007;
		}
		long long classic_time = timer.Stop();

		timer.Start();
		for (auto op : ops) {
			auto val = query(op.l, op.r + 1);
			sum_eff[i] = (sum_eff[i] + val) % 1000000007;
		}
		long long eff_time = timer.Stop();
		times_cl[i] = classic_time;
		times_eff[i] = eff_time;
	}
	long long min_cl = *min_element(times_cl.begin(), times_cl.end()),
			  min_eff = *min_element(times_eff.begin(), times_eff.end());
	return { min_cl, min_eff };
}

pair<long long, long long> test_update(vector<Operation>& ops, int tests) {
	vector<long long> times_cl(tests);
	vector<long long> times_eff(tests);
	for (int i = 0; i < tests; i++) {
		auto timer = Clock();
		timer.Start();
		for (auto op : ops) {
			update(1, 0, n, op.l, op.r);
		}
		times_cl[i] = timer.Stop();

		timer.Start();
		for (auto op : ops) {
			modify(op.l, op.r);
		}
		times_eff[i] = timer.Stop();
	}
	long long min_cl = *min_element(times_cl.begin(), times_cl.end()),
		min_eff = *min_element(times_eff.begin(), times_eff.end());
	return { min_cl, min_eff };
}

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

	int q = 5e6;
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
	auto q_time = test_query(ops, 10);
	long long time_cl_query = q_time.first,
			  time_eff_query = q_time.second;
	cout << "classic sg: " << time_cl_query << " msec" << endl
		<< "efficient: " << time_eff_query << " msec" << endl;

	// Checking update
	auto upd_time = test_update(ops, 10);
	long long time_cl_update = upd_time.first,
			  time_eff_update = upd_time.second;
	cout << "classic sg: " << time_cl_update << " msec" << endl
		<< "efficient: " << time_eff_update << " msec" << endl;
}