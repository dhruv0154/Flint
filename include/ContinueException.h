#pragma once

#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
//  ContinueException – Control Flow Signal for Loop Continue
// ─────────────────────────────────────────────────────────────────────────────
//  A custom exception type used internally by the Interpreter to implement
//  `continue` statements within loops.  Throwing this exception unwinds
//  the current iteration and signals the loop handler to proceed with
//  the next iteration.
//
//  Notes:
//    - Inherits from std::runtime_error as an unchecked exception.
//    - The exception message "Continue" is not displayed to users;
//      it serves solely as an internal control flow marker.
//    - This approach simplifies loop handling by avoiding manual flags.
// ─────────────────────────────────────────────────────────────────────────────

class ContinueException : public std::runtime_error {
public:
    // Constructor: initializes base runtime_error with a sentinel message
    ContinueException()
        : std::runtime_error("Continue")
    {}
};