#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, char const *argv[])
{
    VM vm;
    Chunk chunk;
    chunk.writeConstant(1.2, 123);
    chunk.writeChunk(static_cast<uint8_t>(OpCode::OP_RETURN), 123);

    Disassembler disassembler;
    disassembler.disassembleChunk(&chunk, "==test chunk==");
    return 0;
}
