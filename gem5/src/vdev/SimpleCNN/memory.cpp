#include <cassert>

#include "memory.h"

Memory::Memory(int xLen, int yLen, int zLen, int wLen) :
	_xLen(xLen), _yLen(yLen), _zLen(zLen), _wLen(wLen)
{
	_wzLen = _wLen * _zLen;
	_wzyLen = _wzLen * _yLen;
	_wzyxLen = _wzyLen * _xLen;

	_data = std::unique_ptr<float[]>(new float[_wzyxLen]);
}

float* Memory::getPtr(int xIndex, int yIndex, int zIndex, int wIndex) const
{
	const int index = wIndex + zIndex * _wLen + yIndex * _wzLen + xIndex * _wzyLen;
	assert(index < _wzyxLen);

	return _data.get() + index;
}

int Memory::getMemSizeByByte(void) const
{
	return _wzyxLen * sizeof(float);
}

int Memory::getMemSizeByFloat(void) const
{
	return _wzyxLen;
}
