#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <stdlib.h>
#include <filesystem>
#include <memory>
#include <functional>
#include <sstream>
#include <chrono>
#include <mutex>
#include <thread>

#if defined(ATOM_PLATFORM_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#endif