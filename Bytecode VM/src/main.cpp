#include <string>
#include <sstream>
#include <fstream>
#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

VM vm;

int main(int argc, char const *argv[])
{
    if(argc == 1){
        repl();
    }else if(argc == 2){
        runFile(argv[1]);
    }
    else{
        std::cerr << "Usage: cflint [paht]\n";
        exit(64);
    }
    return 0;
}

static void repl()
{
    std::string line;
    while(true){
        std::cout << "> ";
        if(std::getline(std::cin, line)){
            std::cout << "\n";
            break;
        }
        vm.interpret(line);
    }
}

static void runFile(const std::string &path)
{
    std::string src = readFile(path);
    InterpretResult result = vm.interpret(src);


    if(result == InterpretResult::INTERPRET_COMPILE_ERROR) exit(65);
    if(result == InterpretResult::INTERPRET_RUNTIME_ERROR) exit(70);
}

static std::string readFile(const std::string &path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open source file: " << path << "\n";
        exit(74);
    }
    std::stringstream buffer;
    buffer << file.rdbuf(); // Read entire file into buffer
    std::string source = buffer.str();
    return source;
}