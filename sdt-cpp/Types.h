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

class malHash;
typedef RefCountedPtr<const malHash>      malHashPtr;


#define ARRAY_SIZE(a)   (sizeof(a)/(sizeof(*(a))))

class malEnv;
typedef RefCountedPtr<malEnv> malEnvPtr;

class malEmptyInputException : public std::exception { };

class malObject : public RefCounted {
public:
    malObject() {
        TRACE_OBJECT("Creating malObject %p\n", this);
    }
    malObject(malHashPtr meta) : m_meta(meta) {
        TRACE_OBJECT("Creating malObject %p\n", this);
    }
    virtual ~malObject() {
        TRACE_OBJECT("Destroying malObject %p\n", this);
    }

    malObjectPtr withMeta(malHashPtr meta);
    //{ return malObjectPtr(doWithMeta(meta)); }
    virtual malObjectPtr doWithMeta(malHashPtr meta) = 0;

    bool isTrue() const;

    bool isEqualTo(malObjectPtr rhs) const;

    virtual malObjectPtr eval(malEnvPtr env) const;

    virtual String print(bool readably) const = 0;

protected:
    virtual bool doIsEqualTo(const malObject* rhs) const = 0;

    malHashPtr m_meta;
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

#define WITH_META(Type) \
    virtual malObjectPtr doWithMeta(malHashPtr meta) { \
        return new Type(*this, meta); \
    } \

class malConstant : public malObject {
public:
    malConstant(String name) : m_name(name) { }
    malConstant(const malConstant& that, malHashPtr meta)
        : malObject(meta), m_name(that.m_name) { }

    virtual String print(bool readably) const { return m_name; }

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return this == rhs; // these are singletons
    }

    WITH_META(malConstant);

private:
    const String m_name;
};

class malInteger : public malObject {
public:
    malInteger(int value) : m_value(value) { }
    malInteger(const malInteger& that, malHashPtr meta)
        : malObject(meta), m_value(that.m_value) { }

    virtual String print(bool readably) const {
        return std::to_string(m_value);
    }

    int value() const { return m_value; }

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return m_value == static_cast<const malInteger*>(rhs)->m_value;
    }

    WITH_META(malInteger);

private:
    const int m_value;
};

class malString : public malObject {
public:
    malString(const String& token);
    malString(const malString& that, malHashPtr meta)
        : malObject(meta), m_value(that.m_value) { }

    virtual String print(bool readably) const;

    String value() const { return m_value; }
    String escapedValue() const;

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return m_value == static_cast<const malString*>(rhs)->m_value;
    }

    WITH_META(malString);

private:
    const String m_value;
};

class malKeyword : public malObject {
public:
    malKeyword(const String& token) : m_value(token) { }
    malKeyword(const malKeyword& that, malHashPtr meta)
        : malObject(meta), m_value(that.m_value) { }

    virtual String print(bool readably) const {
        return m_value;
    }

    String value() const { return m_value; }

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return m_value == static_cast<const malKeyword*>(rhs)->m_value;
    }

    WITH_META(malKeyword);

private:
    const String m_value;
};

class malSymbol : public malObject {
public:
    malSymbol(const String& token) : m_value(token) { }
    malSymbol(const malSymbol& that, malHashPtr meta)
        : malObject(meta), m_value(that.m_value) { }

    virtual malObjectPtr eval(malEnvPtr env) const;

    virtual String print(bool readably) const {
        return m_value;
    }

    String value() const { return m_value; }

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return m_value == static_cast<const malSymbol*>(rhs)->m_value;
    }

    WITH_META(malSymbol);

private:
    const String m_value;
};

class malSequence : public malObject {
public:
    malSequence(const malObjectVec& items) : m_items(items) { }
    malSequence(malObjectIter begin, malObjectIter end)
        : m_items(begin, end) { }
    malSequence(const malSequence& that, malHashPtr meta)
        : malObject(meta), m_items(that.m_items) { }

    virtual String print(bool readably) const;

    malObjectVec eval_items(malEnvPtr env) const;
    int count() const { return m_items.size(); }
    bool isEmpty() const { return m_items.empty(); }
    malObjectPtr item(int index) const { return m_items[index]; }

    malObjectIter begin() const { return m_items.cbegin(); }
    malObjectIter end()   const { return m_items.cend(); }

    virtual bool doIsEqualTo(const malObject* rhs) const;

    malObjectPtr first() const;
    virtual malObjectPtr rest() const;

private:
    const malObjectVec m_items;
};

class malList : public malSequence {
public:
    malList(const malObjectVec& items) : malSequence(items) { }
    malList(malObjectIter begin, malObjectIter end)
        : malSequence(begin, end) { }
    malList(const malList& that, malHashPtr meta)
        : malSequence(that, meta) { }

    virtual String print(bool readably) const;

    WITH_META(malList);
};

class malVector : public malSequence {
public:
    malVector(const malObjectVec& items) : malSequence(items) { }
    malVector(malObjectIter begin, malObjectIter end)
        : malSequence(begin, end) { }
    malVector(const malVector& that, malHashPtr meta)
        : malSequence(that, meta) { }

    virtual malObjectPtr eval(malEnvPtr env) const;
    virtual String print(bool readably) const;

    WITH_META(malVector);
};

class malApplicable : public malObject {
public:
    malApplicable() { }
    malApplicable(malHashPtr meta) : malObject(meta) { }

    virtual malObjectPtr apply(malObjectIter argsBegin,
                               malObjectIter argsEnd,
                               malEnvPtr env) const = 0;
};

class malHash : public malObject {
public:
    typedef std::map<String, malObjectPtr> Map;

    malHash(malObjectIter argsBegin, malObjectIter argsEnd);
    malHash(const malHash::Map& map);
    malHash(const malHash& that, malHashPtr meta)
        : malObject(meta), m_map(that.m_map) { }

    malObjectPtr assoc(malObjectIter argsBegin, malObjectIter argsEnd) const;
    malObjectPtr dissoc(malObjectIter argsBegin, malObjectIter argsEnd) const;
    bool contains(malObjectPtr key) const;
    malObjectPtr get(malObjectPtr key) const;
    malObjectPtr keys() const;
    malObjectPtr values() const;

    virtual String print(bool readably) const;

    virtual bool doIsEqualTo(const malObject* rhs) const;

    WITH_META(malHash);

private:
    const Map m_map;
};

class malBuiltIn : public malApplicable {
public:
    typedef malObjectPtr (ApplyFunc)(const String& name,
                                     malObjectIter argsBegin,
                                     malObjectIter argsEnd,
                                     malEnvPtr env);

    malBuiltIn(const String& name, ApplyFunc* handler)
    : m_name(name), m_handler(handler) { }

    malBuiltIn(const malBuiltIn& that, malHashPtr meta)
    : malApplicable(meta), m_name(that.m_name), m_handler(that.m_handler) { }

    virtual malObjectPtr apply(malObjectIter argsBegin,
                               malObjectIter argsEnd,
                               malEnvPtr env) const;

    virtual String print(bool readably) const {
        return STRF("#builtin-function(%s)", m_name.c_str());
    }

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return this == rhs; // these are singletons
    }

    WITH_META(malBuiltIn);

private:
    const String     m_name;
    ApplyFunc* m_handler;
};

class malLambda : public malApplicable {
public:
    malLambda(const StringVec& bindings, malObjectPtr body, malEnvPtr env);
    malLambda(const malLambda& that, malHashPtr meta);

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

    virtual malObjectPtr doWithMeta(malHashPtr meta);

private:
    const StringVec    m_bindings;
    const malObjectPtr m_body;
    const malEnvPtr    m_env;
};

class malMacro : public malObject {
public:
    malMacro(const malLambda* lambda);
    malMacro(const malMacro& that, malHashPtr meta);

    malObjectPtr apply(malObjectIter argsBegin, malObjectIter argsEnd,
                       malEnvPtr env) const;

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return this == rhs; // do we need to do a deep inspection?
    }

    virtual String print(bool readably) const {
        return STRF("#user-macro(%p)", this);
    }

    virtual malObjectPtr doWithMeta(malHashPtr meta);

private:
    typedef RefCountedPtr<const malLambda> malLambdaPtr;
    const malLambdaPtr m_lambda;
};

namespace mal {
    malObjectPtr boolean(bool value);
    malObjectPtr builtin(const String& name, malBuiltIn::ApplyFunc handler);
    malObjectPtr falseObject();
    malObjectPtr hash(malObjectIter argsBegin, malObjectIter argsEnd);
    malObjectPtr hash(const malHash::Map& map);
    malObjectPtr integer(int value);
    malObjectPtr integer(const String& token);
    malObjectPtr keyword(const String& token);
    malObjectPtr lambda(const StringVec&, malObjectPtr, malEnvPtr);
    malObjectPtr list(const malObjectVec& items);
    malObjectPtr list(malObjectIter begin, malObjectIter end);
    malObjectPtr macro(const malLambda* lambda);
    malObjectPtr nil();
    malObjectPtr string(const String& token);
    malObjectPtr symbol(const String& token);
    malObjectPtr trueObject();
    malObjectPtr vector(const malObjectVec& items);
    malObjectPtr vector(malObjectIter begin, malObjectIter end);
};

#endif // INCLUDE_TYPES_H
