#ifndef INCLUDE_REFCOUNTEDPTR_H
#define INCLUDE_REFCOUNTEDPTR_H

#include "RefCounted.h"

#include <cstddef>

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

    operator bool () {
        return m_object != NULL;
    }

    ~RefCountedPtr() {
        release();
    }

    T* operator -> () { return m_object; }

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
