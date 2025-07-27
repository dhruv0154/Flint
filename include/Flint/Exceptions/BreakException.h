#pragma once

#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
//  BreakException – Control Flow Signal for Loop Break
// ─────────────────────────────────────────────────────────────────────────────
//  A custom exception type used internally by the Interpreter to implement
//  `break` statements in loops. Throwing this exception unwinds execution
//  from the current loop body back to the loop handler.
//
//  Notes:
//    - In C++ terms, this is an unchecked exception (derives from std::runtime_error).
//    - The exception message "Break" is not user-visible; it simply signals control flow.
//    - This mechanism enables clean separation of loop logic without manual flags.
// ─────────────────────────────────────────────────────────────────────────────

class BreakException : public std::runtime_error {
public:
    // Constructor: initializes base runtime_error with a sentinel message
    BreakException()
        : std::runtime_error("Break")
    {}
};
