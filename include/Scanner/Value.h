#pragma once

#include <variant>
#include <string>
#include <memory>
#include <vector>
#include <iostream>

using LiteralValue = std::variant<
    std::monostate,       
    double,
    int,
    std::string,
    std::nullptr_t,
    bool
>;