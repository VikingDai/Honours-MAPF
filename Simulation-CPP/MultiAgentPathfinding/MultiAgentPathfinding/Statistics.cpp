#include "Statistics.h"

double Stats::avgSearchTime;
int Stats::agentCounter;

void Stats::Reset()
{
	avgSearchTime = 0;
	agentCounter = 0;
}
