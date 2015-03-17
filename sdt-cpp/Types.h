#ifndef INCLUDE_TYPES_H
#define INCLUDE_TYPES_H

#include "Debug.h"
#include "RefCounted.h"
#include "RefCountedPtr.h"
#include "String.h"

#include <vector>

class malObject;
typedef RefCountedPtr<malObject> malObjectPtr;
typedef std::vector<malObjectPtr>  malObjectVec;

class Environment;

class malObject : public RefCounted {
public:
    malObject() {
        TRACE_OBJECT("Creating malObject %p\n", this);
    }
    virtual ~malObject() {
        TRACE_OBJECT("Destroying malObject %p\n", this);
    }

    virtual malObjectPtr eval(Environment* env) = 0;

    virtual String print() = 0;
    virtual bool isSequence() const { return false; }
};

class malInteger : public malObject {
public:
    malInteger(const String& token) : m_value(std::stoi(token)) { }
    ~malInteger() { }

    virtual malObjectPtr eval(Environment* env);

    virtual String print() {
        return std::to_string(m_value);
    }

private:
    int m_value;
};

class malSymbol : public malObject {
public:
    malSymbol(const String& token) : m_value(token) { }
    ~malSymbol() { }

    virtual malObjectPtr eval(Environment* env);

    virtual String print() {
        return m_value;
    }

private:
    String m_value;
};

class malSequence : public malObject {
public:
    malSequence(const malObjectVec& items) : m_items(items) { }
    virtual bool isSequence() const { return true; }
    virtual String print();

    virtual malObjectPtr eval(Environment* env) {
        return malObjectPtr(this);
    }

private:
    malObjectVec m_items;
};

class malList : public malSequence {
public:
    malList(const malObjectVec& items) : malSequence(items) { }
    ~malList() { }
};

namespace mal {
    inline malObjectPtr integer(const String& token) {
        return malObjectPtr(new malInteger(token));
    };

    inline malObjectPtr list(const malObjectVec& items) {
        return malObjectPtr(new malList(items));
    };

    inline malObjectPtr symbol(const String& token) {
        return malObjectPtr(new malSymbol(token));
    };
};

#endif // INCLUDE_TYPES_H
