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
#include "Timers.h"

using namespace std;

const int N = 1e7;  // limit for array size
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

vector<Operation> generate_ops(int q, int n) {
	vector<Operation> ops(q);
	for (int i = 0; i < q; i++) {
		int type = rand() % 2,
			l = rand() % n,
			r = rand() % n;
		if (l > r)
			swap(l, r);
		ops[i] = { type, l, r };
	}
	return ops;
}

pair<vector<long long>, vector<long long>> test_query(int tests) {
	vector<long long> cl, eff;
	for (int i = 10; i <= N; i = (i < 100 ? i + 10 : i * 10)) {
		vector<long long> times_cl(tests);
		vector<long long> times_eff(tests);
		vector<long long> sum_cl(tests);
		vector<long long> sum_eff(tests);
		for (int j = 0; j < tests; j++) {
			auto timer = QPC();
			auto ops = generate_ops(i, i);
			timer.Start();
			for (auto op : ops) {
				auto val = query(1, 0, n, op.l, op.r);
				sum_cl[j] = (sum_cl[j] + val) % 1000000007;
			}
			long long classic_time = timer.Stop();

			timer.Start();
			for (auto op : ops) {
				auto val = query(op.l, op.r + 1);
				sum_eff[j] = (sum_eff[j] + val) % 1000000007;
			}
			long long eff_time = timer.Stop();
			times_cl[j] = classic_time;
			times_eff[j] = eff_time;
		}
		long long min_cl = *min_element(times_cl.begin(), times_cl.end()),
			      min_eff = *min_element(times_eff.begin(), times_eff.end());
		cl.push_back(min_cl);
		eff.push_back(min_eff);
	}
	return { cl, eff };
}

pair<vector<long long>, vector<long long>> test_update(int tests) {
	vector<long long> cl, eff;
	for (int i = 10; i <= N; i = (i < 100 ? i + 10 : i * 10)) {
		vector<long long> times_cl(tests);
		vector<long long> times_eff(tests);
		for (int j = 0; j < tests; j++) {
			auto timer = QPC();
			auto ops = generate_ops(i, i);
			timer.Start();
			for (auto op : ops) {
				update(1, 0, n, op.l, op.r);
			}
			times_cl[j] = timer.Stop();

			timer.Start();
			for (auto op : ops) {
				modify(op.l, op.r);
			}
			times_eff[j] = timer.Stop();
		}
		long long min_cl = *min_element(times_cl.begin(), times_cl.end()),
			      min_eff = *min_element(times_eff.begin(), times_eff.end());
		cl.push_back(min_cl);
		eff.push_back(min_eff);
	}
	return { cl, eff };
}

void print_results(pair<vector<long long>, vector<long long>>& res, string& intro_str) {
	auto time_cl = res.first,
		time_eff = res.second;

	cout << intro_str << "\nclassic sg: ";
	for (auto time : time_cl) {
		cout << time << " ";
	}
	cout << "nsec\n";
	cout << "efficient sg: ";
	for (auto time : time_eff) {
		cout << time << " ";
	}
	cout << "nsec\n"
		<< "cl[i] / eff[i]: ";
	for (int i = 0; i < time_cl.size(); i++) {
		printf("%.2lf ", double(time_cl[i]) / time_eff[i]);
	}
	cout << "\n\n";
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
	
	// Checking query
	auto q_time = test_query(10);
	print_results(q_time, string("Sum"));

	// Checking update
	auto upd_time = test_update(10);
	print_results(upd_time, string("Modification"));
}