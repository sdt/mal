#ifndef INCLUDE_TYPES_H
#define INCLUDE_TYPES_H

#include "MAL.h"

#include <exception>
#include <map>

#define ARRAY_SIZE(a)   (sizeof(a)/(sizeof(*(a))))

class malEmptyInputException : public std::exception { };

class malObject : public RefCounted {
public:
    malObject() {
        TRACE_OBJECT("Creating malObject %p\n", this);
    }
    malObject(malObjectPtr meta) : m_meta(meta) {
        TRACE_OBJECT("Creating malObject %p\n", this);
    }
    virtual ~malObject() {
        TRACE_OBJECT("Destroying malObject %p\n", this);
    }

    malObjectPtr withMeta(malObjectPtr meta) const;
    virtual malObjectPtr doWithMeta(malObjectPtr meta) const = 0;
    malObjectPtr meta() const;

    bool isTrue() const;

    bool isEqualTo(const malObject* rhs) const;

    virtual malObjectPtr eval(malEnvPtr env);

    virtual String print(bool readably) const = 0;

protected:
    virtual bool doIsEqualTo(const malObject* rhs) const = 0;

    malObjectPtr m_meta;
};

template<class T>
T* object_cast(malObjectPtr obj, const char* typeName) {
    T* dest = dynamic_cast<T*>(obj.ptr());
    ASSERT(dest != NULL, "%s is not a %s", obj->print(true).c_str(), typeName);
    return dest;
}

#define OBJECT_CAST(Type, Object)   object_cast<Type>(Object, #Type)
#define DYNAMIC_CAST(Type, Object)  (dynamic_cast<Type*>((Object).ptr()))
#define STATIC_CAST(Type, Object)   (static_cast<Type*>((Object).ptr()))

#define WITH_META(Type) \
    virtual malObjectPtr doWithMeta(malObjectPtr meta) const { \
        return new Type(*this, meta); \
    } \

class malConstant : public malObject {
public:
    malConstant(String name) : m_name(name) { }
    malConstant(const malConstant& that, malObjectPtr meta)
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
    malInteger(const malInteger& that, malObjectPtr meta)
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

class malStringBase : public malObject {
public:
    malStringBase(const String& token)
        : m_value(token) { }
    malStringBase(const malStringBase& that, malObjectPtr meta)
        : malObject(meta), m_value(that.value()) { }

    virtual String print(bool readably) const { return m_value; }

    String value() const { return m_value; }

private:
    const String m_value;
};

class malString : public malStringBase {
public:
    malString(const String& token)
        : malStringBase(token) { }
    malString(const malString& that, malObjectPtr meta)
        : malStringBase(that, meta) { }

    virtual String print(bool readably) const;

    String escapedValue() const;

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return value() == static_cast<const malString*>(rhs)->value();
    }

    WITH_META(malString);
};

class malKeyword : public malStringBase {
public:
    malKeyword(const String& token)
        : malStringBase(token) { }
    malKeyword(const malKeyword& that, malObjectPtr meta)
        : malStringBase(that, meta) { }

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return value() == static_cast<const malKeyword*>(rhs)->value();
    }

    WITH_META(malKeyword);
};

class malSymbol : public malStringBase {
public:
    malSymbol(const String& token)
        : malStringBase(token) { }
    malSymbol(const malSymbol& that, malObjectPtr meta)
        : malStringBase(that, meta) { }

    virtual malObjectPtr eval(malEnvPtr env);

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return value() == static_cast<const malSymbol*>(rhs)->value();
    }

    WITH_META(malSymbol);
};

class malSequence : public malObject {
public:
    malSequence(malObjectVec* items);
    malSequence(malObjectIter begin, malObjectIter end);
    malSequence(const malSequence& that, malObjectPtr meta);
    virtual ~malSequence();

    virtual String print(bool readably) const;

    malObjectVec* evalItems(malEnvPtr env) const;
    int count() const { return m_items->size(); }
    bool isEmpty() const { return m_items->empty(); }
    malObjectPtr item(int index) const { return (*m_items)[index]; }

    malObjectIter begin() const { return m_items->begin(); }
    malObjectIter end()   const { return m_items->end(); }

    virtual bool doIsEqualTo(const malObject* rhs) const;

    virtual malObjectPtr conj(malObjectIter argsBegin,
                              malObjectIter argsEnd) const = 0;

    malObjectPtr first() const;
    virtual malObjectPtr rest() const;

private:
    malObjectVec* const m_items;
};

class malList : public malSequence {
public:
    malList(malObjectVec* items) : malSequence(items) { }
    malList(malObjectIter begin, malObjectIter end)
        : malSequence(begin, end) { }
    malList(const malList& that, malObjectPtr meta)
        : malSequence(that, meta) { }

    virtual String print(bool readably) const;
    virtual malObjectPtr eval(malEnvPtr env);

    virtual malObjectPtr conj(malObjectIter argsBegin,
                              malObjectIter argsEnd) const;

    WITH_META(malList);
};

class malVector : public malSequence {
public:
    malVector(malObjectVec* items) : malSequence(items) { }
    malVector(malObjectIter begin, malObjectIter end)
        : malSequence(begin, end) { }
    malVector(const malVector& that, malObjectPtr meta)
        : malSequence(that, meta) { }

    virtual malObjectPtr eval(malEnvPtr env);
    virtual String print(bool readably) const;

    virtual malObjectPtr conj(malObjectIter argsBegin,
                              malObjectIter argsEnd) const;

    WITH_META(malVector);
};

class malApplicable : public malObject {
public:
    malApplicable() { }
    malApplicable(malObjectPtr meta) : malObject(meta) { }

    virtual malObjectPtr apply(malObjectIter argsBegin,
                               malObjectIter argsEnd,
                               malEnvPtr env) const = 0;
};

class malHash : public malObject {
public:
    typedef std::map<String, malObjectPtr> Map;

    malHash(malObjectIter argsBegin, malObjectIter argsEnd);
    malHash(const malHash::Map& map);
    malHash(const malHash& that, malObjectPtr meta)
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

    malBuiltIn(const malBuiltIn& that, malObjectPtr meta)
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

    String name() const { return m_name; }

    WITH_META(malBuiltIn);

private:
    const String m_name;
    ApplyFunc* m_handler;
};

class malLambda : public malApplicable {
public:
    malLambda(const StringVec& bindings, malObjectPtr body, malEnvPtr env);
    malLambda(const malLambda& that, malObjectPtr meta);
    malLambda(const malLambda& that, bool isMacro);

    virtual malObjectPtr apply(malObjectIter argsBegin,
                               malObjectIter argsEnd,
                               malEnvPtr env) const;

    malObjectPtr getBody() const { return m_body; }
    malEnvPtr makeEnv(malObjectIter argsBegin, malObjectIter argsEnd) const;

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return this == rhs; // do we need to do a deep inspection?
    }

    virtual String print(bool readably) const {
        return STRF("#user-%s(%p)", m_isMacro ? "macro" : "function", this);
    }

    bool isMacro() const { return m_isMacro; }

    virtual malObjectPtr doWithMeta(malObjectPtr meta) const;

private:
    const StringVec    m_bindings;
    const malObjectPtr m_body;
    const malEnvPtr    m_env;
    const bool         m_isMacro;
};

class malAtom : public malObject {
public:
    malAtom(malObjectPtr value) : m_value(value) { }
    malAtom(const malAtom& that, malObjectPtr meta)
        : malObject(meta), m_value(that.m_value) { }

    virtual bool doIsEqualTo(const malObject* rhs) const {
        return this->m_value->isEqualTo(rhs);
    }

    virtual String print(bool readably) const {
        return "(atom " + m_value->print(readably) + ")";
    };

    malObjectPtr deref() const { return m_value; }

    malObjectPtr reset(malObjectPtr value) { return m_value = value; }

    WITH_META(malAtom);

private:
    malObjectPtr m_value;
};

namespace mal {
    malObjectPtr atom(malObjectPtr value);
    malObjectPtr boolean(bool value);
    malObjectPtr builtin(const String& name, malBuiltIn::ApplyFunc handler);
    malObjectPtr falseObject();
    malObjectPtr hash(malObjectIter argsBegin, malObjectIter argsEnd);
    malObjectPtr hash(const malHash::Map& map);
    malObjectPtr integer(int value);
    malObjectPtr integer(const String& token);
    malObjectPtr keyword(const String& token);
    malObjectPtr lambda(const StringVec&, malObjectPtr, malEnvPtr);
    malObjectPtr list(malObjectVec* items);
    malObjectPtr list(malObjectIter begin, malObjectIter end);
    malObjectPtr list(malObjectPtr a);
    malObjectPtr list(malObjectPtr a, malObjectPtr b);
    malObjectPtr list(malObjectPtr a, malObjectPtr b, malObjectPtr c);
    malObjectPtr macro(const malLambda& lambda);
    malObjectPtr nil();
    malObjectPtr string(const String& token);
    malObjectPtr symbol(const String& token);
    malObjectPtr trueObject();
    malObjectPtr vector(malObjectVec* items);
    malObjectPtr vector(malObjectIter begin, malObjectIter end);
};

#endif // INCLUDE_TYPES_H
