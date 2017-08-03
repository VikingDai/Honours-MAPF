#include "Timer.h"


Timer::Timer()
{
	Reset();
}

void Timer::Reset()
{
	avgTime = timeElapsed = timeAccumulated = numberOfTimesRan = 0;
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
	timeAccumulated += timeElapsed;
	avgTime = timeAccumulated / static_cast<double>(numberOfTimesRan);
}

void Timer::PrintTimeElapsed(char* functionName)
{
	std::cout << functionName << " took " << timeElapsed << " seconds" << std::endl;
}

void Timer::PrintTimeAccumulated(char* functionName)
{
	std::cout << functionName << " took " << timeAccumulated << " seconds" << std::endl;
}