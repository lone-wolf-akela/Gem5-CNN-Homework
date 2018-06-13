#include <iostream>

#include "ai_chip.h"

using namespace std;

#define BATCH 10

int main()
{
	int result[BATCH];
	AIChip chip;
	for (int i = 0; i < BATCH; i++)
	{
		int time = 0;
		while (time += chip.run(), !chip.finished)
		{
			//cout << "time used: " << time << endl;
		}

		result[i] = chip.result;
	}
	ifstream label;
	label.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	label.open("data/t10k-labels.idx1-ubyte", ios::in | ios::binary);
	label.seekg(8);
	int correct = 0;
	for (int i = 0; i < BATCH; i++)
	{
		uint8_t l;
		label.read(reinterpret_cast<char*>(&l), 1);
		if (result[i] == l)
		{
			correct++;
		}
	}
	cout << correct << '/' << BATCH << '\t' << double(correct) / double(BATCH)*100. << '%' << endl;

	return 0;
}