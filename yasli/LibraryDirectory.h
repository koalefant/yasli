#pragma once

#include "yasli/LibraryBase.h"
#include "yasli/Files.h"
#include "yasli/TextIArchive.h"
#include "yasli/TextOArchive.h"

template<class T, class ReferenceType>
class LibraryDirectory : public LibraryBase{
public:

	void serialize(Archive& ar){

	}

	// from LibraryBase:
    int addBase(const char* name){
		Element* e = new Element(name);
		e->set(new T(name));
		elements_.push_back(e);
		return int(elements_.size()) - 1;
	}
    size_t size() const{
		return elements_.size();
	}
    LibraryElementBase& elementByIndex(int index){
		ASSERT(index >= 0 && index < int(elements_.size()));
		Element& element = *elements_[index];
		if(!element.isLoaded())
			loadElement(element);
		ASSERT(element.get());
		return *element.get();
	}
    int findIndexByName(const char* name) const{ 
		Elements::const_iterator it;
		for(it = elements_.begin(); it != elements_.end(); ++it){
			const Element& element = **it;
			if(strcmp(element.name(), name) == 0)
				return it - elements_.begin();
		}
		return -1;
	}
    TypeID libraryTypeID() const{ return TypeID::get<LibraryDirectory>(); }
	void remove(const char* name){
		int index = findIndexByName(name);
		ASSERT(index >= 0);
		if(index < 0)
			return;

		const char* contentGroup = this->contentGroup();
		std::string filename = content->getPath(contentGroup, this->name());
		filename += Files::PATH_SEPARATOR;
		filename += name;

		if(Files::exists(filename.c_str()))
			Files::remove(filename.c_str());
		elements_.erase(elements_.begin() + index);
	}
	void fixNames(){
		Elements::iterator it;
		for(it = elements_.begin(); it != elements_.end(); ++it){
			Element& element = **it;
			if(element.isLoaded()){
				if(strcmp(element.get()->name(), element.name()) != 0)
					element.setName(element.get()->name());
			}
		}
		LibraryBase::fixNames();
	}
	// ^^^
	
	// from ContentUser:
	void preloadContent(){
		std::string directory = content->getPath(contentGroup(), name());
		std::cout << "Loading directory-library: " << directory << std::endl;

		std::string mask = directory;
		mask += Files::PATH_SEPARATOR;
		mask += "*";

		Files::iterator it(mask.c_str());
		Files::iterator end;
		for(; it != end; ++it){
			if(it->isFile()){
				const char* name = it->name();
				int index = findIndexByName(name);
				if(index < 0){
					elements_.push_back(new Element(name));
					index = elements_.size() - 1;
				}

				//loadElement(elements_[index]);
			}
		}
	}

	void loadContent()
	{
	}

	void saveContent()
	{
		Elements::iterator it;
		for(it = elements_.begin(); it != elements_.end(); ++it){
			Element& element = **it;
			if(element.isLoaded())
				saveElement(element);
		}
	}
	// ^^^
protected:
	class Element : public RefCounter{
	public:
		Element(const char* name = "")
		: name_(name)
		{
		}
		bool isLoaded() const{ return element_ != 0; }
		T* get() { return element_.get(); }
		void set(T* t){ element_.set(t); }
		void setName(const char* name){ name_ = name; }
		const char* name() const{ return name_.c_str(); }

		void setPreviousName(const char* prevName){ previousName_ = prevName; }
		const char* previousName() const{ return previousName_.c_str(); }
	protected:
		std::string name_;
		std::string previousName_;
		AutoPtr<T> element_;
	};

	typedef std::vector<SharedPtr<Element> > Elements;

	void loadElement(Element& element)
	{
		std::string filename = content->getPath(contentGroup(), name());
		filename += Files::PATH_SEPARATOR;
		filename += element.name();
		TextIArchive ia;
		ia.open(filename.c_str());

		if(!element.get())
			element.set(new T(element.name()));
		element.get()->serialize(ia);
		element.get()->setName(element.name());
		element.setPreviousName(element.name());
	}

	void saveElement(Element& element)
	{
		std::string filename = content->getPath(contentGroup(), name());
		filename += Files::PATH_SEPARATOR;
		if(!Files::isDirectory(filename.c_str())){
			if(Files::exists(filename.c_str()))
				return;
			Files::createDirectory(filename.c_str());
		}
		std::string fileToRemove;
		element.setName(element.get()->name());
		filename += element.name();

		std::cout << "Saving directory-library element: " << filename << std::endl;

		ASSERT(element.isLoaded());
		if(!element.isLoaded())
			return;
		TextOArchive oa;
		oa.open(filename.c_str());
		element.get()->setName(element.name());
		element.get()->serialize(oa);
		if(strcmp(element.previousName(), element.name()) != 0){
			std::string filename = content->getPath(contentGroup(), name());
			filename += Files::PATH_SEPARATOR;
			filename += element.previousName();
			Files::remove(filename.c_str());
		}
		element.setPreviousName(element.name());
	}

	Elements elements_;
};

