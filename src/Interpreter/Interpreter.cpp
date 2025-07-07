#include "Interpreter/Interpreter.h"
#include "ExpressionNode.h"
#include "RuntimeError.h"
#include "Flint/Flint.h"

// ─────────────────────────────────────────────────────────────────────────────
// Global Interpreter State
// The Evaluator handles expression evaluation (Binary, Literal, etc.).
// The Environment stores runtime variables.
// ─────────────────────────────────────────────────────────────────────────────
std::unique_ptr<Evaluator> Interpreter::evaluator = std::make_unique<Evaluator>();
std::unique_ptr<Environment> Interpreter::environment = std::make_unique<Environment>();

// ─────────────────────────────────────────────────────────────────────────────
// interpret()
// Entry point for executing parsed AST statements.
// Executes each statement in order. If a runtime error occurs,
// it is caught and forwarded to Flint's error reporting mechanism.
// ─────────────────────────────────────────────────────────────────────────────
void Interpreter::interpret(std::vector<std::shared_ptr<Statement>> statements) const
{
    try
    {
        for (std::shared_ptr<Statement> s : statements)
        {
            execute(s);
        }  
    }
    catch (const RuntimeError error)
    {
        Flint::runtimeError(error);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// execute()
// Dispatches a statement using std::visit. This lets us call the appropriate
// overloaded `operator()` based on which statement variant (Print, Let, etc.)
// ─────────────────────────────────────────────────────────────────────────────
void Interpreter::execute(std::shared_ptr<Statement> statement) const
{
    std::visit(*this, *statement.get());
}

// ─────────────────────────────────────────────────────────────────────────────
// stringify()
// Converts any LiteralValue into a human-readable string for output.
// This is used by `print` statements.
// Handles:
//   • Numbers → trims trailing zeros
//   • Strings → as-is
//   • Booleans → true/false
//   • nullptr / monostate → "NOTHING"
// ─────────────────────────────────────────────────────────────────────────────
std::string Interpreter::stringify(const LiteralValue& obj)
{
    auto func = [] (auto&& val) -> std::string 
    {
        using T = std::decay_t<decltype(val)>;

        if constexpr (std::is_same_v<T, nullptr_t> || std::is_same_v<T, std::monostate>) return "NOTHING";
        else if constexpr (std::is_same_v<T, bool>) return val ? "true" : "false";
        else if constexpr (std::is_same_v<T, std::string>) return val;
        else if constexpr (std::is_same_v<T, double>)
        {
            std::string text = std::to_string(val);
            text.erase(text.find_last_not_of('0') + 1);  // Remove trailing zeroes
            if (text.back() == '.') text.pop_back();     // Remove lone dot
            return text;
        }
        else return std::to_string(val);  // fallback: int, etc.
    };

    return std::visit(func, obj);
}

// ─────────────────────────────────────────────────────────────────────────────
// Statement Visitor Implementations
// These overloads are invoked via std::visit inside `execute()`.
// ─────────────────────────────────────────────────────────────────────────────

// PRINT statement: evaluates the expression, prints its string form
void Interpreter::operator()(const PrintStmt& stmt) const 
{
    LiteralValue val = evaluator->evaluate(stmt.expression);
    std::cout << Interpreter::stringify(val) << '\n';
}

// Expression statement: evaluates expression and discards result (side effects only)
void Interpreter::operator()(const ExpressionStmt& exprStatement) const
{
    evaluator->evaluate(exprStatement.expression);
}

// LET statement: evaluates right-hand expression and stores it in the environment
void Interpreter::operator()(const LetStmt& letStatement) const
{
    LiteralValue value = nullptr;

    // Optional: variable declaration without initializer
    if (letStatement.expression != nullptr)
        value = evaluator->evaluate(letStatement.expression);

    environment->define(letStatement.name.lexeme, value);
}