#pragma once

#include <chrono>
#include <ctime>

class Timer
{
	std::chrono::time_point<std::chrono::system_clock> start, endTime;
	int numberOfTimesRan;
	float avgTime;
	float timeElapsed;
	float timeAcc;

public:
	float getAvgTime() { return avgTime; }
	float getTimeElapsed() { return timeElapsed; }

	Timer();
	void Begin();
	void End();

};

