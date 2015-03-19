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
typedef RefCountedPtr<const malObject>    malObjectPtr;
typedef std::vector<malObjectPtr>         malObjectVec;
typedef malObjectVec::const_iterator      malObjectIter;

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

    bool isTrue() const;

    bool isEqualTo(malObjectPtr rhs) const;

    virtual malObjectPtr eval(malEnvPtr env) const;

    virtual String print(bool readably) const = 0;

protected:
    virtual bool doIsEqualTo(const malObject* rhs) const = 0;
};

template<class T>
T* object_cast(malObjectPtr obj, const char* typeName) {
    T* dest = dynamic_cast<T*>(obj.ptr());
    ASSERT(dest != NULL, "%s is not a %s", obj->print(true).c_str(), typeName);
    return dest;
}

#define OBJECT_CAST(Type, Object)   object_cast<const Type>(Object, #Type)
#define DYNAMIC_CAST(Type, Object)  (dynamic_cast<const Type*>((Object).ptr()))
#define STATIC_CAST(Type, Object)   (static_cast<const Type*>((Object).ptr()))

class malConstant : public malObject {
public:
    malConstant(String name) : m_name(name) { }
    virtual String print(bool readably) const { return m_name; }

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return this == rhs; // these are singletons
    }

private:
    String m_name;
};

class malInteger : public malObject {
public:
    malInteger(int value) : m_value(value) { }
    virtual ~malInteger() { }

    virtual String print(bool readably) const {
        return std::to_string(m_value);
    }

    int value() const { return m_value; }

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return m_value == static_cast<const malInteger*>(rhs)->m_value;
    }

private:
    int m_value;
};

class malString : public malObject {
public:
    malString(const String& token);
    virtual ~malString() { }

    virtual String print(bool readably) const;

    String value() const { return m_value; }
    String escapedValue() const;

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return m_value == static_cast<const malString*>(rhs)->m_value;
    }

private:
    String m_value;
};

class malKeyword : public malObject {
public:
    malKeyword(const String& token) : m_value(token) { }
    virtual ~malKeyword() { }

    virtual String print(bool readably) const {
        return m_value;
    }

    String value() const { return m_value; }

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return m_value == static_cast<const malKeyword*>(rhs)->m_value;
    }

private:
    String m_value;
};

class malSymbol : public malObject {
public:
    malSymbol(const String& token) : m_value(token) { }
    virtual ~malSymbol() { }

    virtual malObjectPtr eval(malEnvPtr env) const;

    virtual String print(bool readably) const {
        return m_value;
    }

    String value() const { return m_value; }

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return m_value == static_cast<const malSymbol*>(rhs)->m_value;
    }

private:
    String m_value;
};

class malSequence : public malObject {
public:
    malSequence(const malObjectVec& items) : m_items(items) { }
    virtual String print(bool readably) const;

    malObjectVec eval_items(malEnvPtr env) const;
    int count() const { return m_items.size(); }
    bool isEmpty() const { return m_items.empty(); }
    malObjectPtr item(int index) const { return m_items[index]; }

    malObjectIter begin() const { return m_items.cbegin(); }
    malObjectIter end()   const { return m_items.cend(); }

    virtual bool doIsEqualTo(const malObject* rhs) const;

private:
    malObjectVec m_items;
};

class malList : public malSequence {
public:
    malList(const malObjectVec& items) : malSequence(items) { }
    virtual ~malList() { }

    virtual String print(bool readably) const;
};

class malVector : public malSequence {
public:
    malVector(const malObjectVec& items) : malSequence(items) { }
    virtual ~malVector() { }

    virtual malObjectPtr eval(malEnvPtr env) const;
    virtual String print(bool readably) const;
};

class malApplicable : public malObject {
public:
    virtual ~malApplicable() { }

    virtual malObjectPtr apply(malObjectIter argsBegin,
                               malObjectIter argsEnd,
                               malEnvPtr env) const = 0;
};

class malHash : public malObject {
public:
    malHash(malObjectIter argsBegin, malObjectIter argsEnd);

    virtual String print(bool readably) const;

    virtual bool doIsEqualTo(const malObject* rhs) const;

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
                               malEnvPtr env) const;

    virtual String print(bool readably) const {
        return STRF("#builtin-function(%s)", m_name.c_str());
    }

    virtual bool doIsEqualTo(const malObject* rhs) const {
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
                               malEnvPtr env) const;

    malObjectPtr getBody() const { return m_body; }
    malEnvPtr makeEnv(malObjectIter argsBegin, malObjectIter argsEnd) const;

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return this == rhs; // do we need to do a deep inspection?
    }

    virtual String print(bool readably) const {
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
    malObjectPtr hash(malObjectIter argsBegin, malObjectIter argsEnd);
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
