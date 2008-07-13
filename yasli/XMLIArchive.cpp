#include "StdAfx.h"
#include "XMLIArchive.h"
#include "pugixml.hpp"

struct XMLIArchiveImpl{
    pugi::xml_document document;
    pugi::xml_node node;
    pugi::xml_node childNode;
    pugi::xml_node previousChildNode;
    pugi::xml_attribute childAttribute;
};

using pugi::xml_node;
using pugi::xml_attribute;

XMLIArchive::XMLIArchive()
: Archive( true, false )
{

}

XMLIArchive::~XMLIArchive()
{
    if(isOpen())
        close();
}

void XMLIArchive::open(const char* filename)
{
    if(isOpen())
        close();
    impl_ = new XMLIArchiveImpl();
    impl_->document.load_file(filename);
    impl_->childNode = impl_->document.root();
    enterNode();
}

void XMLIArchive::openFromMemory(const char* buffer, size_t length, bool free)
{
    ASSERT(0 && "Not implemented");
}

bool XMLIArchive::isOpen() const
{
    return impl_ != 0;
}

void XMLIArchive::close()
{
    impl_ = 0;
}

bool XMLIArchive::enterNode()
{
    if(impl_->childNode){
        impl_->node = impl_->childNode;
        impl_->previousChildNode = xml_node();
        impl_->childNode = impl_->node.last_child();
        return true;
    }
    return false;
}

void XMLIArchive::leaveNode()
{
    impl_->childNode = impl_->node;
    impl_->previousChildNode = impl_->childNode;
    impl_->node = impl_->node.parent();
}

bool XMLIArchive::findNode(const char* name)
{
  if(name[0] == '\0')
  {
    name = "el";
    xml_node current = impl_->childNode.next_sibling();
    if(!current){
      if(impl_->previousChildNode)
        return false;
      current = impl_->node.first_child();
    }

    while(true){
        if(strcmp(current.name(), name) == 0){
            impl_->previousChildNode = impl_->childNode;
            impl_->childNode = current;
            return true;
        }
        current = current.next_sibling();
        if(!current)
            return false;
    }
    return false;
  }
  else
  {

    xml_node first = impl_->childNode;
    xml_node current = first;
    current = current.next_sibling();
    if(!current)
      current = impl_->node.first_child();

    while(true){
        if(strcmp(current.name(), name) == 0){
            impl_->previousChildNode = impl_->childNode;
            impl_->childNode = current;
            return true;
        }
        if(current == first)
          return false;

        current = current.next_sibling();
        if(!current)
            current = impl_->node.first_child();
    }
    return false;
  }
}

bool XMLIArchive::findAttribute(const char* name)
{
    if(name[0] == '\0')
        name = "el";
    xml_attribute first = impl_->node.first_attribute();
    xml_attribute current = first;
    current = current.next_attribute();
    if(!current)
        current = impl_->node.first_attribute();
    while(true){
        if(strcmp(current.name(), name) == 0){
            impl_->childAttribute = current;
            return true;
        }
        if(current == first)
          return false;
        current = current.next_attribute();
        if(!current)
            current = impl_->node.first_attribute();
    }
    return false;
}

bool XMLIArchive::operator()(bool& value, const char* name)
{
    return false;
}

bool XMLIArchive::operator()(std::string& value, const char* name)
{
    if(findNode(name)){
        value = impl_->childNode.child_value();
        return true;
    }
    else if(findAttribute(name)){
        value = impl_->childAttribute.value();
        return true;
    }
    return false;
}

bool XMLIArchive::operator()(float& value, const char* name)
{
    if(findNode(name)){
        value = float(atof(impl_->childNode.child_value()));
        return true;
    }
    else if(findAttribute(name)){
        value = float(atof(impl_->childAttribute.value()));
        return true;
    }
    return false;
}

bool XMLIArchive::operator()(double& value, const char* name)
{
    if(findNode(name)){
        value = atof(impl_->childNode.child_value());
        return true;
    }
    else if(findAttribute(name)){
        value = atof(impl_->childAttribute.value());
        return true;
    }
    return false;
}

bool XMLIArchive::operator()(int& value, const char* name)
{
    if(findNode(name)){
        value = atoi(impl_->childNode.child_value());
        return true;
    }
    else if(findAttribute(name)){
        value = atoi(impl_->childAttribute.value());
        return true;
    }
    return false;
}

bool XMLIArchive::operator()(unsigned int& value, const char* name)
{
    if(findNode(name)){
        value = strtoul(impl_->childNode.child_value(), 0, 10);
        return true;
    }
    else if(findAttribute(name)){
        value = strtoul(impl_->childAttribute.value(), 0, 10);
        return true;
    }
    return false;
}

bool XMLIArchive::operator()(__int64& value, const char* name)
{
    ASSERT(0 && "Not implemented");
    return false;
}


bool XMLIArchive::operator()(signed char& value, const char* name)
{
    if(findNode(name)){
        value = (signed char)strtol(impl_->childNode.child_value(), 0, 10);
        return true;
    }
    else if(findAttribute(name)){
        value = (signed char)strtoul(impl_->childAttribute.value(), 0, 10);
        return true;
    }
    return false;
}

bool XMLIArchive::operator()(unsigned char& value, const char* name)
{
    if(findNode(name)){
        value = (unsigned char)strtol(impl_->childNode.child_value(), 0, 10);
        return true;
    }
    else if(findAttribute(name)){
        value = (unsigned char)strtoul(impl_->childAttribute.value(), 0, 10);
        return true;
    }
    return false;
}

bool XMLIArchive::operator()(char& value, const char* name)
{
    if(findNode(name)){
        value = (char)strtol(impl_->childNode.child_value(), 0, 10);
        return true;
    }
    else if(findAttribute(name)){
        value = (char)strtoul(impl_->childAttribute.value(), 0, 10);
        return true;
    }
    return false;
}


bool XMLIArchive::operator()(const Serializer& ser, const char* name)
{
    if(findNode(name)){
        if(enterNode()){

            ser(*this);

            leaveNode();
            return true;
        }
    }
    return false;
}

bool XMLIArchive::operator()(const ContainerSerializationInterface& ser, const char* name)
{
    if(findNode(name)){
        if(enterNode()){

            std::size_t size = ser.size();
            std::size_t index = 0;

            while(true){
                if(index == size)
                    size = ser.resize(index + 1);
                if(!ser(*this, index))
                    break;
                ++index;
            }
            if(size > index)
                ser.resize(index);

            leaveNode();
            return true;
        }
    }
    return false;
}

const char* XMLIArchive::pull()
{
  xml_node next = impl_->childNode.next_sibling();
  if(!next){
    if(impl_->previousChildNode)
      return 0;
    else
      next = impl_->node.first_child();
    if(!next)
      return 0;
  }
  impl_->previousChildNode = impl_->childNode;
  impl_->childNode = next;
  return next.name();
}

