#include "Timer.h"



Timer::Timer()
{
	avgTime = timeElapsed = timeAcc = numberOfTimesRan = 0;
}

void Timer::Begin()
{
	start = std::chrono::system_clock::now();
}

void Timer::End()
{
	numberOfTimesRan += 1;
	endTime = std::chrono::system_clock::now();
	std::chrono::duration<double> duration = endTime - start;
	timeElapsed = duration.count();
	timeAcc += timeElapsed;
	avgTime = timeAcc / static_cast<double>(numberOfTimesRan);
}