// ─────────────────────────────────────────────────────────────────────────────
// Entry point and core runtime for the Flint language interpreter.
//
// This file provides:
//   • A command‑line entry point (`main`) to run a Flint script file.
//   • A REPL (`runPrompt`) for interactive line-by-line input.
//   • Script file loading and in-memory buffering.
//   • High-level integration between the scanner, parser, and interpreter.
//   • Static error tracking and runtime diagnostics.
// ─────────────────────────────────────────────────────────────────────────────

#include <iostream>              // Standard input/output
#include <fstream>               // File stream handling
#include <sstream>               // String stream utilities
#include <stdexcept>             // Exception classes
#include <vector>                // Token container

#include "Scanner/Scanner.h"     // Lexer/tokenizer
#include "Flint/Flint.h"         // Flint runtime system
#include "Parser/Parser.h"       // AST parser
#include "Interpreter/Evaluator.h"
#include "Interpreter/Interpreter.h"
// #include "AstPrinter.h"
// #include "RpnPrinter.h"

// ─────────────────────────────────────────────────────────────────────────────
// Global Interpreter State Flags
// ─────────────────────────────────────────────────────────────────────────────
// These track whether a syntax error or runtime error occurred during execution.
// This helps us exit with appropriate error codes from `main()` or halt REPL
// processing when needed.
// ─────────────────────────────────────────────────────────────────────────────
bool Flint::hadError = false;
bool Flint::hadRuntimeError = false;
const std::unique_ptr<Interpreter> Flint::interpreter = std::make_unique<Interpreter>();

// ─────────────────────────────────────────────────────────────────────────────
// Entry Point: main()
// ─────────────────────────────────────────────────────────────────────────────
// Currently hardcoded to run `test.txt`. Can later accept `argc/argv` to
// support file-based execution from the command line.
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char const *argv[])
{
    if (argc > 1)
    {
        Flint::runFile(argv[1]);
    }
    else
    {
        Flint::runPrompt();
    }
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Flint::runFile
// ─────────────────────────────────────────────────────────────────────────────
// Reads a script file into a string buffer, then invokes the scanner/parser/
// interpreter pipeline. Terminates with distinct status codes on error:
//   • 65 = compile-time (syntax) error
//   • 70 = runtime error
// ─────────────────────────────────────────────────────────────────────────────
void Flint::runFile(const std::string& path)
{
    std::ifstream file(path, std::ios_base::binary);

    if (!file)
    {
        throw std::ios_base::failure("Could not read file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    run(source);

    if (hadError) exit(65);
    if (hadRuntimeError) exit(70);
}

// ─────────────────────────────────────────────────────────────────────────────
// Flint::runPrompt
// ─────────────────────────────────────────────────────────────────────────────
// Launches a simple REPL (Read-Eval-Print Loop). Terminates on EOF.
// After each line, resets the `hadError` flag so the REPL doesn't exit on error.
// ─────────────────────────────────────────────────────────────────────────────
// TODO: make expression print output without using print explicitly
void Flint::runPrompt()
{
    std::string line;

    while (true)
    {
        std::cout << "flint > ";
        if (!std::getline(std::cin, line) || line.empty())
            break;

        run(line);
        hadError = false;
        hadRuntimeError = false;
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// Flint::run
// ─────────────────────────────────────────────────────────────────────────────
// The main pipeline that takes raw source code and processes it:
//   1. Tokenize using the Scanner
//   2. Parse into AST using the Parser
//   3. Interpret the parsed statements using the Interpreter
// Skips execution if a parse error occurred (early return).
// ─────────────────────────────────────────────────────────────────────────────
void Flint::run(const std::string& source) 
{
    auto scanner = std::make_unique<Scanner>(source);
    auto tokens  = scanner->scanTokens();
    auto parser  = std::make_unique<Parser>(tokens);

    auto statements = parser->parse();

    // Filter out invalid (nullptr) statements
    std::vector<std::shared_ptr<Statement>> validStatements;
    for (auto& stmt : statements) {
        if (stmt) validStatements.push_back(stmt);
    }

    if (!validStatements.empty())
        interpreter->interpret(validStatements);
}

// ─────────────────────────────────────────────────────────────────────────────
// Error Reporting Utilities
// ─────────────────────────────────────────────────────────────────────────────
// These are called from various components (e.g., scanner, parser, runtime)
// to signal different types of failures. Each sets flags that control
// termination behavior or REPL continuation.
// ─────────────────────────────────────────────────────────────────────────────

// Compile-time error (by line number)
void Flint::error(int line, const std::string& message)
{
    report(line, "", message);
}

// Compile-time error (with token context)
void Flint::error(Token token, const std::string& message)
{
    if (token.type == TokenType::END_OF_FILE)
        report(token.line, "at end of file", message);
    else
        report(token.line, "at '" + token.lexeme + "'", message);
}

// Runtime error (with thrown RuntimeError)
void Flint::runtimeError(RuntimeError error)
{
    std::cerr << "[line " << error.token.line << "] Runtime error: " << error.what() << std::endl;
    hadRuntimeError = true;
}

// Shared error reporter (used internally)
void Flint::report(int line, const std::string& where, const std::string& message)
{
    std::cerr << "[line " << line << "] Error " << where << ": " << message << std::endl;
    hadError = true;
}