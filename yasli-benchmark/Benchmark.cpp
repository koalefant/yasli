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

template<class F>
struct LeafWriter {
	F func;
	template<class T, class A>
	void operator()(const std::vector<T, A>& vec) {
		int num = vec.size();
		// printf("ovector %d\n", num);
		operator()(num);
		//for(int i = 0; i < num; ++i)
		//	operator()(vec[i]);
		operator()(vec.data(), num);
	}
	template<class C>
	void operator()(const std::basic_string<C>& str) {
		int num = str.size();
		// printf("ostring %d\n", num);
		operator()(num);
		//for(int i = 0; i < num; ++i)
		//	operator()(str[i]);
		operator()(str.data(), num);
	}
	void operator()(const StringListValue& str) {
		int val = str.index();
		//operator()(val);
	}
	template<class A, class B>
	void operator()(const std::pair<A, B>& p) {
		operator()(p.first);
		operator()(p.second);
	}
	template<class A, class B>
	void operator()(const std::map<A, B>& values) {
		int num = values.size();
		operator()(num);
		for (auto& v: values) {
			operator()(v.first);
			operator()(v.second);
		}
	}
	void operator()(const SharedPtr<PolyBase>& p) {
		if (p.get()){
			unsigned char type = type = p->typeIndex;
			operator()(type);
			switch (type) {
			case 0:
			operator()(*p);
			break;
			case 1:
			operator()(static_cast<PolyDerivedA&>(*p));
			break;
			case 2:
			operator()(static_cast<PolyDerivedB&>(*p));
			break;
			}
		} else {
			unsigned char type = 0xff;
			operator()(type);
		}
	}
	template<class T, int num>
	void operator()(const T(&p)[num]) {
		// printf(" o %s[%d]\n", yasli::TypeID::get<T>().name(), num);
		//func(&p[0], num);
		for(int i = 0; i < num; ++i)
			operator()(p[i]);
	}
	template<class T>
	void operator()(const T* p, int num) {
		// printf(" o %s[%d]\n", yasli::TypeID::get<T>().name(), num);
		if(std::is_arithmetic<T>::value) {
			func(p[0], num);
		} else {
			for(int i = 0; i < num; ++i)
				operator()(p[i]);
		}
	}
	template<class T>
	typename std::enable_if<!std::is_arithmetic<T>::value, void>::type
	operator()(const T& var) {
		// printf("o %s\n", yasli::TypeID::get<T>().name());
		const_cast<T&>(var).visit(*this);
	}
	template<class T>
	typename std::enable_if<std::is_arithmetic<T>::value, void>::type
	operator()(const T& var) {
		// printf(" o %s\n", yasli::TypeID::get<T>().name());
		func(var, 1);
	}
};

template<class F>
struct LeafReader {
	F func;
	template<class T, class A>
	void operator()(std::vector<T, A>& vec) {
		int num = 0;
		operator()(num);
		vec.resize(num);
		// printf("ivector %d\n", num);
		operator()(vec.data(), num);
	}
	template<class C>
	void operator()(std::basic_string<C>& str) {
		int num = 0;
		operator()(num);
		// printf("istring %d\n", num);
		YASLI_ASSERT(num < 0xffff);
		if(num > 0xffff)
			return;
		str.resize(num);
		C* ptr = (C*)str.data();
		operator()(ptr, num);
	}
	void operator()(StringListValue& str) {
		int val = 0;
		//operator()(val);
		//str = val;
	}
	template<class A, class B>
	void operator()(std::pair<A, B>& p) {
		operator()(p.first);
		operator()(p.second);
	}
	template<class A, class B>
	void operator()(std::map<A, B>& values) {
		int num = 0;
		operator()(num);
		for(int i = 0; i < num; ++i){
			A key{};
			operator()(key);
			operator()(values[key]);
		}
	}
	void operator()(SharedPtr<PolyBase>& p) {
		unsigned char type = 0xff;
		operator()(type);
		switch (type) {
		case 0xff:
		p = nullptr;
		break;
		case 0:
		p.reset(new PolyBase());
		operator()(*p);
		break;
		case 1:
		p.reset(new PolyDerivedA());
		operator()(static_cast<PolyDerivedA&>(*p));
		break;
		case 2:
		p.reset(new PolyDerivedB());
		operator()(static_cast<PolyDerivedB&>(*p));
		break;
		}
	}
	template<class T, int num>
	void operator()(T(&p)[num]) {
		operator()(&p[0], num);
	}
	template<class T>
	void operator()(T* p, int num) {
		if (std::is_arithmetic<T>::value) {
			func(p[0], num);
		} else {
			for(int i = 0; i < num; ++i)
				operator()(p[i]);
		}
	}
	template<class T>
	typename std::enable_if<!std::is_arithmetic<T>::value, void>::type
	operator()(T& var) {
		// printf("i %s\n", yasli::TypeID::get<T>().name());
		var.visit(*this);
	}
	template<class T>
	typename std::enable_if<std::is_arithmetic<T>::value, void>::type
	operator()(T& var) {
		// printf(" i %s\n", yasli::TypeID::get<T>().name());
		func(var, 1);
	}
};

struct BinaryWriter {
	std::vector<char> buffer;
	template<class T>
	void operator()(const T& value, int num) {
		static_assert(!std::is_pointer<T>::value, "no pointers");
		buffer.insert(buffer.end(), (const char*)&value, (const char*)(&value+num));
	}
};

struct BinaryReaderFixed {
	const char* in{ nullptr };
	const char* in_end{ nullptr };
	template<class T>
	void operator()(T& value, int num) {
		static_assert(!std::is_pointer<T>::value, "no pointers");
		memcpy((void*)&value, in, sizeof(T) * num);
		YASLI_ASSERT(in + sizeof(T) * num <= in_end);
		in += sizeof(T) * num;
	}
};

struct BinaryWriterFixed {
	char* out{ nullptr };
	template<class T>
	void operator()(const T& value, int num) {
		static_assert(!std::is_pointer<T>::value, "no pointers");
		memcpy(out, &value, sizeof(T) * num);
		out += sizeof(T) * num;
	}
};

struct BinaryMeasure {
	int size{ 0 };
	template<class T>
	void operator()(const T& value, int num) {
		static_assert(!std::is_pointer<T>::value, "no pointers");
		size += sizeof(T) * num;
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

	DataSetMixed() : arr(50000) {}
	
	void serialize(Archive& ar) {
		ar(arr, "arr");
	}

	template<class Visitor>
	void visit(Visitor& v) {
		v(arr);
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

void benchmark_mixed_bin_naive_write() {
	DataSetMixed object;
	AutoTimer timer;
	LeafWriter<BinaryWriter> oa;
	oa.func.buffer.reserve(1024);
	oa(object);

	print_result(__FUNCTION__, oa.func.buffer.size(), timer);
}

void benchmark_mixed_bin_two_pass_write() {
	DataSetMixed object;
	AutoTimer timer;
	LeafWriter<BinaryMeasure> calc;
	calc(object);
	int size = calc.func.size;
	std::vector<char> buffer;
	buffer.resize(size);
	LeafWriter<BinaryWriterFixed> oa;
	oa.func.out = buffer.data();
	oa(object);

	print_result(__FUNCTION__, size, timer);
	YASLI_ASSERT(oa.func.out == buffer.data() + buffer.size());
}

void benchmark_mixed_bin_naive_read() {
	std::vector<char> buffer;
	DataSetMixed object;
	object.arr[0].change();
	{
		LeafWriter<BinaryMeasure> calc;
		calc(object);
		int size = calc.func.size;
		buffer.resize(size);
		LeafWriter<BinaryWriterFixed> oa;
		oa.func.out = buffer.data();
		oa(object);
	}
	DataSetMixed object_read;
	AutoTimer timer;
	LeafReader<BinaryReaderFixed> ia;
	ia.func.in = buffer.data();
	ia.func.in_end = buffer.data() + buffer.size();
	ia(object_read);
	print_result(__FUNCTION__, buffer.size(), timer);
	object.arr[0].checkEquality(object_read.arr[0]);
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

	benchmark_mixed_bin_naive_write();
	benchmark_mixed_bin_two_pass_write();
	benchmark_mixed_bin_naive_read();

	return 0;
}
