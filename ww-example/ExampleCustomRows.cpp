#include <string>
#include <vector>
using std::vector;

#include "ww/PropertyTree.h"
#include "ww/Serialization.h"
#include "ww/Color.h"
#include "ww/Icon.h"
#include "yasli/BitVector.h" 
#include "yasli/decorators/BitFlags.h"

using yasli::Archive;
using yasli::SharedPtr;

#include "Icons/favourites.xpm"
#include "Icons/favouritesDisabled.xpm"
static ww::Icon iconFavourite(favourites_xpm);
static ww::Icon iconFavouriteDisabled(favouritesDisabled_xpm);

enum Flags
{
	FLAG_FIRST = 1 << 0,
	FLAG_SECOND = 1 << 1,
	FLAG_THIRD = 1 << 2,
	FLAG_ALL = FLAG_FIRST | FLAG_SECOND | FLAG_THIRD
};

YASLI_ENUM_BEGIN(Flags, "Flags")
YASLI_ENUM(FLAG_FIRST, "first", "First")
YASLI_ENUM(FLAG_SECOND, "second", "Second")
YASLI_ENUM(FLAG_THIRD, "third", "Third")
YASLI_ENUM(FLAG_ALL, "all", "All")
YASLI_ENUM_END()

struct CustomRows
{
	yasli::BitVector<Flags> bitVector;
	int bitFlags;

	void serialize(Archive& ar)
	{
		// old way of serializing bit flags
		ar(bitVector, "bitVector", "Bit Vector");
		// new way, less intrusive
		ar(yasli::BitFlags<Flags>(bitFlags), "bitFlags", "Bit Flags");

	}

	CustomRows()
	: bitVector(0)
	, bitFlags(0)
	{
	}
} customRows;

ww::Widget* createCustomRows()
{
	ww::PropertyTree* propertyTree = new ww::PropertyTree();

	propertyTree->attach(yasli::Serializer(customRows));
	propertyTree->expandAll();

	return propertyTree;
}

