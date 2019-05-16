#pragma once
#include <chrono>
#include <string>
#include <time.h>
#include <intrin.h>
#include <windows.h>

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