#pragma once

#include "Environment.h"

/* The Experiment class is used for generating and plotting 
data for repeated agent coordinator solutions. */
class Experiment
{
public:
	Experiment() = default;
	void RunExperiment(std::string filename, Environment& environment);
};

