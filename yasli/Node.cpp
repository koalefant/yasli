#include "StdAfx.h"

#include "serialization/Node.h"

void NodeBase::free()
{
    freeChildren();
    //::free(reinterpret_cast<void*>(this));
    delete this;
}

void NodeBase::freeChildren()
{
    if(NodeBase* current = children_){
        NodeBase* next = 0;
        do{
            next = current->next_;
            current->free();
            current = next;
        }while(next);
        children_ = 0;
    }
}

void NodeBase::setChildren(NodeBase* node)
{
    ASSERT(children_ == 0);
    children_ = node;
    children_->parent_ = this;
}

void NodeBase::setNext(NodeBase* node)
{
    ASSERT(next_ == 0);
    node->prev_ = this;
    next_ = node;
    node->parent_ = parent_;
}

void NodeBase::replace(NodeBase* node)
{
    if(prev_)
        prev_->next_ = node;
    if(next_)
        next_->prev_ = node;
    if(parent_ && parent_->children_ == this)
        parent_->children_ = node;

    NodeBase* current = children_;
    while(current){
        current->parent_ = node;
        current = current->next_;
    }

}


std::size_t NodeNameTable::sizeOf() const
{
    int count = *(int*)(data());
    ASSERT(count >= 0);
    int dataSize = *(int*)((char*)data() + sizeof(count) + sizeof(char*) * count);
    ASSERT(dataSize >= 0);
    return sizeof(NodeBase) + sizeof(count) + count * sizeof(char*) + sizeof(dataSize) + dataSize;
}

int NodeNameTable::findName(const char* name) const
{
    ASSERT(type() == NODE_TYPE_NAMES_TABLE);
    const char** data = (const char**)this->data();
    int count = (int)(*data);
    ++data;
    for(int i = 0; i < count; ++i){
        if(strcmp(data[i], name) == 0)
            return i;
    }
    return -1;
}

NodeNameTable* NodeNameTable::addName(int& outIndex, const char* name, bool pack)
{
    //                count namePtr0   namePtrN dataCount  name0                   name N
    // [ NodeBase ] [int] [char*] ... [char*] [int] [char[strlen] + '\0'] ... [char[strlen] + '\0']
    ASSERT(type() == NODE_TYPE_NAMES_TABLE);
    ASSERT(findName(name) == -1);

    char* data = this->data();
    int count = *(int*)(data);
    ASSERT(count >= 0);

    int dataSize = *(int*)(data + sizeof(count) + count * sizeof(char*));

    int totalSize = sizeof(count) + sizeof(char*) * (count + 1) + sizeof(dataSize) + dataSize;

    if(pack)
        totalSize += strlen(name) + 1;

    NodeNameTable* newTable = (NodeNameTable*)malloc(sizeof(NodeBase) + totalSize);
    memcpy(newTable, this, sizeof(NodeBase));
    char* newData = (char*)newTable->data();
    *(int*)(newData) = count + 1;


    char** destOffset = (char**)(newData + sizeof(count) + sizeof(char*) * (count + 1) + sizeof(dataSize));
    char** srcOffset = (char**)(data + sizeof(count) + sizeof(char*) * count + sizeof(dataSize));
    for(int i = 0; i < count; ++i){
        char** dest = (char**)(newData + sizeof(count) + i * sizeof(char*));
        char** src = (char**)(data + sizeof(count) + i * sizeof(char*));
        *dest = *src - (unsigned long)srcOffset + (unsigned long)destOffset;
    }

    char** newNamePtr = (char**)(newData + sizeof(count) + sizeof(char*) * (count));

    memcpy(newData + sizeof(count) + sizeof(char*) * (count+1) + sizeof(dataSize),
           data + sizeof(count) + sizeof(char*) * (count) + sizeof(dataSize), dataSize);
    //memcpy(newNamePtr + 2, data + 1 + count + 1, dataSize);
    if(pack){
        char* newNameText = newData + sizeof(count) + sizeof(char*) * (count + 1) + sizeof(dataSize) + dataSize;
        int textSize = strlen(name) + 1;
        memcpy(newNameText, name, textSize);
        *newNamePtr = newNameText;
        *(int*)(newData + sizeof(count) + sizeof(char*) * (count + 1)) = dataSize + textSize;
    }
    else{
        *newNamePtr = const_cast<char*>(name);
        *(int*)(newData + sizeof(count) + sizeof(char*) * (count + 1)) = dataSize;
    }
    outIndex = count;
    ASSERT(newTable->findName(name) == count);
    return newTable;
}

const char* NodeNameTable::nameByIndex(int index) const
{
    ASSERT(type() == NODE_TYPE_NAMES_TABLE);

    char** data = (char**)this->data();
    int count = (int)(*data);
    ++data;
    ASSERT(index >= 0 && index < count);

    return data[index];
}

NodeNameTable* NodeNameTable::create()
{
    NodeNameTable* result = (NodeNameTable*)malloc(sizeof(NodeBase) + sizeof(int) * 2);
    memset(result, 0, sizeof(NodeBase) + sizeof(int) * 2);
    result->setType(NODE_TYPE_NAMES_TABLE);
    return result;
}

/*
     void NodeBase::serialize(Archive& ar)
     {
     ar(prev_, "prev");
     ar(next_, "next");
     ar(parent_, "parent");
     ar(children_, "children");
     ar(type_, "type");
     ar(name_, "name");
     }
     */
