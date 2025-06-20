#include "Scanner\Token.h"
#include <sstream>
#include <iomanip>
#include <C:\Flint\magic_enum-master\include\magic_enum\magic_enum.hpp>

std::string Token::toString() const
{
    std::stringstream ss;
    
    ss << magic_enum::enum_name(type) << " " << lexeme << " " << line;

    auto func = [](auto&& val) -> std::string {
        using T = std::decay_t<decltype(val)>;

        if constexpr (std::is_same_v<T, std::string>) return val;
        else if constexpr(std::is_same_v<T, double>) 
        {
            std::stringstream r;
            r << std::fixed << std::setprecision(2);
            r << val;
            return r.str();
        }
        else if constexpr(std::is_same_v<T, int>) return std::to_string(val);
        else if constexpr (std::is_same_v<T, std::nullptr_t>) return "nullptr";
        else return "NULL";
    };

    ss << " " << std::visit(func, literal);

    return ss.str();
}