#include "StdAfx.h"
#include "Color.h"
#include "yasli/Archive.h"
#include <math.h>

namespace ww {

inline int round(float v)
{
	return int(v);
}

// HSV
//Y = 0.30*R + 0.59*G + 0.11*B ������� �������� � �����-�����
//h=0..360,s=0..1,v=0..1

inline void HSVtoRGB(float h,float s,float v,
					 float& r,float& g,float& b)
{
	const float min=1e-5f;
	int i;
	float f,m,n,k;

	if(s<min){
		r=g=b=v;
	}
	else {
		if(h>=360.0f)
			h=0;
		else
			h=h/60.0f;

		i=round(floor(h));
		f=h-i;
		m=v*(1-s);
		n=v*(1-s*f);
		k=v*(1-s*(1-f));

		switch(i){
		case 0:
			r=v; g=k; b=m;
			break;
		case 1:
			r=n; g=v; b=m;
			break;
		case 2:
			r=m; g=v; b=k;
			break;
		case 3:
			r=m; g=n; b=v;
			break;
		case 4:
			r=k; g=m; b=v;
			break;
		case 5:
			r=v; g=m; b=n;
			break;
		default:
			ASSERT(0);
		}
	}

	ASSERT(r>=0 && r<=1);
	ASSERT(g>=0 && g<=1);
	ASSERT(b>=0 && b<=1);
}

void Color::setHSV(float h,float s,float v, unsigned char alpha)
{
	float rf,gf,bf;
	HSVtoRGB(h,s,v, rf,gf,bf);
	r = round(rf*255);
	g = round(gf*255);
	b = round(bf*255);
	a = alpha;
}


void Color::toHSV(float& h,float& s,float& v)
{
	float rf = r/255.f;
	float gf = g/255.f;
	float bf = b/255.f;
	v = max(max(rf,gf),bf);
	float temp=min(min(rf,gf),bf);
	if(v==0)
		s=0;
	else 
		s=(v-temp)/v;

	if(s==0)
		h=0;
	else {
		float Cr=(v-rf)/(v-temp);
		float Cg=(v-gf)/(v-temp);
		float Cb=(v-bf)/(v-temp);

		if(rf==v) {
			h=Cb-Cg;
		}
		else if(gf==v) {
			h=2+Cr-Cb;
		} 
		else if(bf==v) {
			h=4+Cg-Cr;
		}

		h=60*h;
		if(h<0)h+=360;
	}
}

void Color::serialize(yasli::Archive& ar) 
{
	ar(r, "", "&r");
	ar(g, "", "&g");
	ar(b, "", "&b");
	ar(a, "", "&a");
}

}