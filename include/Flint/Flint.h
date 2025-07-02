#pragma once  // Ensures this header is only included once during compilation

#include <string>
#include <vector>
#include "RuntimeError.h"
#include "Interpreter/Interpreter.h"

// Provides core control functions for running source code and handling errors
class Flint
{
public:
    // Runs a source file from the given file path
    static void runFile(const std::string& path);

    // Starts an interactive prompt (REPL-style input)
    static void runPrompt();

    // Runs a single source string (used by both file and prompt)
    static void run(const std::string& source);

    // Reports an error on a specific line
    static void error(int line, const std::string& message);

    // Reports an error on a specific token
    static void error(Token token, const std::string& message);

    // Reports a runtime error
    static void runtimeError(RuntimeError error);

    // Outputs a detailed error message with line number and context
    static void report(int line, const std::string& where, const std::string& message);

    // Entry point wrapper that interprets command-line arguments
    static void main(const std::vector<std::string>& args);

private:
    // Flag indicating whether any error has occurred
    static const std::unique_ptr<Interpreter> interpreter;
    static bool hadError;
    static bool hadRuntimeError;
};