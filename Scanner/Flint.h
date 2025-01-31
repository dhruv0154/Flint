#pragma once

#include <string>
#include <vector>

class Flint
{
    public:
        static void runFile(const std::string& path);
        static void runPrompt();
        static void run(const std::string& source);
        static void error(int line, const std::string& message);
        static void report(int line, const std::string& where, const std::string& message);
        static void main(const std::vector<std::string>& args);

    private:
        static bool hadError;
};