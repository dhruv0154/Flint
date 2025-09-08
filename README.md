Flint Programming Language

A lightweight, object-oriented, interpreted programming language built from scratch in modern C++.

About The Project
Flint is a personal project born from a passion for understanding the fundamentals of how programming languages work. Inspired by Bob Nystrom's incredible book, "Crafting Interpreters", this language is a complete implementation of an interpreter, built from the ground up in C++.

The ultimate vision for Flint is to serve as a high-level, easy-to-use scripting language for a custom 3D physics engine, allowing for a clean separation between the high-performance C++ core and the flexible scripting layer.

Watch the Journey on YouTube!
I've documented the entire chaotic and fun journey of building the first version of Flint in a detailed devlog. You can watch it here:

https://youtu.be/WOoQ7zPeS9s

Features
Flint is a dynamically-typed, object-oriented language. The current tree-walk interpreter supports:

[x] Variables: Global, local, and block scope.

[x] Data Types: Numbers, Strings, Booleans, Nil.

[x] Control Flow: if/else statements, and/or logical operators.

[x] Loops: for and while loops.

[x] Functions: First-class functions, closures, and recursion.

[x] Built-in Functions: Core functions for basic operations.

[x] Error Handling: Robust runtime error reporting.

[x] classes, static functions, first class functions, arrays, strings, ternary operators... many more

Getting Started
To get a local copy up and running, follow these simple steps.

Prerequisites
You will need a C++17 (or later) compatible compiler and CMake.

g++ / clang

cmake >= 3.10

Build & Run
Clone the repo

Bash

git clone https://github.com/dhruv0154/Flint.git
Create a build directory

Bash

cd flint
mkdir build && cd build
Run CMake and build the project

Bash

cmake ..
make
Run the Flint interpreter!

Bash

./flint            # To run the REPL
./flint myscript.flint # To run a .flint file
Project Status & Roadmap
Flint is an actively developed project.

Current Stage: Fully functional Tree-Walk Interpreter. This version is stable and implements the full feature set described above.

Next Steps: I am currently halfway through implementing a Bytecode Virtual Machine (VM). This will replace the tree-walk interpreter for a massive performance boost. Early tests show the VM is already significantly faster than CPython in recursive Fibonacci benchmarks.

Future plans include:

A more comprehensive standard library.

Object-oriented features (classes).

Integration into the planned 3D Physics Engine.
