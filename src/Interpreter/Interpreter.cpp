#include <chrono>
#include "Flint/Interpreter/Interpreter.h"
#include "Flint/Flint.h"
#include "Flint/Parser/Value.h"
#include "Flint/ASTNodes/ExpressionNode.h"
#include "Flint/Exceptions/BreakException.h"
#include "Flint/Exceptions/ContinueException.h"
#include "Flint/Exceptions/RuntimeError.h"
#include "Flint/Callables/FlintCallable.h"
#include "Flint/Callables/Functions/NativeFunction.h"
#include "Flint/Callables/Functions/FlintFunction.h"
#include "Flint/Exceptions/ReturnException.h"
#include "Flint/Callables/Classes/FlintClass.h"
#include "Flint/Callables/Classes/FlintInstance.h"
#include "Flint/FlintArray.h"

// ─────────────────────────────────────────────────────────────────────────────
// Global Interpreter State
// The Evaluator handles expression evaluation (Binary, Literal, etc.).
// The Environment stores runtime variables.
// ─────────────────────────────────────────────────────────────────────────────
Interpreter::Interpreter()
{
    globals = std::make_shared<Environment>();
    environment = globals;
    evaluator = std::make_unique<Evaluator>(*this);

    // Define clock()
    globals -> define("clock", std::make_shared<NativeFunction>(
    0,
    [](const std::vector<LiteralValue>& args, const Token& paren) -> LiteralValue {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()).count();
        return static_cast<double>(ms) / 1000.0;
    },
    "clock"
    ));

    globals -> define("scan", std::make_shared<NativeFunction>(
        -1,
        [this](const std::vector<LiteralValue>& args, 
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
            return this -> isNumber(line) ? 
                LiteralValue(std::stod(line)) : LiteralValue(line);
        },
        "scan"
    ));

    globals->define("print", std::make_shared<NativeFunction>(
    -1, // -1 means variadic
    [](const std::vector<LiteralValue>& args, const Token& paren) -> LiteralValue {
        for (const auto& arg : args)
            std::cout << Interpreter::stringify(arg);
        return nullptr;
    },
    "print"
    ));

    globals->define("intDiv", std::make_shared<NativeFunction>(
    2,
    [](const std::vector<LiteralValue>& args, const Token& paren) -> LiteralValue {
        if (!std::holds_alternative<double>(args[0]) 
        || !std::holds_alternative<double>(args[1])) {
            throw RuntimeError(paren, "intDiv() expects two numbers.");
        }

        int a = static_cast<int>(std::get<double>(args[0]));
        int b = static_cast<int>(std::get<double>(args[1]));

        if (b == 0) throw RuntimeError(paren, "Division by zero.");

        return static_cast<double>(a / b); 
    },
    "intDiv"
    ));

    globals -> define("toString", std::make_shared<NativeFunction>(
        1,
        [this](const std::vector<LiteralValue>& args, const Token& paren) -> LiteralValue {
            if(args.size() > 1) 
                throw RuntimeError(paren, "toString() takes at most 1 argument.");
            
            if(!std::holds_alternative<double>(args[0]))
            {
                throw RuntimeError(paren, "toString() takes a number as an argument.");
            }

            return stringify(args[0]);
        },
        "toString"
    ));


    globals->define("ord", std::make_shared<NativeFunction>(
    1,
    [](const std::vector<LiteralValue>& args, const Token& paren) -> LiteralValue {
        if (!std::holds_alternative<std::string>(args[0])) {
            throw RuntimeError(paren, "ord() expects a string argument.");
        }

        const std::string& str = std::get<std::string>(args[0]);
        if (str.length() != 1) {
            throw RuntimeError(paren, "ord() expects a single character string.");
        }

        return static_cast<double>(static_cast<unsigned char>(str[0]));
    },
    "ord"
    ));

    globals->define("chr", std::make_shared<NativeFunction>(
    1,
    [](const std::vector<LiteralValue>& args, const Token& paren) -> LiteralValue {
        if (!std::holds_alternative<double>(args[0])) {
            throw RuntimeError(paren, "chr() expects a number.");
        }

        int code = static_cast<int>(std::get<double>(args[0]));
        if (code < 0 || code > 255) {
            throw RuntimeError(paren, "chr() argument must be in range 0–255.");
        }

        return std::string(1, static_cast<char>(code));
    },
    "chr"
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
// overloaded operator() based on which statement variant (Print, Let, etc.)
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
        throw;
    }

    environment = previous;
}

void Interpreter::resolve(ExprPtr expr, int depth)
{
    locals[expr] = depth;
}

// ─────────────────────────────────────────────────────────────────────────────
// stringify()
// Converts any LiteralValue into a human-readable string for output.
// This is used by print statements.
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
        else if constexpr (std::is_same_v<T, std::shared_ptr<FlintClass>>) {
            return val -> toString();  // call custom string method on class
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<FlintInstance>>) {
            return val -> toString();  // call custom string method on class
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<FlintArray>>) {
            return val -> toString();
        }
        else {
            return "<unknown>";
        }
    };

    return std::visit(func, obj);
}

bool Interpreter::isNumber(const std::string& str) 
{
    char* end = nullptr;
    std::strtod(str.c_str(), &end);
    return end != str.c_str() && *end == '\0';
}


// ─────────────────────────────────────────────────────────────────────────────
// Statement Visitor Implementations
// These overloads are invoked via std::visit inside execute().
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

void Interpreter::operator()(const FunctionStmt &stmt) const
{
    std::shared_ptr<FunctionStmt> statmentPtr = std::make_shared<FunctionStmt>(stmt);
    std::shared_ptr<FlintFunction> function = 
        std::make_shared<FlintFunction>(statmentPtr, environment, false);
    environment -> define(stmt.name -> lexeme, function);
}

void Interpreter::operator()(const ReturnStmt &stmt) const
{
    LiteralValue val = nullptr;
    if(stmt.val) val = evaluator -> evaluate(stmt.val);
     
    throw ReturnException(val);
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
    catch(const ContinueException& e) {}
}

// Expression statement: evaluates expression and discards result (side effects only)
void Interpreter::operator()(const ExpressionStmt& exprStatement) const
{
    evaluator->evaluate(exprStatement.expression);
}

// LET statement: evaluates right-hand expression and stores it in the environment
void Interpreter::operator()(const LetStmt& letStatement) const
{
    for (const auto &[name, initializer] : letStatement.declarations)
    {
        LiteralValue value = nullptr;

        if (initializer != nullptr)
        {
            value = evaluator -> evaluate(initializer);  // evaluate expression
        }

        environment->define(name.lexeme, value);
    }
}

void Interpreter::operator()(const BlockStmt& blockStatement) const
{
    auto env = std::make_shared<Environment>(environment);
    executeBlock(blockStatement.statements, env);
}

void Interpreter::operator()(const ClassStmt& classStmt) const
{
    LiteralValue superClass = nullptr;
    std::shared_ptr<FlintClass> convertedClass = nullptr;

    if(classStmt.superClass)
    {
        superClass = evaluator -> evaluate(classStmt.superClass);
        if(!std::holds_alternative<std::shared_ptr<FlintCallable>>(superClass))
        {
            throw RuntimeError(classStmt.name, "Superclass must be a class.");
        }
        else
        {
            convertedClass = std::dynamic_pointer_cast<FlintClass>(
                std::get<std::shared_ptr<FlintCallable>>(superClass));
            if (!convertedClass) 
                throw RuntimeError(classStmt.name, "Superclass must be a class.");
        }
    }
    environment -> define(classStmt.name.lexeme, nullptr);

    if(convertedClass) {
        environment =  std::make_shared<Environment>(environment);
        environment -> define("super", convertedClass);
    }
    std::unordered_map<std::string, std::shared_ptr<FlintFunction>> classMethods;
    std::unordered_map<std::string, std::shared_ptr<FlintFunction>> instanceMethods;
    for(auto method : classStmt.classMethods)
    {
        auto methodPtr = std::make_shared<FunctionStmt>
            (std::get<FunctionStmt>(*method));
        auto function = std::make_shared<FlintFunction>
            (methodPtr, environment, methodPtr -> name -> lexeme == "init");
        classMethods[methodPtr -> name -> lexeme] = function;
    }
    for(auto method : classStmt.instanceMethods)
    {
        auto methodPtr = std::make_shared<FunctionStmt>
            (std::get<FunctionStmt>(*method));
        auto function = std::make_shared<FlintFunction>
            (methodPtr, environment, methodPtr -> name -> lexeme == "init");
        instanceMethods[methodPtr -> name -> lexeme] = function;
    }
    std::shared_ptr<FlintCallable> klass = std::make_shared<FlintClass>
        (classStmt.name.lexeme, instanceMethods, classMethods, convertedClass);

    if (convertedClass) environment = environment -> enclosing;
    environment -> assign(classStmt.name, klass);
}