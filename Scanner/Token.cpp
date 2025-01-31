#include "Token.h"
#include <sstream>
#include <C:\Flint\magic_enum-master\include\magic_enum\magic_enum.hpp>

std::string Token::toString() const
{
    std::stringstream ss;

    ss << magic_enum::enum_name(type) << " " << lexeme << " " << line << " " << literal.type().name();
    return ss.str();
}