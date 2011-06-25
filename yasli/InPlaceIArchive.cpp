#include "StdAfx.h"
#include "InPlaceIArchive.h"

namespace yasli {

InPlaceIArchive::InPlaceIArchive()
{

}

const void* InPlaceIArchive::load(size_t rootSize, const char* filename)
{
	FILE* file = fopen(filename, "rb");
	if(!file)
		return 0;
  fseek(file, 0, SEEK_END);
  size_t fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

	ESCAPE(fileSize >= rootSize, return 0);

	char header[8];
	if (fread(header, sizeof(header), 1, file) != 1)
		return 0;

	if (memcmp(header, "YINPA32", sizeof(header)) != 0)
		return 0;

	size_t offsetsSize = 0;
	if (fread(&offsetsSize, sizeof(offsetsSize), 1, file) != 1)
		return 0;

	ESCAPE(offsetsSize < 1024 * 1024 * 1024, return 0);
	std::vector<size_t> pointerOffsets;
	pointerOffsets.resize(offsetsSize / sizeof(size_t));
	if (!pointerOffsets.empty())
	{
		if (fread(&pointerOffsets[0], sizeof(size_t), pointerOffsets.size(), file) != pointerOffsets.size())
			return 0;
	}

	size_t dataSize = 0;
	if (fread(&dataSize, sizeof(dataSize), 1, file) != 1)
		return 0;

	char* data = (char*)malloc(dataSize);
	if(fread(data, dataSize, 1, file) != 1)
	{
		fclose(file);
		free(data);
		return 0;
	}
	fclose(file);

	// hydrate pointers
	size_t numPointers = pointerOffsets.size();
	for (size_t i = 0; i < numPointers; ++i)
	{
		size_t pointerOffset = pointerOffsets[i];
		ESCAPE(pointerOffset <= dataSize, return 0);
		size_t offset = (size_t&)data[pointerOffset];
		reinterpret_cast<char*&>(data[pointerOffset]) = data + offset;
	}
	return data;
}

}
