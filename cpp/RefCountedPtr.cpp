#include "RefCountedPtr.h"

#if DEBUG_MEMORY_AUDITING

RefCounted::Set RefCounted::s_tracker;

#endif
