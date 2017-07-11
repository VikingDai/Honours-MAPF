#pragma once

#include <stdlib.h>

namespace MathUtils
{
	float randomFloat()
	{
		return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	}
}