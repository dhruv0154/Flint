//  Generates an Expr AST header the way you might see in many compiler projects.
//  - Produces forward declarations, a Visitor interface, and concrete Expr
//    subclasses.
//  - Uses templates so the visitor return type can be chosen by the caller.
//  - Keeps all logic identical to the user's original version; only comments
//    have been added for clarity.
//------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <vector>

//------------------------------------------------------------------------------
//  Helper: emit forward declarations for the Visitor and each Expr subclass.
//------------------------------------------------------------------------------
void defineForwardDecls(std::ofstream& writer,
                        const std::string& baseName,
                        const std::vector<std::string>& types)
{
    writer << "// Forward declarations\n";

    // Base class and Visitor forward decls
    writer << "class " << baseName << ";\n";
    writer << "class Visitor;\n";
    
    // One forward declaration per concrete subclass (e.g., Binary, Grouping)
    for (const std::string& type : types)
    {
        // Class name precedes the ':' character in each type description
        size_t colon = type.find(':');
        std::string className = type.substr(0, colon - 1); // drop trailing space
        writer << "class " << className << ";\n";
    }

    writer << "\n";
}

//------------------------------------------------------------------------------
//  Helper: emit the full definition for a single concrete Expr subclass.
//------------------------------------------------------------------------------
void defineType(std::ofstream& writer,
                const std::string& baseName,
                const std::string& className,
                const std::string& fieldList)
{
    // Begin class declaration.
    writer << "class " << className << " : public " << baseName << "\n{\n";
    writer << "    public:\n";

    // ---------------------------------------------------------------------
    //  Constructor
    //  Uses an initializer list because Token has no default constructor.
    // ---------------------------------------------------------------------
    writer << "        " << className << "(" << fieldList << ")";
    writer << " : ";

    // Convert comma‑separated field list into an initializer list.
    std::istringstream ss(fieldList);
    std::string field;
    bool first = true;

    while (std::getline(ss, field, ','))
    {
        // Special‑case Literal so that the entire variant is stored in a
        // single member called `value` (see field list below).
        if (className == "Literal")
        {
            size_t space = fieldList.find_last_of(' ');
            std::string name = fieldList.substr(space + 1);
            writer << name << "(" << name << ")";
            break;
        }

        // For fields like "std::shared_ptr<Expr<T>> left" we want just "left".
        size_t space = field.find_last_of(' ');
        std::string name = field.substr(space + 1);
        if (!first) writer << ", ";
        writer << name << "(" << name << ")";
        first = false;
    }
    writer << " {}\n"; // end constructor

    // ---------------------------------------------------------------------
    //  Member variables
    // ---------------------------------------------------------------------
    if (className == "Literal")
    {
        // Literal stores its variant in a single field named `value`.
        writer << "        " << fieldList << ";\n";
    }
    else
    {
        // For non‑Literal classes, split all fields and emit one per line.
        ss.clear();             // reset stringstream state
        ss.seekg(0);            // rewind to beginning
        first = true;
        while (std::getline(ss, field, ','))
        {
            if (!first) field = field.substr(1); // trim leading space after comma
            writer << "        " << field << ";\n";
            first = false;
        }
    }

    writer << "    private:\n";
    writer << "        std::any acceptAny(Visitor& visitor) override { return visitor.visit" + className + baseName + "(*this); }\n";

    writer << "};\n\n"; // end class
}

void defineVisitor(std::ofstream& writer,
                   const std::string baseName,
                   const std::vector<std::string>& types)
{
    writer << "class Visitor\n{\n";

    writer << "    public:\n";

    // One pure‑virtual method per concrete subclass.
    for (const std::string& type : types)
    {
        size_t colon = type.find(':');
        std::string name = type.substr(0, colon - 1);
        writer << "        virtual std::any visit" + name + baseName + "(" + name + "& " + "e" + ") = 0;\n";
    }

    writer << "        virtual ~Visitor() = default;\n";
    writer << "};\n\n"; // end VisitorBase
}

//------------------------------------------------------------------------------
//  Top‑level helper: drive generation of an entire AST header file.
//------------------------------------------------------------------------------
void defineAst(const std::string& outputDir,
               const std::string& baseName,
               const std::vector<std::string>& types)
{
    // Compose path and open the output header.
    std::string path = outputDir + "/" + baseName + ".h";
    std::ofstream writer(path);

    if (!writer.is_open())
    {
        std::cerr << "Failed to open file: " << path << std::endl;
        exit(1);
    }

    // ---------------------------------------------------------------------
    //  Preamble & common includes
    // ---------------------------------------------------------------------
    writer << "#pragma once\n\n";
    writer << "#include <memory>\n";
    writer << "#include <vector>\n";
    writer << "#include <string>\n";
    writer << "#include <variant>\n";
    writer << "#include <any>\n";
    writer << "#include \"C:\\Flint\\include\\Scanner\\Token.h\"\n\n";

    // Forward declarations & visitor
    defineForwardDecls(writer, baseName, types);

    // ---------------------------------------------------------------------
    //  Base class definition
    // ---------------------------------------------------------------------
    writer << "class " << baseName << "\n{\n";
    writer << "    public:\n";
    writer << "        template <typename T>\n";
    writer << "        T accept(Visitor& visitor) { return std::any_cast<T>(acceptAny(visitor)); };\n";
    writer << "        virtual ~" << baseName << "() = default;\n";
    writer << "    private:\n";
    writer << "        virtual std::any acceptAny(Visitor& visitor) = 0;";
    writer << "};\n\n";

    // Visitor interface
    defineVisitor(writer, baseName, types);

    // ---------------------------------------------------------------------
    //  Concrete subclass definitions
    // ---------------------------------------------------------------------
    for (const std::string& type : types)
    {
        size_t colon = type.find(":");
        std::string className = type.substr(0, colon - 1);
        std::string fields    = type.substr(colon + 2); // skip ": "
        defineType(writer, baseName, className, fields);
    }

    writer.close();
}

//------------------------------------------------------------------------------
//  Main: expects a single argument (output directory) and generates Expr.h
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: generate_ast <output directory>\n";
        return 64;
    }

    std::string outputDir = argv[1];

    // List of concrete Expr types in "ClassName : field, field" format.
    const std::vector<std::string> types =
    {
        "Binary : std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right",
        "Grouping : std::shared_ptr<Expr> expression",
        "Literal : LiteralValue value",
        "Unary : Token op, std::shared_ptr<Expr> right"
    };

    defineAst(outputDir, "Expr", types);

    return 0;
}