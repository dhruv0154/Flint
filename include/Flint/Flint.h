#pragma once  // Ensures this header is only included once during compilation

#include <string>
#include <vector>
#include "RuntimeError.h"
#include "Interpreter/Interpreter.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Flint Class — Core Entry Point for Running Code
// ─────────────────────────────────────────────────────────────────────────────
//  This class provides the high-level interface to run Flint programs.
//  It supports running files, entering interactive prompt (REPL),
//  and reporting errors (lexical, syntax, and runtime).
//
//  Acts as the "driver" of the interpreter — coordinates parsing, execution,
//  and error handling, and is the first layer called from main() or tests.
// ─────────────────────────────────────────────────────────────────────────────

class Flint
{
public:
    // ───────────────────────────────────────────────────────────────
    // runFile(path):
    // Executes a Flint script from a file path.
    // This is typically used when the interpreter is run via CLI.
    // ───────────────────────────────────────────────────────────────
    static void runFile(const std::string& path);

    // ───────────────────────────────────────────────────────────────
    // runPrompt():
    // Launches a REPL (Read-Eval-Print Loop) for interactive execution.
    // Used when no script is provided — enables live code testing.
    // ───────────────────────────────────────────────────────────────
    static void runPrompt();

    // ───────────────────────────────────────────────────────────────
    // run(source):
    // Core function that processes a single source code string:
    // - Tokenizes
    // - Parses
    // - Interprets
    //
    // Called by both `runFile()` and `runPrompt()`.
    // ───────────────────────────────────────────────────────────────
    static void run(const std::string& source);

    // ───────────────────────────────────────────────────────────────
    // error(line, message):
    // Reports a compile-time error at a specific line in the source.
    // Used by Scanner/Parser to notify invalid syntax.
    // ───────────────────────────────────────────────────────────────
    static void error(int line, const std::string& message);

    // ───────────────────────────────────────────────────────────────
    // error(token, message):
    // Reports an error tied to a specific token — used for clearer context.
    // If the token is EOF, reports it as "at end".
    // ───────────────────────────────────────────────────────────────
    static void error(Token token, const std::string& message);

    // ───────────────────────────────────────────────────────────────
    // runtimeError(error):
    // Handles interpreter-time errors like division by zero,
    // undefined variable access, or invalid operations.
    // ───────────────────────────────────────────────────────────────
    static void runtimeError(RuntimeError error);

    // ───────────────────────────────────────────────────────────────
    // report(line, where, message):
    // Formats and prints a user-facing error message.
    // Internal helper for the above `error()` functions.
    // ───────────────────────────────────────────────────────────────
    static void report(int line, const std::string& where, const std::string& message);

    // ───────────────────────────────────────────────────────────────
    // main(args):
    // Acts as a wrapper to process command-line arguments passed to
    // Flint interpreter and call appropriate functions (runFile or runPrompt).
    // ───────────────────────────────────────────────────────────────
    static void main(const std::vector<std::string>& args);

private:
    // ───────────────────────────────────────────────────────────────
    // Global interpreter instance, used to evaluate parsed ASTs.
    // Shared across runFile and runPrompt.
    // ───────────────────────────────────────────────────────────────
    static const std::shared_ptr<Interpreter> interpreter;

    // Tracks whether a syntax or lexical error has occurred.
    static bool hadError;

    // Tracks whether a runtime error occurred during interpretation.
    static bool hadRuntimeError;
};