#pragma once

#if defined(NDEBUG)
#undef NDEBUG
#include <cassert>
#define NDEBUG
#else
#include <cassert>
#endif
