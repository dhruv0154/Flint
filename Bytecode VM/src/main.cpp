#include "common.h"
#include "chunk.h"
#include "debug.h"

int main(int argc, char const *argv[])
{
    Chunk chunk;
    chunk.writeChunk(static_cast<uint8_t>(OpCode::OP_RETURN));
    Disassembler disassembler;
    disassembler.disassembleChunk(&chunk, "test chunk");
    return 0;
}
