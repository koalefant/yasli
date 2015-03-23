:source-highlighter: pygments
## yasli - easy C++ serialization

Highlights:

- Write serialization once, access multiple formats:
	* JSON
	* Expose in <<QPropertyTree>>
- Works with complex types and STL containers
- Support polymorphic types and enum-annotations
- Non-intrusive when needed
- Extendable in user code
- Crossplatform

	:::cpp
	enum Position
	{
	   ENGINEER,
	   MANAGER,
	};
	
	struct Entry
	{
	    std::string name = "Foo Bar";
	    int age = 25;
	    Position position = ENGINEER;
	
	    void serialize(yasli::Archive& ar)
	    {
	        ar(name, "name");
	        ar(age, "age");
	        ar(position, "position");
	    }
	};
	
	struct Document
	{
	    std::vector<Entry> entries = { Entry() };
	
	    void serialize(yasli::Archive& ar)
	    {
	        ar(entries, "entries");
	    }
	};
	
	YASLI_ENUM_BEGIN(Position, "Position")
	YASLI_ENUM(ENGINEER, "engineer", "Engineer")
	YASLI_ENUM(MANAGER, "manager", "Manager")
	YASLI_ENUM_END()

Document instance, serialized through JSON-archive will look like this:

	:::json
	{ "entries": [ { "name": "Foo Bar", "age": 25, "position": "engineer" } ] }

## QPropertyTree - property widget for Qt

Features:

- Uses serialization to query data
- Container operations: addition, removal and reordering with Drag&Drop
- Enumerations appear as combo-boxes
- Access to polymorphic types
- Extendable
