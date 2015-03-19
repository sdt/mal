#ifndef INCLUDE_TYPES_H
#define INCLUDE_TYPES_H

#include "Debug.h"
#include "RefCounted.h"
#include "RefCountedPtr.h"
#include "String.h"
#include "Validation.h"

#include <exception>
#include <map>
#include <vector>

class malObject;
typedef RefCountedPtr<malObject>    malObjectPtr;
typedef std::vector<malObjectPtr>   malObjectVec;
typedef malObjectVec::iterator      malObjectIter;

#define ARRAY_SIZE(a)   (sizeof(a)/(sizeof(*(a))))

class malEnv;
typedef RefCountedPtr<malEnv> malEnvPtr;

class malEmptyInputException : public std::exception { };

class malObject : public RefCounted {
public:
    malObject() {
        TRACE_OBJECT("Creating malObject %p\n", this);
    }
    virtual ~malObject() {
        TRACE_OBJECT("Destroying malObject %p\n", this);
    }

    bool isEqualTo(malObjectPtr rhs);

    virtual malObjectPtr eval(malEnvPtr env) = 0;

    virtual String print(bool readably) = 0;

protected:
    virtual bool doIsEqualTo(malObject* rhs) = 0;
};

template<class T>
T* object_cast(malObjectPtr obj, const char* typeName) {
    T* dest = dynamic_cast<T*>(obj.ptr());
    ASSERT(dest != NULL, "%s is not a %s", obj->print(true).c_str(), typeName);
    return dest;
}

#define OBJECT_CAST(Type, Object)   object_cast<Type>(Object, #Type)
#define DYNAMIC_CAST(Type, Object)  (dynamic_cast<Type*>((Object).ptr()))

class malConstant : public malObject {
public:
    malConstant(String name) : m_name(name) { }
    virtual malObjectPtr eval(malEnvPtr env)  { return malObjectPtr(this); }
    virtual String print(bool readably) { return m_name; }

    virtual bool doIsEqualTo(malObject* rhs) {
        return this == rhs; // these are singletons
    }

private:
    String m_name;
};

class malInteger : public malObject {
public:
    malInteger(int value) : m_value(value) { }
    virtual ~malInteger() { }

    virtual malObjectPtr eval(malEnvPtr env);

    virtual String print(bool readably) {
        return std::to_string(m_value);
    }

    int value() { return m_value; }

    virtual bool doIsEqualTo(malObject* rhs) {
        return m_value == static_cast<malInteger*>(rhs)->m_value;
    }

private:
    int m_value;
};

class malString : public malObject {
public:
    malString(const String& token);
    virtual ~malString() { }

    virtual malObjectPtr eval(malEnvPtr env);

    virtual String print(bool readably);

    String value() { return m_value; }
    String escapedValue();

    virtual bool doIsEqualTo(malObject* rhs) {
        return m_value == static_cast<malString*>(rhs)->m_value;
    }

private:
    String m_value;
};

class malKeyword : public malObject {
public:
    malKeyword(const String& token) : m_value(token) { }
    virtual ~malKeyword() { }

    virtual malObjectPtr eval(malEnvPtr env);

    virtual String print(bool readably) {
        return m_value;
    }

    String value() { return m_value; }

    virtual bool doIsEqualTo(malObject* rhs) {
        return m_value == static_cast<malKeyword*>(rhs)->m_value;
    }

private:
    String m_value;
};

class malSymbol : public malObject {
public:
    malSymbol(const String& token) : m_value(token) { }
    virtual ~malSymbol() { }

    virtual malObjectPtr eval(malEnvPtr env);

    virtual String print(bool readably) {
        return m_value;
    }

    String value() { return m_value; }

    virtual bool doIsEqualTo(malObject* rhs) {
        return m_value == static_cast<malSymbol*>(rhs)->m_value;
    }

private:
    String m_value;
};

class malSequence : public malObject {
public:
    malSequence(const malObjectVec& items) : m_items(items) { }
    virtual String print(bool readably);

    malObjectVec eval_items(malEnvPtr env);
    int count() { return m_items.size(); }
    bool isEmpty() { return m_items.empty(); }
    malObjectPtr item(int index) { return m_items[index]; }

    malObjectIter begin() { return m_items.begin(); }
    malObjectIter end()   { return m_items.end(); }

    virtual bool doIsEqualTo(malObject* rhs);

private:
    malObjectVec m_items;
};

class malList : public malSequence {
public:
    malList(const malObjectVec& items) : malSequence(items) { }
    virtual ~malList() { }

    virtual String print(bool readably);

    virtual malObjectPtr eval(malEnvPtr env);
};

class malVector : public malSequence {
public:
    malVector(const malObjectVec& items) : malSequence(items) { }
    virtual ~malVector() { }

    virtual String print(bool readably);

    virtual malObjectPtr eval(malEnvPtr env);
};

class malApplicable : public malObject {
public:
    virtual ~malApplicable() { }

    virtual malObjectPtr apply(malObjectIter argsBegin,
                               malObjectIter argsEnd,
                               malEnvPtr env) = 0;
};

class malHash : public malObject {
public:
    malHash(const malObjectVec& items);

    virtual String print(bool readably);

    virtual malObjectPtr eval(malEnvPtr env);

    virtual bool doIsEqualTo(malObject* rhs);

private:

    typedef std::map<String, malObjectPtr> Map;
    Map m_map;
};

class malBuiltIn : public malApplicable {
public:
    typedef malObjectPtr (ApplyFunc)(const String& name,
                                     malObjectIter argsBegin,
                                     malObjectIter argsEnd,
                                     malEnvPtr env);

    malBuiltIn(const String& name, ApplyFunc* handler)
    : m_name(name), m_handler(handler) { }

    virtual malObjectPtr apply(malObjectIter argsBegin,
                               malObjectIter argsEnd,
                               malEnvPtr env);

    virtual malObjectPtr eval(malEnvPtr env);

    virtual String print(bool readably) {
        return STRF("#builtin-function(%s)", m_name.c_str());
    }

    virtual bool doIsEqualTo(malObject* rhs) {
        return this == rhs; // these are singletons
    }

private:
    String     m_name;
    ApplyFunc* m_handler;
};

class malLambda : public malApplicable {
public:
    malLambda(const StringVec& bindings, malObjectPtr body, malEnvPtr env);

    virtual malObjectPtr apply(malObjectIter argsBegin,
                               malObjectIter argsEnd,
                               malEnvPtr env);

    virtual malObjectPtr eval(malEnvPtr env);

    malObjectPtr getBody() { return m_body; }
    malEnvPtr makeEnv(malObjectIter argsBegin, malObjectIter argsEnd);

    virtual bool doIsEqualTo(malObject* rhs) {
        return this == rhs; // do we need to do a deep inspection?
    }

    virtual String print(bool readably) {
        return STRF("#user-function(%p)", this);
    }

private:
    StringVec    m_bindings;
    malObjectPtr m_body;
    malEnvPtr    m_env;
};

namespace mal {
    malObjectPtr boolean(bool value);
    malObjectPtr builtin(const String& name, malBuiltIn::ApplyFunc handler);
    malObjectPtr falseObject();
    malObjectPtr hash(const malObjectVec& items);
    malObjectPtr integer(int value);
    malObjectPtr integer(const String& token);
    malObjectPtr keyword(const String& token);
    malObjectPtr lambda(const StringVec&, malObjectPtr, malEnvPtr);
    malObjectPtr list(const malObjectVec& items);
    malObjectPtr nil();
    malObjectPtr string(const String& token);
    malObjectPtr symbol(const String& token);
    malObjectPtr trueObject();
    malObjectPtr vector(const malObjectVec& items);
};

#endif // INCLUDE_TYPES_H
