// ------------------------------------------------------------
// Implements the `toString()` method for the Token class.
// This function provides a human-readable string representation
// of a token, including its type, lexeme, line number, and literal value.
// ------------------------------------------------------------

#include "Scanner\Token.h"                        // Token class definition
#include <sstream>                                 // For stringstream formatting
#include <iomanip>                                 // For setprecision (float formatting)
#include <C:\Flint\magic_enum-master\include\magic_enum\magic_enum.hpp> // For enum name extraction

// ---------------------------------------------------------------------------
// Converts a token to a readable string:
//   [TokenType] [lexeme] [line] [literal value]
// ---------------------------------------------------------------------------
std::string Token::toString() const
{
    std::stringstream ss;

    // Use magic_enum to convert enum value to its name
    ss << magic_enum::enum_name(type) << " " << lexeme << " " << line;

    // Lambda function to format the literal value using std::visit
    auto func = [](auto&& val) -> std::string {
        using T = std::decay_t<decltype(val)>;

        // Handle each possible type in the variant
        if constexpr (std::is_same_v<T, std::string>) return val;
        else if constexpr (std::is_same_v<T, double>) 
        {
            std::stringstream r;
            r << std::fixed << std::setprecision(2); // Format floating points with 2 decimal places
            r << val;
            return r.str();
        }
        else if constexpr (std::is_same_v<T, int>) return std::to_string(val);
        else if constexpr (std::is_same_v<T, std::nullptr_t>) return "nullptr";
        else if constexpr (std::is_same_v<T, bool>) return val ? "true" : "false";
        else return "NULL"; // Fallback (shouldn't occur if variant is exhaustive)
    };

    // Format and append literal value
    ss << " " << std::visit(func, literal);

    return ss.str();
}
