#include "memhub.h"

void MemHub::addMemory(std::string memName, int xLen, int yLen, int zLen, int wLen)
{
	_memories.emplace(memName, Memory(xLen, yLen, zLen, wLen));
}

Memory& MemHub::getMemory(std::string memName)
{
	return _memories.at(memName);
}
