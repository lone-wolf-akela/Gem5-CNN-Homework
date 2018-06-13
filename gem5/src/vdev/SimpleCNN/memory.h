#pragma once
#include<memory>


class Memory
{
public:
	Memory(int xLen = 1, int yLen = 1, int zLen = 1, int wLen = 1);

	float* getPtr(int xIndex = 0, int yIndex = 0, int zIndex = 0, int wIndex = 0) const;
	int getMemSizeByByte(void) const;
	int getMemSizeByFloat(void) const;
private:
	std::unique_ptr<float[]> _data;
	int _xLen, _yLen, _zLen, _wLen;
	int _wzLen, _wzyLen, _wzyxLen;
};