// ------------------------------------------------------------
// Entry point and core runtime for the Flint language interpreter.
//
// This file provides:
//   • A command‑line entry point (global `main`) that runs a script file.
//   • A REPL (`runPrompt`) for interactive experimentation.
//   • Helpers to load a script into memory and pass it to the Scanner.
//   • Error‑handling utilities that track whether a scan or parse error
//     occurred during execution.
//   • A thin façade over `Scanner` that tokenizes the source and prints
//     the resulting tokens for debugging purposes.
//

#include <iostream>              // Standard I/O streams
#include <fstream>               // File‑stream utilities
#include <sstream>               // String‑stream buffer helpers
#include <stdexcept>             // Standard exception types
#include <vector>                // `std::vector` container for token list
#include "Scanner\Scanner.h"     // Tokenizer for the Flint language
#include "Flint\Flint.h"       // Declarations for the Flint class
#include "Parser\Parser.h"
// #include "AstPrinter.h"
// #include "RpnPrinter.h"
#include "Evaluator.h"

// ---------------------------------------------------------------------------
// Static error flag: becomes true if any error is reported during execution.
// Externally visible so other translation units (if any) can read it.
// ---------------------------------------------------------------------------
bool Flint::hadError = false;
bool Flint::hadRuntimeError = false;
const std::unique_ptr<Interpreter> Flint::interpreter = std::make_unique<Interpreter>();

/* ===========================================================================
 *  Program Entry Point (global `main`)
 * ---------------------------------------------------------------------------
 *  For now we simply hard‑code a test file path and execute it. A future
 *  enhancement could parse `argc`/`argv` and forward arguments into
 *  `Flint::main` below so that users can run any script from the command line.
 * ===========================================================================*/
int main(int argc, char const *argv[])
{
    Flint::runFile("C:\\Flint\\test.txt");
    return 0;
}

/* ===========================================================================
 *  Flint::runFile
 * ---------------------------------------------------------------------------
 *  Reads an entire script file into a string buffer, then forwards the source
 *  to `run` for tokenization (and eventually parsing/evaluation). If any
 *  error is flagged during processing, we exit with status code 65
 * ===========================================================================*/
void Flint::runFile(const std::string& path)
{
    std::ifstream file(path, std::ios_base::binary);

    if (!file)
    {
        // Using `std::ios_base::failure` lets callers distinguish I/O errors.
        throw std::ios_base::failure("Could not read file: " + path);
    }

    // Slurp entire file into memory using a stringstream.
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // Tokenize (and later: parse/interpret) the program.
    run(source);

    // If an error occurred, terminate with a distinct code so scripts/bash
    // can detect interpreter failure.
    if(hadError)
    {
        exit(65);
    }
    if(hadRuntimeError)
    {
        exit(70);
    }
}

/* ===========================================================================
 *  Flint::runPrompt
 * ---------------------------------------------------------------------------
 *  Implements a minimal REPL (Read‑Eval‑Print Loop). The loop terminates when
 *  the user sends an EOF (Ctrl‑D on *nix, Ctrl‑Z then Enter on Windows).
 * ===========================================================================*/
void Flint::runPrompt()
{
    std::string line;

    while (true)
    {
        std::cout << "> ";                // Prompt
        std::getline(std::cin, line);

        // Break when the user enters EOF (empty line + eofbit set)
        if (line.empty() && std::cin.eof()) break;

        run(line);                         // Process the single line
        hadError = false;                  // Allow next line even if this one erred
    }
}

/* ===========================================================================
 *  Flint::run
 * ---------------------------------------------------------------------------
 *  The core driver that turns raw source text into a vector of `Token`s by
 *  delegating to the `Scanner` class. For now we simply dump each token’s
 *  string representation to stdout so we can verify that scanning works as
 *  expected.
 * ===========================================================================*/
void Flint::run(const std::string& source) 
{
    auto scanner = std::make_unique<Scanner>(source);
    auto tokens  = scanner->scanTokens();
    auto parser  = std::make_unique<Parser>(tokens);


    if(hadError) return;

    auto expr = parser -> parse();
    interpreter -> interpret(expr);
}

/* ===========================================================================
 *  Error Handling Helpers
 * ---------------------------------------------------------------------------
 *  These functions report errors and toggle the `hadError` flag so that the
 *  interpreter can decide whether to abort at the appropriate time.
 * ===========================================================================*/
void Flint::error(int line, const std::string& message)
{
    report(line, "", message);
}

void Flint::error(Token token, const std::string& message)
{
    if(token.type == TokenType::END_OF_FILE) report(token.line, "At the end of file", message);
    else report(token.line, "at '" + token.lexeme + "'", message);
}

void Flint::runtimeError(RuntimeError error)
{
    std::cerr << "[line " << error.token.line << "] : " << error.what() << std::endl;
    hadRuntimeError = true;
}

void Flint::report(int line, const std::string& where, const std::string& message)
{
    std::cerr << "[line " << line << "] Error " << where << ": " << message << std::endl;
    hadError = true;
}