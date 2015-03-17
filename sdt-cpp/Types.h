#ifndef INCLUDE_TYPES_H
#define INCLUDE_TYPES_H

#include "String.h"

#include <vector>

class malObject {
public:
    virtual ~malObject() { };

    virtual String print() = 0;
    virtual bool isSequence() const { return false; }
};

typedef std::shared_ptr<malObject> malObjectPtr;
typedef std::vector<malObjectPtr>  malObjectVec;

class malInteger : public malObject {
public:
    malInteger(const String& rep) : m_value(std::stoi(rep)) { }
    ~malInteger() { }

    virtual String print() {
        return std::to_string(m_value);
    }

private:
    int m_value;
};

class malSequence : public malObject {
public:
    malSequence(const malObjectVec& items) : m_items(items) { }
    virtual bool isSequence() const { return true; }
    virtual String print();

private:
    malObjectVec m_items;
};

class List : public malSequence {
public:
    List(const malObjectVec& items) : malSequence(items) { }
    ~List() { }
};

#endif // INCLUDE_TYPES_H
