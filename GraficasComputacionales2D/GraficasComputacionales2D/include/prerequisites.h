#pragma once

//librerias STD
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <map>
#include <fstream>
#include <unordered_map>
#include <limits>
#include <cassert>
#include <utility>
#include <tuple>
#include <cstddef>
#include <queue>

#include <imgui.h>
#include <imgui-SFML.h>

//Third parties
#include <SFML/Graphics.hpp>

// MACRO for safe release of resources
//Macro is a code that the compiler replaces textually
#define SAFE_PTR_RELEASE(x) if(x != nullptr) { delete x; x = nullptr; }

#define MESSAGE(classObj, method, state)                      \
{                                                             \
    std::ostringstream os_;                                   \
    os_ << classObj << "::" << method << " : "                \
        << "[CREATION OF RESOURCE" << ": " << state "] \n";\
    std::cerr << os_.str();                                   \
}

#define ERROR(classObj, method, errorMSG)                         \
{                                                                 \
    std::ostringstream os_;                                       \
    os_ << "ERROR : " << classObj << "::" << method << " : "      \
        << "  Error in data from params [" << errorMSG"] \n"; \
    std::cerr << os_.str();                                       \
    exit(1);                                                      \
}

#include <memory>
//ENUMS
enum
    ShapeType {
    EMPTY = 0,
    CIRCLE = 1,
    RECTANGLE = 2,
    TRIANGLE = 3,
    POLYGON = 4,
    LINE = 5
};