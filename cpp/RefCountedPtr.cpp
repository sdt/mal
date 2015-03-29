#include "RefCountedPtr.h"

#if DEBUG_MEMORY_AUDITING

#include "String.h"

RefCounted::Set* RefCounted::s_tracker;

void RefCounted::memoryReport(const RefCounted* root)
{
    static int mark = 0;
    mark++;

    root->mark(mark);

    int unreachableObjectCount = 0;
    for (auto &it : *s_tracker) {
        if (it->getMark() != mark) {
            unreachableObjectCount++;

            TRACE("Unreachable object at %p\n", it);
        }
    }

    TRACE("%d unreachable object%s total.\n",
        unreachableObjectCount, PLURAL(unreachableObjectCount));
}

#endif
