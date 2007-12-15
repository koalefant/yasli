#include "StdAfx.h"
#include "uMath.h"
#include "uMath/uMath.h"

#include "yasli/uMath.h"
#include "yasli/Archive.h"

struct Mat2x3Serializeable : public Mat2x3{
    void serialize(Archive& ar){
        if(ar.isEdit()){
            ar(trans, "translation");
        }
        else{
            ar(x, "");
            ar(y, "");
            ar(trans, "");
        }
    }
};


bool serialize(Archive& ar, Mat2x3& mat, const char* name)
{
    return ar(static_cast<Mat2x3Serializeable&>(mat), name);
}

struct RectiSerializeable : public Recti{
    void serialize(Archive& ar){
        ar(min, "");
        ar(max, "");
    }
};

bool serialize(Archive& ar, Recti& rect, const char* name)
{
    return ar(static_cast<RectiSerializeable&>(rect), name);
}

struct Vect2iSerializeable : public Vect2i{
    void serialize(Archive& ar){
        ar(x, "");
        ar(y, "");
    }
};

bool serialize(Archive& ar, Vect2i& vect, const char* name)
{
    return ar(static_cast<Vect2iSerializeable&>(vect), name);
}

struct Vect2fSerializeable : public Vect2f{
    void serialize(Archive& ar){
        ar(x, "");
        ar(y, "");
    }
};

bool serialize(Archive& ar, Vect2f& vect, const char* name)
{
    return ar(static_cast<Vect2fSerializeable&>(vect), name);
}
