#pragma once

#include "round.h"

class Archive;
struct Color3c;

struct Color4f
{
	float r,g,b,a;
	
	Color4f() {}
	Color4f(float _r, float _g, float _b, float _a = 1.0f)
    : r(_r), g(_g), b(_b), a(_a) {}
	explicit Color4f(const struct Color4c& color);

	void set(float _r, float _g, float _b, float _a) { r = _r; g = _g; b = _b; a = _a; }

	void setHSV(float h,float s,float v, float alpha = 1.f);

	Color4f& operator+= (const Color4f &color)	{ r+=color.r; g+=color.g; b+=color.b; a+=color.a; return *this; }
	Color4f& operator-= (const Color4f &color)	{ r-=color.r; g-=color.g; b-=color.b; a-=color.a; return *this; }
	Color4f& operator*= (const Color4f &color)	{ r*=color.r; g*=color.g; b*=color.b; a*=color.a; return *this; }
	Color4f& operator*= (float f)			{ r*=f; g*=f; b*=f; a*=f; return *this; }
	Color4f& operator/= (float f)			{ if(f!=0) f=1/f; else f=0.001f; r*=f; g*=f; b*=f; a*=f; return *this; }
	Color4f	operator+ (const Color4f &color) const	{ Color4f tmp(r+color.r,g+color.g,b+color.b,a+color.a); return tmp; }
	Color4f	operator- (const Color4f &color) const	{ Color4f tmp(r-color.r,g-color.g,b-color.b,a-color.a); return tmp; }
	Color4f	operator* (const Color4f &color) const	{ Color4f tmp(r*color.r,g*color.g,b*color.b,a*color.a); return tmp; }
	Color4f	operator* (float f) const 		{ Color4f tmp(r*f,g*f,b*f,a*f); return tmp; }
	Color4f	operator/ (float f) const 		{ if(f!=0.f) f=1/f; else f=0.001f; Color4f tmp(r*f,g*f,b*f,a*f); return tmp; }
	void mul3(const Color4f& x,const Color4f& y){r=x.r*y.r;g=x.g*y.g;b=x.b*y.b;}

	int R() const 						{ return round(255*r); }
	int G() const 						{ return round(255*g); }
	int B() const 						{ return round(255*b); }
	int A() const 						{ return round(255*a); }
	unsigned int RGBA() const 						{ return (round(255*r) << 16) | (round(255*g) << 8) | round(255*b) | (round(255*a) << 24); }
	unsigned int GetRGB() const 					{ return (round(255*r) << 16) | (round(255*g) << 8) | round(255*b); }
	unsigned int RGBGDI() const 					{ return round(255*r) | (round(255*g) << 8) | (round(255*b) << 16); }
	void interpolate(const Color4f &u,const Color4f &v,float f) { r=u.r+(v.r-u.r)*f; g=u.g+(v.g-u.g)*f; b=u.b+(v.b-u.b)*f; a=u.a+(v.a-u.a)*f; }
	void interpolate3(const Color4f &u,const Color4f &v,float f) { r=u.r+(v.r-u.r)*f; g=u.g+(v.g-u.g)*f; b=u.b+(v.b-u.b)*f; }

	void serialize(Archive& ar);
};

struct Color4c
{
	union{
		struct{ unsigned char b,g,r,a; };
		struct{ unsigned long argb; };
	};
	
	Color4c()										{ }
	Color4c(const Color4f& color)					{ set(color.R(),color.G(),color.B(),color.A()); }
	Color4c(const Color3c& color);
	Color4c(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a=255) { r=_r; g=_g; b=_b; a=_a; }
	explicit Color4c(unsigned long _argb) { argb=_argb; }
	void set(int rc,int gc,int bc,int ac=255)	{ r=rc; g=gc; b=bc; a=ac; }
	void set(const Color4f& color)			{ set(color.R(),color.G(),color.B(),color.A()); }
	
	Color4c& setSafe(const Color4f& color)		{ set(clamp(color.R(),0,255), clamp(color.G(),0,255), clamp(color.B(),0,255), clamp(color.A(),0,255)); return *this; }
	Color4c& setSafe1(const Color4f& color)		{ set(clamp(color.R(),0,255), clamp(color.G(),0,255), clamp(color.B(),0,255), clamp(color.A(),0,255)); return *this; }

	Color4c& setGDI(unsigned long color);
	void setHSV(float h,float s,float v, unsigned char alpha = 255);

	Color4c& operator *= (float f)			{ r=round(r*f); g=round(g*f); b=round(b*f); a=round(a*f); return *this; }
	Color4c& operator += (Color4c &p)		{ r+=p.r; g+=p.g; b+=p.b; a+=p.a; return *this; }
	Color4c& operator -= (Color4c &p)		{ r-=p.r; g-=p.g; b-=p.b; a-=p.a; return *this; }
	Color4c operator + (Color4c &p)		{ return Color4c(r+p.r,g+p.g,b+p.b,a+p.a); }
	Color4c operator - (Color4c &p)		{ return Color4c(r-p.r,g-p.g,b-p.b,a-p.a); }
	Color4c operator * (int f) const 		{ return Color4c(r*f,g*f,b*f,a*f); }
	Color4c operator / (int f) const 		{ if(f!=0) f=(1<<16)/f; else f=1<<16; return Color4c((r*f)>>16,(g*f)>>16,(b*f)>>16,(a*f)>>16); }
	
	bool operator==(const Color4c& rhs) const{ return argb == rhs.argb; }
	bool operator!=(const Color4c& rhs) const{ return argb != rhs.argb; }
	
	unsigned int  RGBA() const 						{ return ((const unsigned int*)this)[0]; }
	unsigned int& RGBA()							{ return ((unsigned int*)this)[0]; }
	unsigned int RGBGDI() const 					{ return r | g << 8 | b << 16; }
	void HSV(float& h,float& s,float& v);
	unsigned char& operator[](int i)				{ return ((unsigned char*)this)[i];}
	void interpolate(const Color4c &u,const Color4c &v,float f) { r=round(u.r+int(v.r-u.r)*f); g=round(u.g+int(v.g-u.g)*f); b=round(u.b+int(v.b-u.b)*f); a=round(u.a+(v.a-u.a)*f); }
	void serialize(Archive& ar);

};

struct Color3c
{
	unsigned char b,g,r;
	
	Color3c()										{ }
	Color3c(int rc,int gc,int bc)                  { r=rc; g=gc; b=bc; }
	explicit Color3c(const Color4f& color)					{ set(color.R(),color.G(),color.B()); }
	explicit Color3c(const Color4c& color)					{ set(color.r, color.g, color.b); }

	inline void set(int rc,int gc,int bc)	{ r=rc; g=gc; b=bc; }
	inline void set(const Color4f& color)			{ set(color.R(),color.G(),color.B()); }
	inline Color3c& operator *= (float f)			{ r=round(r*f); g=round(g*f); b=round(b*f); return *this; }
	inline Color3c& operator += (Color3c &p)		{ r+=p.r; g+=p.g; b+=p.b; return *this; }
	inline Color3c& operator -= (Color3c &p)		{ r-=p.r; g-=p.g; b-=p.b; return *this; }
	inline Color3c operator + (Color3c &p)		{ return Color3c(r+p.r,g+p.g,b+p.b); }
	inline Color3c operator - (Color3c &p)		{ return Color3c(r-p.r,g-p.g,b-p.b); }
	inline Color3c operator * (int f) const 		{ return Color3c(r*f,g*f,b*f); }
	inline Color3c operator / (int f) const 		{ if(f!=0) f=(1<<16)/f; else f=1<<16; return Color3c((r*f)>>16,(g*f)>>16,(b*f)>>16); }
	unsigned char& operator[](int i)				{ return ((unsigned char*)this)[i];}
	void interpolate(const Color3c &u,const Color3c &v,float f) { r=round(u.r+int(v.r-u.r)*f); g=round(u.g+int(v.g-u.g)*f); b=round(u.b+int(v.b-u.b)*f); }
	void serialize(Archive& ar);
};


inline Color4f::Color4f(const Color4c& color) 
{ 
	r = color.r/255.f; 
	g = color.g/255.f; 
	b = color.b/255.f; 
	a = color.a/255.f; 
}

inline Color4c::Color4c(const Color3c& color)
{
	set(color.r,color.g,color.b,255);
}
