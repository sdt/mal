#include "RefCountedPtr.h"

#if DEBUG_MEMORY_AUDITING

#include "String.h"

#include <typeinfo>

RefCounted::Set* RefCounted::s_tracker;

void RefCounted::dump(const String& indent, Set& seen) const
{
    if (seen.find(this) == seen.end()) {
        seen.insert(this);
        doDump(indent, seen);
    }
    else {
        TRACE("%s%s: (SEEN)\n", indent.c_str(), info().c_str());
    }
}

String RefCounted::info() const
{
    return STRF("%-12s %p (%d ref%s)",
        typeid(*this).name(), this, m_refCount, PLURAL(m_refCount));
}

void RefCounted::mark(int value) const
{
    if (m_mark != value) {  // stop if we've been here already
        m_mark = value;
        doMark(value);
    }
}

void RefCounted::memoryReport(const RefCounted* root)
{
    static int mark = 0;
    mark++;

    root->mark(mark);

    int unreachableObjectCount = 0;
    for (auto &it : *s_tracker) {
        if (it->getMark() != mark) {
            unreachableObjectCount++;

            TRACE("Unreachable object:\n");
            Set seen;
            it->dump("  ", seen);
        }
    }

    TRACE("%d unreachable object%s total.\n",
        unreachableObjectCount, PLURAL(unreachableObjectCount));
}

#endif
