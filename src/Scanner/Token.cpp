// ---------------------------------------------------------------------------
// Implements the `toString()` method for the Token class.
// This function provides a human-readable representation of a token,
// useful for debugging the lexer or AST. Format:
//   TokenType LEXEME LINE_LITERAL
// Example:
//   EQUAL = 5 42
// ---------------------------------------------------------------------------

#include "Scanner/Token.h"
#include <sstream>                  // For building the output string
#include <iomanip>                  // For controlling float precision

// ---------------------------------------------------------------------------
// Returns a string representation of the token, including:
// - The token type (e.g., IDENTIFIER, EQUAL)
// - The original source lexeme (e.g., "x", "=", "true")
// - The line number where the token appeared
// - The resolved literal value (if any)
// ---------------------------------------------------------------------------
std::string Token::toString() const
{
    std::stringstream ss;

    // Use magic_enum to get the enum name as string (e.g., "EQUAL")
    ss << lexeme << " "
       << line << " ";

    // Format literal using std::visit for the variant type
    auto formatLiteral = [](auto&& val) -> std::string {
        using T = std::decay_t<decltype(val)>;

        if constexpr (std::is_same_v<T, std::string>) {
            return "\"" + val + "\"";
        } 
        else if constexpr (std::is_same_v<T, double>) {
            std::ostringstream r;
            r << std::fixed << std::setprecision(2) << val;
            return r.str();
        } 
        else if constexpr (std::is_same_v<T, int>) {
            return std::to_string(val);
        } 
        else if constexpr (std::is_same_v<T, bool>) {
            return val ? "true" : "false";
        } 
        else if constexpr (std::is_same_v<T, std::nullptr_t> || std::is_same_v<T, std::monostate>) {
            return "nothing";
        } 
        else {
            return "unknown";  // Fallback (should not happen in practice)
        }
    };

    // Apply formatter to the variant literal
    ss << std::visit(formatLiteral, literal);

    return ss.str();
}