#ifndef INCLUDE_REFCOUNTED_H
#define INCLUDE_REFCOUNTED_H

class RefCounted {
public:
    RefCounted() : m_refCount(0) { }
    virtual ~RefCounted() { }

    const RefCounted* acquire() const { m_refCount++; return this; }
    int release() const { return --m_refCount; }
    int refCount() const { return m_refCount; }

private:
    RefCounted(const RefCounted&); // no copy ctor
    RefCounted& operator = (const RefCounted&); // no assignments

    mutable int m_refCount;
};

#endif // INCLUDE_REFCOUNTED_H
