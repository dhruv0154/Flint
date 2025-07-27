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
#include <stdexcept>            // Exception classes
#include <vector>               // Token container

#include "Flint/Scanner/Scanner.h"     // Lexer/tokenizer
#include "Flint/Flint.h"         // Flint runtime system
#include "Flint/Parser/Parser.h"       // AST parser
#include "Flint/Interpreter/Evaluator.h"
#include "Flint/Interpreter/Interpreter.h"
#include "Flint/Resolver/Resolver.h"

// ─────────────────────────────────────────────────────────────────────────────
// Global Interpreter State Flags
// ─────────────────────────────────────────────────────────────────────────────
// These track whether a syntax error or runtime error occurred during execution.
// This helps us exit with appropriate error codes from `main()` or halt REPL
// processing when needed.
// ─────────────────────────────────────────────────────────────────────────────
bool Flint::hadError = false;
bool Flint::hadRuntimeError = false;
const std::shared_ptr<Interpreter> Flint::interpreter = std::make_shared<Interpreter>();

// ─────────────────────────────────────────────────────────────────────────────
// Entry Point: main()
// ─────────────────────────────────────────────────────────────────────────────
// Runs Flint in either batch mode (file passed via CLI) or interactive mode.
// You can later add CLI flags for debugging, profiling, etc.
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
// Reads entire file contents into a string buffer before executing.
// Terminates with different exit codes for compile/runtime errors.
// Uses binary mode for consistent cross-platform file reading.
// ─────────────────────────────────────────────────────────────────────────────
void Flint::runFile(const std::string& path)
{
    std::ifstream file(path, std::ios_base::binary);

    if (!file)
    {
        throw std::ios_base::failure("Could not read file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Read entire file into buffer
    std::string source = buffer.str();

    run(source);

    if (hadError) exit(65);          // Syntax error
    if (hadRuntimeError) exit(70);   // Runtime error
}

// ─────────────────────────────────────────────────────────────────────────────
// Flint::runPrompt
// ─────────────────────────────────────────────────────────────────────────────
// Interactive REPL loop. Evaluates single lines and prints results immediately.
// - You might consider persisting interpreter state across lines.
// - Currently requires explicit `print` statements.
// ─────────────────────────────────────────────────────────────────────────────
void Flint::runPrompt()
{
    std::string line;

    while (true)
    {
        std::cout << "flint > ";
        if (!std::getline(std::cin, line) || line.empty())
            break;

        run(line);
        hadError = false;         // Reset between REPL runs
        hadRuntimeError = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Flint::run
// ─────────────────────────────────────────────────────────────────────────────
// Executes a string of Flint source code:
//   1. Lexing (Scanner) → Token stream
//   2. Parsing (Parser) → AST
//   3. Resolving → Variable scope resolution
//   4. Interpreting (Interpreter) → Execute program
//
// Short-circuits if a compile-time error is detected at any step.
// ─────────────────────────────────────────────────────────────────────────────
void Flint::run(const std::string& source) 
{
    auto scanner = std::make_unique<Scanner>(source);
    auto tokens  = scanner->scanTokens();
    
    auto parser  = std::make_unique<Parser>(tokens);
    auto statements = parser->parse();

    if (hadError) return; // Stop if syntax error occurred

    auto resolver = std::make_unique<Resolver>(interpreter);
    resolver->resolve(statements); // Perform static scope resolution

    if (hadError) return;

    interpreter->interpret(statements); // Finally, run the program
}

// ─────────────────────────────────────────────────────────────────────────────
// Error Reporting Utilities
// ─────────────────────────────────────────────────────────────────────────────
// These report errors at either compile-time or runtime.
// They are called by the scanner, parser, resolver, and interpreter layers.
// ─────────────────────────────────────────────────────────────────────────────

// Emit error by line number (e.g., during scanning)
void Flint::error(int line, const std::string& message)
{
    report(line, "", message);
}

// Emit error at a specific token (e.g., during parsing/resolution)
void Flint::error(Token token, const std::string& message)
{
    if (token.type == TokenType::END_OF_FILE)
        report(token.line, "at end of file", message);
    else
        report(token.line, "at '" + token.lexeme + "'", message);
}

// Log runtime error (thrown by Interpreter or built-in functions)
void Flint::runtimeError(RuntimeError error)
{
    std::cerr << "[line " << error.token.line << "] Runtime error: " << error.what() << std::endl;
    hadRuntimeError = true;
}

// Shared internal error reporting mechanism
void Flint::report(int line, const std::string& where, const std::string& message)
{
    std::cerr << "[line " << line << "] Error " << where << ": " << message << std::endl;
    hadError = true;
}