#ifndef INCLUDE_TYPES_H
#define INCLUDE_TYPES_H

#include "Debug.h"
#include "RefCounted.h"
#include "RefCountedPtr.h"
#include "String.h"

#include <vector>

class malObject;
typedef RefCountedPtr<malObject>    malObjectPtr;
typedef std::vector<malObjectPtr>   malObjectVec;

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
    if (dest == NULL) {
        throw STR("%s is not a %s", obj->print().c_str(), typeName);
    }
    return dest;
}

#define OBJECT_CAST(Type, Object)   object_cast<Type>(Object, #Type)

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

private:
    String m_value;
};

class malSequence : public malObject {
public:
    malSequence(const malObjectVec& items) : m_items(items) { }
    virtual String print();

    malObjectVec eval_items(malEnvPtr env);

private:
    malObjectVec m_items;
};

class malList : public malSequence {
public:
    malList(const malObjectVec& items) : malSequence(items) { }
    virtual ~malList() { }

    virtual malObjectPtr eval(malEnvPtr env);
};

class malApplicable : public malObject {
public:
    virtual ~malApplicable() { }

    virtual malObjectPtr apply(const malObjectVec& args, malEnvPtr env) = 0;
};

class malBuiltIn : public malApplicable {
public:
    typedef malObjectPtr (ApplyFunc)(const malObjectVec& args, malEnvPtr env);

    malBuiltIn(const String& name, ApplyFunc* handler)
    : m_name(name), m_handler(handler) { }

    malObjectPtr apply(const malObjectVec& args, malEnvPtr env) {
        return m_handler(args, env);
    }

    virtual malObjectPtr eval(malEnvPtr env);

    virtual String print() {
        return STR("#builtin-function(%s)", m_name.c_str());
    }

private:
    String m_name;
    ApplyFunc* m_handler;
};

namespace mal {
    inline malObjectPtr builtin(const String& name, malBuiltIn::ApplyFunc handler) {
        return malObjectPtr(new malBuiltIn(name, handler));
    };

    inline malObjectPtr integer(int value) {
        return malObjectPtr(new malInteger(value));
    };

    inline malObjectPtr integer(const String& token) {
        return integer(std::stoi(token));
    };

    inline malObjectPtr list(const malObjectVec& items) {
        return malObjectPtr(new malList(items));
    };

    inline malObjectPtr symbol(const String& token) {
        return malObjectPtr(new malSymbol(token));
    };
};

#endif // INCLUDE_TYPES_H
