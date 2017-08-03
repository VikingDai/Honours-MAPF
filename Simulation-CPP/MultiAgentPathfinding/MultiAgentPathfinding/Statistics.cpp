#include "Statistics.h"

double Stats::avgMipTime;
double Stats::avgCoordinatorTime;
double Stats::avgSearchTime;
int Stats::agentCounter;

void Stats::Reset()
{
	avgMipTime = 0;
	avgCoordinatorTime = 0;
	avgSearchTime = 0;
	agentCounter = 0;
}
