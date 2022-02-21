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

#include "Atom/Core/Core.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "Atom/Platform/Windows/AtomWin.h"

#endif