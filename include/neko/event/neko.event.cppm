/**
 * @file neko.event.cppm
 * @brief C++20 module interface for NekoEvent
 * @details This module exports all NekoEvent functionality by wrapping the header files.
 *          The original headers are still available for traditional include-based usage.
 */

module;

// Global module fragment - include headers that should not be exported
#include <any>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>

#include <chrono>

#include <functional>
#include <future>
#include <optional>

#include <memory>

#include <queue>
#include <string>
#include <vector>

#include <type_traits>
#include <typeindex>

#include <unordered_map>
#include <unordered_set>

#include <algorithm>

export module neko.event;

import neko.schema;

// Control header files to not import dependencies (dependencies are declared and imported by the cppm)
#define NEKO_EVENT_ENABLE_MODULE true

export {
    #include "event.hpp"
}
