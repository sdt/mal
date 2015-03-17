#ifndef INCLUDE_REFCOUNTED_H
#define INCLUDE_REFCOUNTED_H

class RefCounted {
public:
    RefCounted() : m_refCount(0) { }
    virtual ~RefCounted() { }

    RefCounted* acquire() { m_refCount++; return this; }
    int release() { return --m_refCount; }
    int refCount() { return m_refCount; }

private:
    RefCounted(const RefCounted&); // no copy ctor
    RefCounted& operator = (const RefCounted&); // no assignments

    int m_refCount;
};

#endif // INCLUDE_REFCOUNTED_H
