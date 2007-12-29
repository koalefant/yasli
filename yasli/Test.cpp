#include "StdAfx.h"
#include "testo/Testo.h"
#include <math.h>

#ifndef M_PI
# define M_PI 3.1415926
#endif

#include "yasli/Pointers.h"
#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/STLImpl.h"
#include "yasli/PointersImpl.h"

#include "yasli/TextIArchive.h"
#include "yasli/TextOArchive.h"

#include "yasli/TypesFactory.h"
#include "utils/MemoryWriter.h"

TESTO_BEGIN()


/*{{{*/
enum TestEnum{
    TEST_VALUE_2,
    TEST_NONE,
    TEST_VALUE_1
};

struct TestChunk{
    TestChunk(){
        type_ = TEST_NONE; 
        a = 0.0f;
        b = 0.0f;
        enabled_ = false;
    }

    void fill(int index = 0)
    {
        enabled_ = true;
        type_ = TEST_VALUE_1;
        MemoryWriter name;
        name << "element " << index;
        name_ = name.c_str();
        ++index;
    }

    void serialize(Archive& ar){
        TESTO_ENSURE(ar(name_));
        TESTO_ENSURE(ar(type_, "type"));
        TESTO_ENSURE(ar(enabled_));
        TESTO_ENSURE(ar(a, "absolute"));
        TESTO_ENSURE(ar(b, "balance"));
    }

    void verifyCopy(const TestChunk& rhs) const{
        TESTO_ENSURE(type_ == rhs.type_);
        TESTO_ENSURE(name_ == rhs.name_);
        TESTO_ENSURE(enabled_ == rhs.enabled_);
        TESTO_ENSURE(fabs(a - rhs.a) < 1e-5f);
        TESTO_ENSURE(fabs(b - rhs.b) < 1e-5f);
    }

    TestEnum type_;
    std::string name_;
    bool enabled_;
    float a;
    float b;
};

struct TestMember{
    TestMember()
    {
    }
    void fill(){
        TestChunk chunk;
        chunk.a = 1.0f;
        chunks_.push_back(chunk);

        {
            TestChunk chunk;
            chunk.b = 1.0f;
            chunks_.push_back(chunk);
        }

        {
            TestChunk chunk;
            chunk.a = 2.0f;
            chunks_.push_back(chunk);
        }

        {
            TestChunk chunk;
            chunk.b = 4.0f;
            chunks_.push_back(chunk);
        }
    }
    void serialize(Archive& ar){
        ar(chunks_);
        ar(fixedChunks_, "fixedChunks");
    }
    bool operator==(const TestMember& rhs) const{
        return true;
    }
    void verifyCopy(const TestMember& rhs) const{
        TESTO_ENSURE(chunks_.size() == rhs.chunks_.size());
        if((chunks_.size() == rhs.chunks_.size()))
            for(std::size_t i = 0; i < chunks_.size(); ++i)
                chunks_[i].verifyCopy(rhs.chunks_[i]);
        for(std::size_t i = 0; i < 10; ++i)
            fixedChunks_[i].verifyCopy(rhs.fixedChunks_[i]);
    }
    std::vector<TestChunk> chunks_;
    TestChunk fixedChunks_[10];
};

class PolyBase : public RefCounter{
public:
    virtual ~PolyBase() {}
    PolyBase()
    :a(0)
    {
    }
    virtual void fill(){
        a = 13;
    }
    virtual void serialize(Archive& ar){
        ar(a, "a");
    }
    virtual void verifyCopy(const PolyBase& rhs) const{
        bool sameType = TypeID::get(&rhs) == TypeID::get(this);
        TESTO_ENSURE(sameType);
        if(sameType)
            TESTO_ENSURE(a == rhs.a);
    }
    int a;
};

class PolyDerived1 : public PolyBase{
public:
    // virtuals:
    PolyDerived1(){
        name_ = "Derived class 1\'Undesired test\'";
    }
    void serialize(Archive& ar){
        ar(name_);
        PolyBase::serialize(ar);
        ar(label_, "label");
        ar(chunk_, "chunk");
    }
    void verifyCopy(const PolyBase& rhs) const{
        bool sameType = TypeID::get(&rhs) == TypeID::get(this);
        TESTO_ENSURE(sameType);
        if(sameType){
            const PolyDerived1& r = *safe_cast<const PolyDerived1*>(&rhs);
            TESTO_ENSURE(name_ == r.name_);
            TESTO_ENSURE(a == r.a);
            TESTO_ENSURE(label_ == r.label_);
            chunk_.verifyCopy(r.chunk_);
        }
    }
protected:
    TestChunk chunk_;
    std::string name_;
    std::string label_;
};

class PolyDerived2 : public PolyBase{
public:
    // virtuals:
    PolyDerived2(){
        name_ = "Derived\"class 2";
        width_ = 0;
        height_ = 0;
    }
    void serialize(Archive& ar){
        ar(name_, "");
        PolyBase::serialize(ar);
        ar(width_, "width");
        ar(height_, "height");
    }
    void verifyCopy(const PolyBase& rhs) const{
        bool sameType = TypeID::get(&rhs) == TypeID::get(this);
        TESTO_ENSURE(sameType);
        if(sameType){
            const PolyDerived2& r = *safe_cast<const PolyDerived2*>(&rhs);
            TESTO_ENSURE(name_ == r.name_);
            TESTO_ENSURE(a == r.a);
            TESTO_ENSURE(width_ == r.width_);
            TESTO_ENSURE(height_ == r.height_);
        }
    }
protected:
    std::string name_;
    int width_;
    int height_;
};

SERIALIZATION_TYPE(TestChunk, "TestChunk");

SERIALIZATION_TYPE(PolyBase, "PolyBase");
SERIALIZATION_DERIVED_TYPE(PolyBase, PolyDerived1, "PolyDerived1");
SERIALIZATION_DERIVED_TYPE(PolyBase, PolyDerived2, "PolyDerived2");


SERIALIZATION_ENUM_BEGIN(TestEnum, "TestEnumeration")
SERIALIZATION_ENUM_VALUE(TEST_NONE, "TEST_NONE")
SERIALIZATION_ENUM_VALUE(TEST_VALUE_1, "TEST_VALUE_1")
SERIALIZATION_ENUM_VALUE(TEST_VALUE_2, "TEST_VALUE_2")
SERIALIZATION_ENUM_END()

class ComplexData{
public:
    ComplexData()
    : name_("UnitializedName")
    , scale_(-1.0f)
    {

    }

    void fill(){
        name_ = "Named data";
        scale_ = 3.0f;
        member_.fill();
        pointers_.push_back(new PolyDerived1);
        for(int i = 0; i < 10; ++i)
            fixedChunks_[i].fill(i);
    }

    template<class T, unsigned int Size>
    static void testFunc(T array[Size]){
        std::cout << "DONE!" << std::endl;
    }

    void serialize(Archive& ar){
		TESTO_ENSURE(ar(fixedChunks_, "fixedChunks"));
		TESTO_ENSURE(ar(name_, "name"));

		TESTO_ENSURE(ar(scale_, "scale"));
		TESTO_ENSURE(ar(member_, "member"));
		TESTO_ENSURE(ar(pointers_, "pointers"));
		TESTO_ENSURE(ar(strings_, "strings"));
    }

    TestMember member_;

    std::vector< SharedPtr<PolyBase> >  pointers_;
    std::vector< std::string > strings_;

    TestChunk fixedChunks_[10];

    std::string name_;
    float scale_;

    void verifyCopy(const ComplexData& rhs) const{
        member_.verifyCopy(rhs.member_);
        //if(!(pointers_ == rhs.pointers_))
        TESTO_ENSURE(strings_ == rhs.strings_);
        for(int i = 0; i < 10; ++i)
            fixedChunks_[i].verifyCopy(rhs.fixedChunks_[i]);
        TESTO_ENSURE(name_ == rhs.name_);
        TESTO_ENSURE(fabs(scale_ - rhs.scale_) < 1e-5f);
    }
};
SERIALIZATION_TYPE(ComplexData, "ComplexData");
/*}}}*/

struct TextIOComplex{
    void invoke(){
        TESTO_ENSURE(TypeID::get<ComplexData>().registered());
        TESTO_ENSURE(TypeID::get<PolyBase>().registered());
        TESTO_ENSURE(TypeID::get<PolyDerived1>().registered());
        TESTO_ENSURE(TypeID::get<PolyDerived2>().registered());
        TESTO_ENSURE(TypeID::get<TestChunk>().registered());
        TESTO_ENSURE(getEnumDescription<TestEnum>().registered());

        const char* filename = "!testData.textarchive";
        {
            ComplexData data;
            data.fill();

            TextOArchive oa;
            oa.open(filename);
            oa(data, "data");
        }

        ComplexData reference;
        reference.fill();
        {
            ComplexData data;
            TextIArchive ia;
            ia.open(filename);
            ia(data, "data");

            data.verifyCopy(reference);
        }
    }
};

TESTO_ADD_TEST("Serialization", TextIOComplex)
// ---------------------------------------------------------------------------

struct TestEnumData{/*{{{*/
    TestEnumData()
    : value1(TEST_NONE)
    , value2(TEST_NONE)
    {
    }

    void fill()
    {
        value1 = TEST_VALUE_1;
        value2 = TEST_VALUE_2;
        values.resize(5);
        for(std::size_t i = 0; i < values.size(); ++i)
            values[i] = i % 2 ? TEST_VALUE_2 : TEST_VALUE_1;
    }

    void serialize(Archive& ar){
        ar(value1, "value1");
        ar(value2, "value2");
        ar(values, "values");
    }

    TestEnum value1;
    TestEnum value2;
    std::vector<TestEnum> values;
};/*}}}*/

struct TextIOEnum{
    void invoke(){
        TESTO_ENSURE(getEnumDescription<TestEnum>().registered());

        const char* filename = "!testData.textarchive";
        {
            TestEnumData data;
            data.fill();

            TextOArchive oa;
            oa.open(filename);
            oa(data, "data");
        }

        TestEnumData reference;
        reference.fill();
        {
            TestEnumData data;
            TextIArchive ia;
            ia.open(filename);
            ia(data, "data");

            TESTO_ENSURE(data.value1 == reference.value1 && data.value2 == reference.value2);
            TESTO_ENSURE(data.values == reference.values);
        }
    }
};

TESTO_ADD_TEST("Serialization", TextIOEnum)
// ---------------------------------------------------------------------------

struct TestDataReorder{

    TestDataReorder()
    : enumValue(TEST_NONE)
    , scale(0.0f)
    {
    }

    void fill(){
        name = "Unique Name";
        scale = float(M_PI);
        strings.clear();
        strings.push_back("First string");
        strings.push_back("Another one");
        strings.push_back("And last");
        enumValue = TEST_VALUE_2;
    }

    void serialize(Archive& ar)
    {
        if(ar.isOutput()){
            TESTO_ENSURE(ar(enumValue, "enumValue"));
            TESTO_ENSURE(ar(name, ""));
            TESTO_ENSURE(ar(strings, "strings"));
            TESTO_ENSURE(ar(scale, ""));
        }
        else{
            TESTO_ENSURE(ar(name, ""));
            TESTO_ENSURE(ar(scale, ""));
            TESTO_ENSURE(ar(strings, "strings"));
            TESTO_ENSURE(ar(enumValue, "enumValue"));
        }
    }

    void verifyCopy(const TestDataReorder& rhs) const{
        TESTO_ENSURE(name == rhs.name);
        TESTO_ENSURE(enumValue == rhs.enumValue);
        TESTO_ENSURE(strings == rhs.strings);
        TESTO_ENSURE(fabs(scale - rhs.scale) < 1e-5f);
    }

    std::string name;
    TestEnum enumValue;
    std::vector<std::string> strings;
    float scale;
};

struct TextIOReorder{
    typedef TestDataReorder DataType;
    void invoke(){
        const char* filename = "!testData.textarchive";
        {
            DataType data;
            data.fill();

            TextOArchive oa;
            oa.open(filename);
            oa(data, "data");
        }

        DataType reference;
        reference.fill();
        {
            DataType data;
            TextIArchive ia;
            ia.open(filename);
            ia(data, "data");

            data.verifyCopy(reference);
        }
    }
};

TESTO_ADD_TEST("Serialization", TextIOReorder)


struct TestDataUnnamedNegativeFloat{
    float x, y, z;
    TestDataUnnamedNegativeFloat()
    : x(0.0f), y(0.0f), z(0.0f)
    {
    }

    void fill(){
        z = -10.0f;
    }

    void serialize(Archive& ar){
        ar(x);
        ar(y);
        ar(z);
    }

    void verifyCopy(TestDataUnnamedNegativeFloat& rhs){
        const float FLT_COMPARE_TOLERANCE = 1e-5f;
        TESTO_ENSURE(fabs(x - rhs.x) < FLT_COMPARE_TOLERANCE);
        TESTO_ENSURE(fabs(y - rhs.y) < FLT_COMPARE_TOLERANCE);
        TESTO_ENSURE(fabs(z - rhs.z) < FLT_COMPARE_TOLERANCE);
    }
};

struct TextIOUnnamedNegativeFloat{
    typedef TestDataUnnamedNegativeFloat DataType;
    static void invoke()
    {
        const char* filename = "!testData.textarchive";
        {
            DataType data;
            data.fill();

            TextOArchive oa;
            oa.open(filename);
            oa(data, "data");
        }

        DataType reference;
        reference.fill();
        {
            DataType data;
            TextIArchive ia;
            ia.open(filename);
            ia(data, "data");

            data.verifyCopy(reference);
        }
    }
};

TESTO_ADD_TEST("Serialization", TextIOUnnamedNegativeFloat)
// ---------------------------------------------------------------------------

struct TestDataPointers{

    void fill()
    {
        pointer = new PolyDerived1;
    }

    void serialize(Archive& ar){
        ar(pointer, "pointer");
    }

    void verifyCopy(const TestDataPointers& rhs) const{
        bool both = !pointer && !rhs.pointer || pointer && rhs.pointer;
        TESTO_ENSURE(both);
        if(both && pointer){
            pointer->verifyCopy(*rhs.pointer);
        }
    }

    SharedPtr<PolyBase> pointer;

};

struct Pointers{
    typedef TestDataPointers DataType;
    void invoke(){
        const char* filename = "!testData.textarchive";
        {
            DataType data;
            data.fill();

            TextOArchive oa;
            oa.open(filename);
            oa(data, "data");
        }

        DataType reference;
        reference.fill();
        {
            DataType data;
            TextIArchive ia;
            ia.open(filename);
            ia(data, "data");
            TESTO_ENSURE(data.pointer != 0);
            data.verifyCopy(reference);
        }
    }
};

TESTO_ADD_TEST("Serialization", Pointers)
// ---------------------------------------------------------------------------
//
struct TextIAddLastElementData{
    TextIAddLastElementData(bool newFormat)
    : newFormat_(newFormat){}
    void serialize(Archive& ar){
        ar(name, "name");
        if(newFormat_)
            ar(name1, "name");
    }
protected:

    std::string name[6];
    std::string name1;

    bool newFormat_;
};

struct TextIAddLastElement{
    typedef TextIAddLastElementData DataType;
    void invoke(){
        const char* filename = "!testData.textarchive";
        {
            DataType data(false);
            TextOArchive oa;
            oa.open(filename);
            data.serialize(oa);
        }

        {
            DataType data(true);
            TextIArchive ia;
            ia.open(filename);
            data.serialize(ia);
        }
    }
};
TESTO_ADD_TEST("Serialization", TextIAddLastElement)
// ---------------------------------------------------------------------------

TESTO_END()
