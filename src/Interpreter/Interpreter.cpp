#include <chrono>
#include "Interpreter/Interpreter.h"
#include "Flint/Flint.h"
#include "ExpressionNode.h"
#include "BreakException.h"
#include "ContinueException.h"
#include "RuntimeError.h"
#include "FlintCallable.h"
#include "NativeFunction.h"
#include "FlintFunction.h"
#include "ReturnException.h"
#include "FlintClass.h"
#include "FlintInstance.h"

// ─────────────────────────────────────────────────────────────────────────────
// Global Interpreter State
// - `globals` holds built-in functions and top–level vars.
// - `environment` points to the current variable scope.
// - `evaluator` handles all Expr evaluation.
// ─────────────────────────────────────────────────────────────────────────────
Interpreter::Interpreter()
{
    // Create the global (outermost) environment
    globals     = std::make_shared<Environment>();
    environment = globals;
    evaluator   = std::make_unique<Evaluator>(*this);

    // Define native clock(): returns seconds since epoch as double
    globals->define("clock", std::make_shared<NativeFunction>(
        0,  // arity 0 (no args)
        [](const std::vector<LiteralValue>& /*args*/, const Token& /*paren*/) -> LiteralValue {
            // capture current time
            auto now = std::chrono::system_clock::now();
            // convert to milliseconds since epoch
            auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                           now.time_since_epoch()
                       ).count();
            // return seconds as double
            return static_cast<double>(ms) / 1000.0;
        },
        "clock"
    ));

    // Define scan(prompt?): read a line from stdin, trim, return number or string
    globals->define("scan", std::make_shared<NativeFunction>(
        -1,  // variadic: 0 or 1 arg
        [this](const std::vector<LiteralValue>& args, const Token &paren) -> LiteralValue {
            // scan takes at most one prompt string
            if (args.size() > 1)
                throw RuntimeError(paren, "scan() takes at most 1 argument.");
            // if provided, it must be a string
            if (!args.empty() && !std::holds_alternative<std::string>(args[0]))
                throw RuntimeError(paren, "scan() expects string as prompt.");
            // print prompt if given
            if (!args.empty())
                std::cout << std::get<std::string>(args[0]);
            // read a full line
            std::string line;
            std::getline(std::cin, line);
            // trim whitespace
            line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
            line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
            // if it's a valid number, return as double, else as string
            return this->isNumber(line)
                ? LiteralValue(std::stod(line))
                : LiteralValue(line);
        },
        "scan"
    ));

    // Define print(...): writes each arg’s stringified form to stdout
    globals->define("print", std::make_shared<NativeFunction>(
        -1,  // variadic
        [](const std::vector<LiteralValue>& args, const Token& /*paren*/) -> LiteralValue {
            for (const auto& arg : args) {
                std::cout << Interpreter::stringify(arg);
            }
            return nullptr;  // print returns NOTHING
        },
        "print"
    ));

    // Define intDiv(a, b): integer division on two numeric args
    globals->define("intDiv", std::make_shared<NativeFunction>(
        2,  // exactly two args
        [](const std::vector<LiteralValue>& args, const Token& paren) -> LiteralValue {
            // both must be numbers
            if (!std::holds_alternative<double>(args[0]) ||
                !std::holds_alternative<double>(args[1])) {
                throw RuntimeError(paren, "intDiv() expects two numbers.");
            }
            int a = static_cast<int>(std::get<double>(args[0]));
            int b = static_cast<int>(std::get<double>(args[1]));
            if (b == 0) throw RuntimeError(paren, "Division by zero.");
            // integer division semantics
            return static_cast<double>(a / b);
        },
        "intDiv"
    ));

    // Define toString(x): convert a single number to its trimmed string form
    globals->define("toString", std::make_shared<NativeFunction>(
        1,  // exactly one arg
        [this](const std::vector<LiteralValue>& args, const Token& paren) -> LiteralValue {
            if (args.size() > 1)
                throw RuntimeError(paren, "toString() takes at most 1 argument.");
            if (!std::holds_alternative<double>(args[0]))
                throw RuntimeError(paren, "toString() takes a number as an argument.");
            // reuse our stringify logic
            return stringify(args[0]);
        },
        "toString"
    ));
}

// ─────────────────────────────────────────────────────────────────────────────
// interpret()
//   Execute a list of top–level statements, catching and reporting runtime
//   errors via Flint::runtimeError, then continuing.
// ─────────────────────────────────────────────────────────────────────────────
void Interpreter::interpret(std::vector<std::shared_ptr<Statement>> statements) const
{
    for (auto& stmt : statements) {
        try {
            execute(stmt);
        } catch (const RuntimeError& error) {
            Flint::runtimeError(error);
            // then keep going with the next statement
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// execute()
//   Dispatch a single statement node via std::visit, which calls the
//   matching operator()(StmtType).
// ─────────────────────────────────────────────────────────────────────────────
void Interpreter::execute(std::shared_ptr<Statement> statement) const
{
    std::visit(*this, *statement.get());
}

// ─────────────────────────────────────────────────────────────────────────────
// executeBlock()
//   Temporarily switch into a new inner environment for
//   block-local variables, then restore the previous one.
//   Any exception (break/return/etc) is propagated after cleanup.
// ─────────────────────────────────────────────────────────────────────────────
void Interpreter::executeBlock(std::vector<std::shared_ptr<Statement>> statements,
                               std::shared_ptr<Environment> newEnv) const
{
    auto previous = environment;
    environment   = newEnv;

    try {
        for (auto& stmt : statements) {
            execute(stmt);
        }
    } catch (...) {
        // restore env on any exit
        environment = previous;
        throw;
    }
    // normal exit: restore
    environment = previous;
}

// ─────────────────────────────────────────────────────────────────────────────
// resolve()
//   Called by the resolver to map each ExprPtr to its lexical distance.
//   Stored in 'locals' so lookup knows which env level to consult.
// ─────────────────────────────────────────────────────────────────────────────
void Interpreter::resolve(ExprPtr expr, int depth)
{
    locals[expr] = depth;
}

// ─────────────────────────────────────────────────────────────────────────────
// stringify()
//   Turns any LiteralValue into human‐readable text for print()
//   - trims trailing zeros on doubles, etc.
//
// Note: uses std::visit to discriminate types in the variant.
// ─────────────────────────────────────────────────────────────────────────────
std::string Interpreter::stringify(const LiteralValue& obj)
{
    auto func = [](auto&& val) -> std::string {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, std::nullptr_t> ||
                      std::is_same_v<T, std::monostate>)
            return "NOTHING";
        else if constexpr (std::is_same_v<T, bool>)
            return val ? "true" : "false";
        else if constexpr (std::is_same_v<T, std::string>)
            return val;
        else if constexpr (std::is_same_v<T, double>) {
            // convert to string then strip trailing '0's and lone '.'
            std::string text = std::to_string(val);
            text.erase(text.find_last_not_of('0') + 1);
            if (text.back() == '.') text.pop_back();
            return text;
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<FlintCallable>>)
            return val->toString();
        else if constexpr (std::is_same_v<T, std::shared_ptr<FlintClass>>)
            return val->toString();
        else if constexpr (std::is_same_v<T, std::shared_ptr<FlintInstance>>)
            return val->toString();
        else
            return "<unknown>";
    };

    return std::visit(func, obj);
}

// ─────────────────────────────────────────────────────────────────────────────
// isNumber()
//   Utility: tests if a trimmed string is a valid floating‐point literal.
// ─────────────────────────────────────────────────────────────────────────────
bool Interpreter::isNumber(const std::string& str)
{
    char* end = nullptr;
    std::strtod(str.c_str(), &end);
    // valid if strtod consumed the entire string
    return end != str.c_str() && *end == '\0';
}


// ─────────────────────────────────────────────────────────────────────────────
// Statement Visitor Implementations
// Each operator()(StmtType) is invoked via std::visit in execute().
// ─────────────────────────────────────────────────────────────────────────────

void Interpreter::operator()(const IfStmt& stmt) const
{
    // evaluate condition, choose branch
    if (evaluator->isTruthy(evaluator->evaluate(stmt.condition))) {
        execute(stmt.thenBranch);
    } else if (stmt.elseBranch) {
        execute(stmt.elseBranch);
    }
}

void Interpreter::operator()(const WhileStmt& stmt) const
{
    // mark that we’re inside a loop for break/continue checks
    bool prevInside = isInsideLoop;
    isInsideLoop     = true;

    // run until condition becomes false or a BreakException occurs
    while (evaluator->isTruthy(evaluator->evaluate(stmt.condition))) {
        try {
            execute(stmt.statement);
        }
        catch (const BreakException&) {
            break;  // exit loop
        }
        catch (const ContinueException&) {
            continue;  // next iteration
        }
    }

    // restore previous loop flag
    isInsideLoop = prevInside;
}

void Interpreter::operator()(const FunctionStmt &stmt) const
{
    // define a new function object, capturing current env as closure
    auto ptr = std::make_shared<FunctionStmt>(stmt);
    auto fn  = std::make_shared<FlintFunction>(ptr, environment, /*isInit=*/false);
    environment->define(stmt.name->lexeme, fn);
}

void Interpreter::operator()(const ReturnStmt &stmt) const
{
    // evaluate return value if any, then unwind via exception
    LiteralValue val = nullptr;
    if (stmt.val) val = evaluator->evaluate(stmt.val);
    throw ReturnException(val);
}

void Interpreter::operator()(const BreakStmt& stmt) const
{
    if (!isInsideLoop)
        throw RuntimeError(stmt.keyword, "Cannot use 'break' outside of a loop");
    throw BreakException();
}

void Interpreter::operator()(const ContinueStmt& stmt) const
{
    if (!isInsideLoop)
        throw RuntimeError(stmt.keyword, "Cannot use 'continue' outside of a loop");
    throw ContinueException();
}

void Interpreter::operator()(const TryCatchContinueStmt& stmt) const
{
    // swallow any ContinueException inside a try–continue block
    try {
        execute(stmt.body);
    } catch (const ContinueException&) {
        // effectively “catch and ignore”
    }
}

void Interpreter::operator()(const ExpressionStmt& exprStatement) const
{
    // just evaluate for side effects, ignore the result
    evaluator->evaluate(exprStatement.expression);
}

void Interpreter::operator()(const LetStmt& letStatement) const
{
    // define one or more variables, with optional initializers
    for (auto& [name, initializer] : letStatement.declarations) {
        LiteralValue value = nullptr;
        if (initializer) {
            value = evaluator->evaluate(initializer);
        }
        environment->define(name.lexeme, value);
    }
}

void Interpreter::operator()(const BlockStmt& blockStatement) const
{
    // create inner environment chained to current, then execute
    auto inner = std::make_shared<Environment>(environment);
    executeBlock(blockStatement.statements, inner);
}

void Interpreter::operator()(const ClassStmt& classStmt) const
{
    // first bind a placeholder so methods can reference the class name
    environment->define(classStmt.name.lexeme, nullptr);

    // prepare method maps
    std::unordered_map<std::string, std::shared_ptr<FlintFunction>> classMethods;
    std::unordered_map<std::string, std::shared_ptr<FlintFunction>> instanceMethods;

    // wrap each declared method in a FlintFunction
    for (auto& method : classStmt.classMethods) {
        auto ptr      = std::make_shared<FunctionStmt>(std::get<FunctionStmt>(*method));
        bool isCtor   = (ptr->name->lexeme == "init");
        classMethods[ptr->name->lexeme] = std::make_shared<FlintFunction>(ptr, environment, isCtor);
    }
    for (auto& method : classStmt.instanceMethods) {
        auto ptr      = std::make_shared<FunctionStmt>(std::get<FunctionStmt>(*method));
        bool isCtor   = (ptr->name->lexeme == "init");
        instanceMethods[ptr->name->lexeme] = std::make_shared<FlintFunction>(ptr, environment, isCtor);
    }

    // create the class object and overwrite the placeholder
    auto klass = std::make_shared<FlintClass>(
        classStmt.name.lexeme,
        instanceMethods,
        classMethods
    );
    environment->assign(classStmt.name, klass);
}