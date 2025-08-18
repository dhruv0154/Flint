#include "common.h"
#include "chunk.h"
#include "debug.h"

int main(int argc, char const *argv[])
{
    Chunk chunk;
    int constant = chunk.addConstant(1.2);
    chunk.writeChunk(static_cast<uint8_t>(OpCode::OP_CONSTANT), 123);
    chunk.writeChunk(constant, 123);
    chunk.writeChunk(static_cast<uint8_t>(OpCode::OP_RETURN), 123);
    Disassembler disassembler;
    disassembler.disassembleChunk(&chunk, "==test chunk==");
    return 0;
}
