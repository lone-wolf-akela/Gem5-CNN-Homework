#include "ai_chip.h"

int AIChip::run()
{
	int cyclesUsed = 0;
	finished = false;

	if (_counter == -1)
	{
		cyclesUsed = core.op_init();
	}
	if (_counter == 0)
	{
		cyclesUsed = core.op_input("data/padding/in");
	}
	//padding1
	if(_counter==1)
	{
		cyclesUsed = core.op_padding(
			"data/padding/in", "data/padding/out|conv2d/in",
			2, 28, 28, 1, 1
		);
	}
	//conv1
	if(_counter >= 2 && _counter<2 + 32)
	{
		const int channel = _counter - 2;
		cyclesUsed = core.op_conv(
			"data/padding/out|conv2d/in", "data/conv2d/out|pool/in",
			"model/conv2d/kernel", "model/conv2d/bias",
			1, Range(channel, channel + 1), Range(0, 28), Range(0, 28),
			5, 1
		);
	}
	//pool1
	if(_counter>= 34 && _counter<34+32)
	{
		const int channel = _counter - 34;
		cyclesUsed = core.op_pool(
			"data/conv2d/out|pool/in", "data/pool/out|padding_1/in",
			Range(channel, channel + 1), Range(0, 14), Range(0, 14),
			2, 2
		);
	}
	//padding2
	if (_counter == 66)
	{
		cyclesUsed = core.op_padding(
			"data/pool/out|padding_1/in", "data/padding_1/out|conv2d_1/in",
			2, 14, 14, 32, 1
		);
	}
	//conv2
	if (_counter >= 67 && _counter<67 + 64)
	{
		const int channel = _counter - 67;
		cyclesUsed = core.op_conv(
			"data/padding_1/out|conv2d_1/in", "data/conv2d_1/out|pool_1/in",
			"model/conv2d_1/kernel", "model/conv2d_1/bias",
			32, Range(channel, channel + 1), Range(0, 14), Range(0, 14),
			5, 1
		);
	}
	//pool2
	if(_counter>=131 && _counter<131+64)
	{
		const int channel = _counter - 131;
		cyclesUsed = core.op_pool(
			"data/conv2d_1/out|pool_1/in", "data/pool_1/out|dense/in",
			Range(channel, channel + 1), Range(0, 7), Range(0, 7),
			2, 2
		);
	}
	//fc1
	if (_counter >= 195 && _counter<195 + 16)
	{
		const int outIndex = (_counter - 195) * 32;
		cyclesUsed = core.op_fc(
			"data/pool_1/out|dense/in", "data/dense/out|dense_1/in",
			"model/dense/kernel", "model/dense/bias",
			7 * 7 * 64, Range(outIndex, outIndex + 32)
		);
	}
	//fc2
	if(_counter == 211)
	{
		cyclesUsed = core.op_fc(
			"data/dense/out|dense_1/in", "data/dense_1/out",
			"model/dense_1/kernel", "model/dense_1/bias",
			512, Range(0, 10)
		);
	}
	//output
	if(_counter == 212)
	{
		cyclesUsed = core.op_output("data/dense_1/out");

		result = core.outputIndex;
		finished = true;
		_counter = -1;
	}

	_counter++;
	return cyclesUsed * tickPerCycle;
}
