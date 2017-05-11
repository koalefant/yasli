#include <string>
#include <vector>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "yasli/STL.h"
#include "yasli/Pointers.h"
#include "yasli/StringList.h"
#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"

#include "yasli/JSONOArchive.h"
#include "yasli/JSONIArchive.h"
#include "yasli/TextIArchive.h"
#include "yasli/TextOArchive.h"
#include "yasli/BinArchive.h"
#include "yasli/Enum.h"
#include "../yasli-test/ComplexClass.h"

using namespace yasli;

struct AutoTimer
{
	unsigned int startTime_;

	AutoTimer()
	{
		startTime_ = clock() * 1000 / CLOCKS_PER_SEC;
	}

	int result() const
	{
		return clock() * 1000 / CLOCKS_PER_SEC - startTime_;
	}

	~AutoTimer() {
	}
};

int getFileSize(const char* filename)
{
#ifdef WIN32
	struct _stat64 desc;
	if (_stat64(filename, &desc) != 0)
    return -1;
#else
  struct stat desc;
  if (stat(filename, &desc) != 0)
    return -1;
#endif

  return (int)desc.st_size;
}

void testText()
{
}

struct DataSetMixed {
	std::vector<ComplexClass> arr;

	DataSetMixed() : arr(10000) {}
	
	void serialize(Archive& ar) {
		ar(arr, "arr");
	}
};

void print_result(const char* name, std::size_t size, AutoTimer& timer) {
	int duration = std::max(1, timer.result());
	printf("%s:\t%g MByte/s\t[%d ms,\t%g MBytes]\n", name, size / 1024.0 / 1024.0 / duration * 1000.0, duration, size / 1024.0 / 1024.0);
}

void benchmark_mixed_json_write() {
	DataSetMixed object;
	AutoTimer timer;
	JSONOArchive oa;
	oa(object, "");

	print_result(__FUNCTION__, oa.length(), timer);
}

void benchmark_mixed_copy() {
	DataSetMixed temp;

	AutoTimer timer;

	DataSetMixed copy = temp;
	print_result(__FUNCTION__, sizeof(copy), timer);
}

void benchmark_mixed_json_read() {
	DataSetMixed temp;
	JSONOArchive oa;
	oa(temp, "");

	DataSetMixed object;

	AutoTimer timer;

	JSONIArchive ia;
	ia.setDisableWarnings(true);
	ia.setWarnAboutUnusedFields(false);
	ia.open(oa.c_str(), oa.length());
	ia(object, "");
	
	print_result(__FUNCTION__, oa.length(), timer);
}

void benchmark_mixed_text_write() {
	DataSetMixed object;
	AutoTimer timer;
	TextOArchive oa;
	oa(object, "");

	print_result(__FUNCTION__, oa.length(), timer);
}

void benchmark_mixed_text_read() {
	DataSetMixed temp;
	TextOArchive oa;
	oa(temp, "");

	DataSetMixed object;

	AutoTimer timer;

	TextIArchive ia;
	ia.open(oa.c_str(), oa.length());
	ia(object, "");
	
	print_result(__FUNCTION__, oa.length(), timer);
}


void benchmark_mixed_bin_write() {
	DataSetMixed object;
	AutoTimer timer;
	BinOArchive oa;
	oa(object, "");

	print_result(__FUNCTION__, oa.length(), timer);
}

void benchmark_mixed_bin_read() {
	DataSetMixed temp;
	BinOArchive oa;
	oa(temp, "");

	DataSetMixed object;

	AutoTimer timer;

	BinIArchive ia;
	ia.open(oa.buffer(), oa.length());
	ia(object, "");
	
	print_result(__FUNCTION__, oa.length(), timer);
}

int main(int argc, char** argv)
{
	benchmark_mixed_copy();

	benchmark_mixed_json_write();
	benchmark_mixed_json_read();

	benchmark_mixed_text_write();
	benchmark_mixed_text_read();

	benchmark_mixed_bin_write();
	benchmark_mixed_bin_read();

	return 0;
}
