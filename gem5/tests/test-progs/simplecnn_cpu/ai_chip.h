#pragma once
#include "core.h"

class AIChip
{
public:
	int tickPerCycle = 1;

	int run();

	int result = -1;
	bool finished = false;
private:
	int _counter = -1;
	Core core;
};