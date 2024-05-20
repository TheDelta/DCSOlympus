#pragma once

#define DllExport __declspec(dllexport)

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

#include <iostream>
#include <string>
#include <time.h>
#include <chrono>
#include <string>
#include <map>
#include <list>
#include <fstream>
#include <iostream>
#include <cstdarg>
#include <filesystem>
#include <codecvt>
#include <set>
#include <mutex>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "luaconf.h"
#include "lauxlib.h"
}