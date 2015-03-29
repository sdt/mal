#ifndef INCLUDE_REFCOUNTEDPTR_H
#define INCLUDE_REFCOUNTEDPTR_H

#include "Debug.h"

#include <cstddef>

#ifdef DEBUG_MEMORY_AUDITING
    #include <set>
#endif

class RefCounted {
public:
    RefCounted() : m_refCount(0)
    {
#ifdef DEBUG_MEMORY_AUDITING
        if (s_tracker == NULL) {
            s_tracker = new Set;
        }
        s_tracker->insert(this);
#endif
    }
    virtual ~RefCounted()
    {
#ifdef DEBUG_MEMORY_AUDITING
        s_tracker->erase(this);
#endif
    }

    const RefCounted* acquire() const { m_refCount++; return this; }
    int release() const { return --m_refCount; }
    int refCount() const { return m_refCount; }

#ifdef DEBUG_MEMORY_AUDITING
    typedef std::set<RefCounted*>   Set;
    typedef Set::iterator           SetIter;

    static SetIter begin() { return s_tracker->begin(); }
    static SetIter end()   { return s_tracker->end(); }

    void mark(int value) const {
        if (m_mark != value) {  // stop if we've been here already
            m_mark = value;
            doMark(value);
        }
    }

    int getMark() const { return m_mark; }
#endif

private:
    RefCounted(const RefCounted&); // no copy ctor
    RefCounted& operator = (const RefCounted&); // no assignments

    mutable int m_refCount;

#ifdef DEBUG_MEMORY_AUDITING
    virtual void doMark(int value) const = 0;
    mutable int m_mark;
    static Set* s_tracker;
#endif
};

template<class T>
class RefCountedPtr {
public:
    RefCountedPtr() : m_object(0) { }

    RefCountedPtr(T* object) : m_object(0)
    { acquire(object); }

    RefCountedPtr(const RefCountedPtr& rhs) : m_object(0)
    { acquire(rhs.m_object); }

    const RefCountedPtr& operator = (const RefCountedPtr& rhs) {
        acquire(rhs.m_object);
        return *this;
    }

    bool operator == (const RefCountedPtr& rhs) const {
        return m_object == rhs.m_object;
    }

    bool operator != (const RefCountedPtr& rhs) const {
        return m_object != rhs.m_object;
    }

    operator bool () const {
        return m_object != NULL;
    }

    ~RefCountedPtr() {
        release();
    }

    T* operator -> () const { return m_object; }
    T* ptr() const { return m_object; }

private:
    void acquire(T* object) {
        if (object != NULL) {
            object->acquire();
        }
        release();
        m_object = object;
    }

    void release() {
        if ((m_object != NULL) && (m_object->release() == 0)) {
            delete m_object;
        }
    }

    T* m_object;
};

#endif // INCLUDE_REFCOUNTEDPTR_H
