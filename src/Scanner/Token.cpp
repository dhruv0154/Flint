// ---------------------------------------------------------------------------
// Implements the `toString()` method for the Token class.
// This function provides a human-readable representation of a token,
// useful for debugging the lexer or AST. Format:
//   LEXEME LINE LITERAL
// Example output:
//   "=" 5 "true"
// ---------------------------------------------------------------------------

#include "Scanner/Token.h"
#include <sstream>  // For building the output string
#include <iomanip>  // For controlling float precision

std::string Token::toString() const
{
    std::stringstream ss;

    // 1) Print the raw lexeme (the exact text from source)
    ss << lexeme << " ";

    // 2) Print the line number where this token was found
    ss << line << " ";

    // 3) Format the literal variant into a string
    //    We use std::visit to handle each possible type in the union.
    auto formatLiteral = [](auto&& val) -> std::string {
        using T = std::decay_t<decltype(val)>;

        // String literals: wrap in quotes
        if constexpr (std::is_same_v<T, std::string>) {
            return "\"" + val + "\"";
        }
        // Floating-point numbers: fixed notation with two decimals
        else if constexpr (std::is_same_v<T, double>) {
            std::ostringstream r;
            r << std::fixed << std::setprecision(2) << val;
            return r.str();
        }
        // Integers: plain decimal
        else if constexpr (std::is_same_v<T, int>) {
            return std::to_string(val);
        }
        // Booleans: "true" or "false"
        else if constexpr (std::is_same_v<T, bool>) {
            return val ? "true" : "false";
        }
        // nullptr_t and monostate: represent absence of value
        else if constexpr (std::is_same_v<T, std::nullptr_t> ||
                           std::is_same_v<T, std::monostate>) {
            return "nothing";
        }
        // Other types should not occur here; fallback safely
        else {
            return "unknown";
        }
    };

    // 4) Append the formatted literal to the output
    ss << std::visit(formatLiteral, literal);

    // 5) Return the assembled string
    return ss.str();
}