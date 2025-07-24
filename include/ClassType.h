#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  ClassType.h – Enumeration of Class Contexts in Flint
// ─────────────────────────────────────────────────────────────────────────────
//  Defines the context of class-related parsing and resolution.
//
//  Used by the Resolver and Interpreter to enforce rules and
//  differentiate behavior when inside or outside classes.
//
//  Values:
//    - NONE  : Not currently within any class declaration.
//    - CLASS : Within a standard class declaration (allows `this`, methods).
//
//  This enum helps guard against invalid `this` or `super` usage
//  and supports class vs. instance method resolution.
// ─────────────────────────────────────────────────────────────────────────────

enum class ClassType {
    NONE,   // Default: no class context active
    CLASS   // Inside a class declaration context
};