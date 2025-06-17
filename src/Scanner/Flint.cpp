#include "Scanner\Flint.h"
#include "Scanner\Flint.h"
#include "Scanner\Scanner.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

bool Flint::hadError = false;

int main(int argc, char const *argv[])
{
    Flint::runFile("C:\\Flint\\test.txt");
    return 0;
}

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

    if(hadError)
    {
        exit(65);
    }
}

void Flint::runPrompt()
{
    std::string line;

    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, line);

        if (line.empty() && std::cin.eof()) break;
        run(line);
        hadError = false;
    }
}

void Flint::run(const std::string& source)
{
    Scanner* scanner = new Scanner(source);
    std::vector<Token> tokens = scanner -> scanTokens(); 

    for (const Token& token : tokens)
    {
        std::cout << token.toString() << std::endl; 
    }
}

void Flint::error(int line, const std::string& message)
{
    report(line, "", message);
}

void Flint::report(int line, const std::string& where, const std::string& message)
{
    std::cerr << "[line " << line << "] Error" << where << ": " << message << std::endl;
    hadError = true;
}

void Flint::main(const std::vector<std::string>& args)
{
    if (args.size() > 1)
    {
        std::cout << "Usage: dflint [script]" << std::endl;
        exit(64);
    }
    else if (args.size() == 1)
    {
        runFile(args.at(0));
    }
    else
    {
        runPrompt();
    }
}