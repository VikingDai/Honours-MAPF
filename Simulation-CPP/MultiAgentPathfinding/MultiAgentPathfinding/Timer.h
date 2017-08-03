#pragma once

#include <chrono>
#include <ctime>
#include <iostream>

class Timer
{
	std::chrono::time_point<std::chrono::system_clock> start, endTime;
	int numberOfTimesRan;
	float avgTime;
	float timeElapsed;
	float timeAcc;

public:
	float GetAvgTime() { return avgTime; }
	float GetTimeElapsed() { return timeElapsed; }

	Timer();
	void Begin();
	void End();

	void PrintTimeElapsed(char* functionName);
};

