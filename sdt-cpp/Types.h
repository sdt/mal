#ifndef INCLUDE_TYPES_H
#define INCLUDE_TYPES_H

#include "Debug.h"
#include "RefCounted.h"
#include "RefCountedPtr.h"
#include "String.h"
#include "Validation.h"

#include <vector>

class malObject;
typedef RefCountedPtr<malObject>    malObjectPtr;
typedef std::vector<malObjectPtr>   malObjectVec;
typedef malObjectVec::iterator      malObjectIter;

class malEnv;
typedef RefCountedPtr<malEnv> malEnvPtr;

class malObject : public RefCounted {
public:
    malObject() {
        TRACE_OBJECT("Creating malObject %p\n", this);
    }
    virtual ~malObject() {
        TRACE_OBJECT("Destroying malObject %p\n", this);
    }

    virtual malObjectPtr eval(malEnvPtr env) = 0;

    virtual String print() = 0;
};

template<class T>
T* object_cast(malObjectPtr obj, const char* typeName) {
    T* dest = dynamic_cast<T*>(obj.ptr());
    ASSERT(dest != NULL, "%s is not a %s", obj->print().c_str(), typeName);
    return dest;
}

#define OBJECT_CAST(Type, Object)   object_cast<Type>(Object, #Type)
#define DYNAMIC_CAST(Type, Object)  (dynamic_cast<Type*>((Object).ptr()))

class malInteger : public malObject {
public:
    malInteger(int value) : m_value(value) { }
    virtual ~malInteger() { }

    virtual malObjectPtr eval(malEnvPtr env);

    virtual String print() {
        return std::to_string(m_value);
    }

    int value() { return m_value; }

private:
    int m_value;
};

class malSymbol : public malObject {
public:
    malSymbol(const String& token) : m_value(token) { }
    virtual ~malSymbol() { }

    virtual malObjectPtr eval(malEnvPtr env);

    virtual String print() {
        return m_value;
    }

    String value() { return m_value; }

private:
    String m_value;
};

class malSequence : public malObject {
public:
    malSequence(const malObjectVec& items) : m_items(items) { }
    virtual String print();

    malObjectVec eval_items(malEnvPtr env);
    int count() { return m_items.size(); }
    malObjectPtr item(int index) { return m_items[index]; }

protected:
    malObjectVec items() { return m_items; }

private:
    malObjectVec m_items;
};

class malList : public malSequence {
public:
    malList(const malObjectVec& items) : malSequence(items) { }
    virtual ~malList() { }

    virtual String print();

    virtual malObjectPtr eval(malEnvPtr env);
};

class malVector : public malSequence {
public:
    malVector(const malObjectVec& items) : malSequence(items) { }
    virtual ~malVector() { }

    virtual String print();

    virtual malObjectPtr eval(malEnvPtr env);
};

class malApplicable : public malObject {
public:
    virtual ~malApplicable() { }

    virtual malObjectPtr apply(malObjectIter argsBegin,
                               malObjectIter argsEnd,
                               malEnvPtr env) = 0;
};

class malBuiltIn : public malApplicable {
public:
    typedef malObjectPtr (ApplyFunc)(malObjectIter argsBegin,
                                     malObjectIter argsEnd,
                                     malEnvPtr env);

    malBuiltIn(const String& name, ApplyFunc* handler)
    : m_name(name), m_handler(handler) { }

    virtual malObjectPtr apply(malObjectIter argsBegin,
                               malObjectIter argsEnd,
                               malEnvPtr env);

    virtual malObjectPtr eval(malEnvPtr env);

    virtual String print() {
        return STR("#builtin-function(%s)", m_name.c_str());
    }

private:
    String m_name;
    ApplyFunc* m_handler;
};

namespace mal {
    malObjectPtr builtin(const String& name, malBuiltIn::ApplyFunc handler);
    malObjectPtr integer(int value);
    malObjectPtr integer(const String& token);
    malObjectPtr list(const malObjectVec& items);
    malObjectPtr symbol(const String& token);
    malObjectPtr vector(const malObjectVec& items);
};

#endif // INCLUDE_TYPES_H
