#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <vector>

void defineForwardDecls(std::ofstream& writer, const std::string& baseName, const std::vector<std::string>& types)
{
    writer << "// Forward declarations\n";
    writer << "template <typename T> class " << baseName << ";\n";
    writer << "template <typename T> class Visitor;\n";

    for (const std::string& type : types)
    {
        size_t colon = type.find(':');
        std::string className = type.substr(0, colon - 1); // extract class name before ':'
        writer << "template <typename T> class " << className << ";\n";
    }

    writer << "\n";
}

void defineType(std::ofstream& writer, const std::string& baseName, const std::string& className, const std::string& fieldList)
{
    writer << "template <typename T>\n";
    writer << "class " << className << " : public " << baseName << "<T>" << "\n{\n";
    writer << "    public:\n";

    // Constructor using intializer list because Token doesnt has a default constructor 
    writer << "        " << className << "(" << fieldList << ")";
    writer << " : ";

    std::istringstream ss(fieldList);
    std::string field;
    bool first = true;

    while (std::getline(ss, field, ',')) 
    {
        if(className == "Literal")
        {
            size_t space= fieldList.find_last_of(' ');
            std::string name = fieldList.substr(space + 1);
            writer << name << "(" << name << ")";
            break;
        }
        size_t space = field.find_last_of(' '); // to find the last occurance of " " just before the variable name
        std::string name = field.substr(space + 1);
        if (!first) writer << ", ";
        writer << name << "(" << name << ")";
        first = false;
    }
    writer << " {}\n";

    writer << "        T accept(Visitor<T>& visitor) override {";
    writer << " return visitor.visit" + className + baseName + "(*this); }\n\n";

    if(className == "Literal")
    {
        writer << "        " << fieldList << ";\n";
        writer << "};\n\n";
        return;
    }

    // All the member variables
    ss.clear(); // clears EOF/fail flags so we can reuse ss
    ss.seekg(0); // to send the cursor to 0 position again to reuse ss
    first = true;

    while (std::getline(ss, field, ',')) 
    {
        if(!first) field = field.substr(1);
        writer << "        " << field << ";\n";
        first = false;
    }

    writer << "};\n\n";
}

void defineVisitor(std::ofstream& writer, const std::string baseName, const std::vector<std::string>& types)
{
    writer << "template <typename T> \nclass Visitor \n{\n";

    auto lower = [](unsigned char c) { return std::tolower(c); };
    std::string lowerCaseBaseName = baseName;
    std::transform(lowerCaseBaseName.begin(), lowerCaseBaseName.end(), lowerCaseBaseName.begin(), lower);
    writer << "    public:\n";

    for(const std::string& type : types)
    {
        size_t colon = type.find(':');
        std::string name = type.substr(0, colon - 1); // colon - 1 to remove the space
        
        writer << "        virtual T visit" + name + baseName + "(" + name + "<T>&" + " " + lowerCaseBaseName + ") = 0;\n";
    }

    writer << "        virtual ~Visitor() = default;\n";

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
    writer << "#include <variant>\n";
    writer << "#include \"C:\\Flint\\include\\Scanner\\Token.h\"\n";
    writer << "\n";
    defineForwardDecls(writer, baseName, types);

    writer << "template <typename T> \n";
    writer << "class " << baseName << "\n{\n";
    writer << "    public:\n";
    writer << "        virtual T accept(Visitor<T>& visitor) = 0;\n";
    writer << "        virtual ~" << baseName << "() = default;\n";
    writer << "};\n\n";

    defineVisitor(writer, baseName, types);

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
        "Binary : std::shared_ptr<Expr<T>> left, Token op, std::shared_ptr<Expr<T>> right",
        "Grouping : std::shared_ptr<Expr<T>> expression",
        "Literal : std::variant<std::monostate,int,double,std::string> value",
        "Unary : Token op, std::shared_ptr<Expr<T>> right"
    };

    defineAst(outputDir, "Expr", types);

    return 0;
}