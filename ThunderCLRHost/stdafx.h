// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <string>
#include <filesystem>
#include <vector>
#include <functional>

#include <metahost.h>
#include <atlbase.h>

// Import mscorlib.tlb (Microsoft Common Language Runtime Class Library).
#import "mscorlib.tlb" raw_interfaces_only				\
    high_property_prefixes("_get","_put","_putref")		\
    rename("ReportEvent", "InteropServices_ReportEvent")

#define BLACKBONE_STATIC

#include <BlackBone\Process\Process.h>
#include <BlackBone\PE\PEImage.h>

using namespace mscorlib;

using namespace std::experimental;