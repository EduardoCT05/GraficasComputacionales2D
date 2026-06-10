#pragma once

// 1. C libraries
#include <cassert>
#include <cstddef>
#include <cstdint>

// 2. C++ libraries
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

// 3. Third-party libraries
#include <SFML/Graphics.hpp>

// MACRO for safe release of resources
#define SAFE_PTR_RELEASE(x) if(x != nullptr) { delete x; x = nullptr; }

#define MESSAGE(classObj, method, state)                                \
{                                                                       \
  std::ostringstream os_;                                               \
  os_ << classObj << "::" << method << " : "                            \
      << "[RESOURCE STATE: " << state << "] \n";                        \
  std::cerr << os_.str();                                               \
}

#define ERROR(classObj, method, errorMSG)                               \
{                                                                       \
  std::ostringstream os_;                                               \
  os_ << "ERROR : " << classObj << "::" << method << " : "              \
      << " Error info: [" << errorMSG << "] \n";                        \
  std::cerr << os_.str();                                               \
  exit(1);                                                              \
}

// ENUMS
enum ShapeType {
    EMPTY = 0,
    CIRCLE = 1,
    RECTANGLE = 2,
    TRIANGLE = 3,
    POLYGON = 4,
    LINE = 5,
};