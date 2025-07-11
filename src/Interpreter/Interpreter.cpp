#include <chrono>
#include "Interpreter/Interpreter.h"
#include "Flint/Flint.h"
#include "ExpressionNode.h"
#include "BreakException.h"
#include "ContinueException.h"
#include "RuntimeError.h"
#include "FlintCallable.h"
#include "NativeFunction.h"

// ─────────────────────────────────────────────────────────────────────────────
// Global Interpreter State
// The Evaluator handles expression evaluation (Binary, Literal, etc.).
// The Environment stores runtime variables.
// ─────────────────────────────────────────────────────────────────────────────

Interpreter::Interpreter()
{
    globals = std::make_shared<Environment>();
    environment = globals;
    evaluator = std::make_unique<Evaluator>(environment, *this);

    // Define clock()
    globals -> define("clock", std::make_shared<NativeFunction>(
    0,
    [](const std::vector<LiteralValue>&, const Token&) -> LiteralValue {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()).count();
        return static_cast<double>(ms) / 1000.0;
    },
    "clock"
    ));

    globals -> define("scan", std::make_shared<NativeFunction>(
        -1,
        [](const std::vector<LiteralValue>& args, 
            const Token &paren) -> LiteralValue 
        {
            
            if(args.size() > 1) 
                throw RuntimeError(paren, "scan() takes at most 1 argument.");
            
            if(!args.empty() && !std::holds_alternative<std::string>(args[0]))
                throw RuntimeError(paren, "scan() expects string as prompt.");
            
            if(!args.empty())
                std::cout << std::get<std::string>(args[0]);
            
            std::string line;
            std::getline(std::cin, line);
            line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1); // trim right
            line.erase(0, line.find_first_not_of(" \t\n\r\f\v")); // trim left
            return line;
        },
        "scan"
    ));

    globals->define("print", std::make_shared<NativeFunction>(
    -1, // -1 means variadic
    [](const std::vector<LiteralValue>& args, const Token&) -> LiteralValue {
        for (const auto& arg : args)
            std::cout << Interpreter::stringify(arg);
        return nullptr;
    },
    "print"
    ));
}

// ─────────────────────────────────────────────────────────────────────────────
// interpret()
// Entry point for executing parsed AST statements.
// Executes each statement in order. If a runtime error occurs,
// it is caught and forwarded to Flint's error reporting mechanism.
// ─────────────────────────────────────────────────────────────────────────────
void Interpreter::interpret(std::vector<std::shared_ptr<Statement>> statements) const
{
    for (std::shared_ptr<Statement> s : statements)
    {
        try {
            execute(s);
        } catch (const RuntimeError& error) {
            Flint::runtimeError(error);
            // Continue with next statement
        }
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

void Interpreter::executeBlock(std::vector<std::shared_ptr<Statement>> statements,
    std::shared_ptr<Environment> newEnv) const
{
    auto previous = environment;
    environment = newEnv;

    evaluator -> environment = newEnv;

    try
    {
        for (const auto& statement : statements)
        {
            execute(statement);
        } 
    }
    catch (...) 
    {
        environment = previous;
        evaluator->environment = previous;
        throw;
    }

    environment = previous;
    evaluator->environment = previous;
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
        else if constexpr (std::is_same_v<T, std::shared_ptr<FlintCallable>>) {
            return val -> toString();  // call custom string method on function
        } 
        else {
            return "<unknown>";
        }
    };

    return std::visit(func, obj);
}

// ─────────────────────────────────────────────────────────────────────────────
// Statement Visitor Implementations
// These overloads are invoked via std::visit inside `execute()`.
// ─────────────────────────────────────────────────────────────────────────────
void Interpreter::operator()(const IfStmt& stmt) const
{
    if(evaluator -> isTruthy(evaluator -> evaluate(stmt.condition))) execute(stmt.thenBranch);
    else if(stmt.elseBranch) execute(stmt.elseBranch);
}

void Interpreter::operator()(const WhileStmt& stmt) const
{
    bool previousInsideLoop = isInsideLoop;
    isInsideLoop = true;
    while (evaluator -> isTruthy(evaluator -> evaluate(stmt.condition)))
    { 
        try
        {
            execute(stmt.statement);
        }
        catch(const BreakException& e)
        {
            break;
        }
        catch(const ContinueException& e)
        {
            continue;
        } 
    }
    isInsideLoop = previousInsideLoop;
}

void Interpreter::operator()(const BreakStmt& stmt) const
{
    if(!isInsideLoop) throw RuntimeError(stmt.keyword, "Cannot use 'break' outside of a loop");
    throw BreakException();
}

void Interpreter::operator()(const ContinueStmt& stmt) const
{
    if(!isInsideLoop) throw RuntimeError(stmt.keyword, "Cannot use 'continue' outside of a loop");
    throw ContinueException();
}

void Interpreter::operator()(const TryCatchContinueStmt& stmt) const
{
    try
    {
        execute(stmt.body);
    }
    catch(const ContinueException& e)
    {
    
    }
    
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

void Interpreter::operator()(const BlockStmt& blockStatement) const
{
    auto env = std::make_shared<Environment>(environment);
    executeBlock(blockStatement.statements, env);
}