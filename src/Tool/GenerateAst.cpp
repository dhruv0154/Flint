#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

void defineType(std::ofstream& writer, std::string baseName, std::string className, std::string fieldList)
{
    writer << "class " << className << " : public " << baseName << "\n{\n";
    writer << "  public:\n";

    // Constructor using intializer list because Token doesnt has a default constructor 
    writer << "    " << className << "(" << fieldList << ")";
    writer << " : ";

    std::istringstream ss(fieldList);
    std::string field;
    bool first = true;

    while (std::getline(ss, field, ',')) 
    {
        size_t space = field.find_last_of(' '); // to find the last occurance of " " just before the variable name
        std::string name = field.substr(space + 1);
        if (!first) writer << ", ";
        writer << name << "(" << name << ")";
        first = false;
    }
    writer << " {}\n\n";

    // All the member variables
    ss.clear(); // clears EOF/fail flags so we can reuse ss
    ss.seekg(0); // to send the cursor to 0 position again to reuse ss
    first = true;
    while (std::getline(ss, field, ',')) 
    {
        if(!first) field = field.substr(1);
        writer << "  " << field << ";\n";
        first = false;
    }

    writer << "};\n\n";
}

void defineAst(const std::string& outputDir, const std::string& baseName, const std::vector<std::string>& types) 
{
    std::string path = outputDir + "/" + baseName + ".h";
    std::ofstream writer(path);

    if (!writer.is_open()) 
    {
        std::cerr << "Failed to open file: " << path << std::endl;
        exit(1);
    }

    writer << "#pragma once\n";
    writer << "\n";
    writer << "#include <memory>\n";
    writer << "#include <vector>\n";
    writer << "#include <string>\n";
    writer << "#include <any>\n";
    writer << "#include \"Scanner/Token.h\"\n";
    writer << "\n";
    writer << "class " << baseName << "\n{\n";
    writer << "public:\n";
    writer << "    virtual ~" << baseName << "() = default;\n";
    writer << "};\n\n";

    // Seperating each sub className and fields of that class based on :
    for (const std::string& type : types) 
    {
        size_t colon = type.find(":");
        std::string className = type.substr(0, colon - 1); // colon - 1 to remove the space
        std::string fields = type.substr(colon + 2); // +2 to remove colon and space 
        defineType(writer, baseName, className, fields);
    }

    writer.close();
}

int main(int argc, char* argv[]) 
{
    if (argc != 2) 
    {
        std::cerr << "Usage: generate_ast <output directory>\n";
        return 64;
    }

    std::string outputDir = argv[1];
    const std::vector<std::string> types = 
    {
        "Binary : std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right",
        "Grouping : std::shared_ptr<Expr> expression",
        "Literal : std::any value",
        "Unary : Token op, std::shared_ptr<Expr> right"
    };

    defineAst(outputDir, "Expr", types);

    return 0;
}