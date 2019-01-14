#pragma once

// Unless you are very confident, don't set either OS flag
#if defined(PXX_OS_UNIX) &&   defined(PXX_OS_WINDOWS)
#error Both Unix and Windows flags are set, which is not allowed!
#elif defined(PXX_OS_UNIX)
#pragma message Using defined Unix flag
#elif defined(PXX_OS_WINDOWS)
#pragma message Using defined Windows flag
#else
#if defined(unix)        || defined(__unix)      || defined(__unix__) \
	|| defined(linux) || defined(__linux) || defined(__linux__) \
	|| defined(sun) || defined(__sun) \
	|| defined(BSD) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| defined(__FreeBSD__) || defined (__DragonFly__) \
	|| defined(sgi) || defined(__sgi) \
	|| (defined(__MACOSX__) || defined(__APPLE__)) \
	|| defined(__CYGWIN__) || defined(__MINGW32__)
#define PXX_OS_UNIX	1	//!< Unix like OS(POSIX compliant)
#undef PXX_OS_WINDOWS
#elif defined(_MSC_VER) || defined(WIN32)  || defined(_WIN32) || defined(__WIN32__) \
	|| defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#define PXX_OS_WINDOWS	1	//!< Microsoft Windows
#undef PXX_OS_UNIX
#else
#error Unable to support this unknown OS.
#endif
#endif

#if PXX_OS_WINDOWS
#if _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
#include <windows.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#elif PXX_OS_UNIX
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif