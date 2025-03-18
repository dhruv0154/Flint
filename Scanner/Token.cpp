#include "Token.h"
#include <sstream>
#include <C:\Flint\magic_enum-master\include\magic_enum\magic_enum.hpp>

std::string Token::toString() const
{
    std::stringstream ss;
    
    ss << magic_enum::enum_name(type) << " " << lexeme << " " << line;

    // Check if literal is a string and properly print it
    if (literal.has_value()) 
    {
        if (literal.type() == typeid(std::string)) 
        {
            ss << " " << std::any_cast<std::string>(literal);
        } 
        else if (literal.type() == typeid(double)) 
        {
            ss << " " << std::any_cast<double>(literal);
        }
    }

    return ss.str();
}