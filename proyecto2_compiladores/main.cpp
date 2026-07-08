#include <iostream>
#include <fstream>
#include <string>
#include "scanner.h"
#include "parser.h"
#include "ast.h"
#include "visitor.h"

using namespace std;

int main(int argc, const char* argv[]) {
    // Verificar número de argumentos
    if (argc != 2) {
        cout << "Número incorrecto de argumentos.\n";
        cout << "Uso: " << argv[0] << " <archivo_de_entrada>" << endl;
        return 1;
    }

    // Abrir archivo de entrada
    ifstream infile(argv[1]);
    if (!infile.is_open()) {
        cout << "No se pudo abrir el archivo: " << argv[1] << endl;
        return 1;
    }

    // Leer contenido completo del archivo en un string
    string input, line;
    while (getline(infile, line)) {
        input += line + '\n';
    }
    infile.close();

    // Crear instancias de Scanner
    Scanner scanner1(input.c_str());
    Scanner scanner2(input.c_str());

    // Tokens
    ejecutar_scanner(&scanner1, argv[1]);

    // Crear instancia de Parser
    Parser parser(&scanner2);

    // Parsear y generar AST
    Programa* ast = nullptr;

    try {
        ast = parser.parseProgram();
    } catch (const std::exception& e) {
        cerr << "Error al parsear: " << e.what() << endl;
        return 1;   // no tiene sentido seguir sin AST
    }

    if (ast == nullptr) {
        cerr << "Error: no se generó un AST válido." << endl;
        return 1;
    }

    // Armar nombre de archivo de salida: mismo nombre, extensión .s
    string nombreEntrada = argv[1];
    size_t pos = nombreEntrada.find_last_of('.');
    string nombreSalida = (pos == string::npos)
                            ? nombreEntrada + ".s"
                            : nombreEntrada.substr(0, pos) + ".s";

    ofstream outfile(nombreSalida);
    if (!outfile.is_open()) {
        cerr << "No se pudo crear el archivo de salida: " << nombreSalida << endl;
        return 1;
    }

    opt(ast);

    Op1Visitor opt1;
    opt1.Opt1(ast);

    Op2Visitor opt2;
    opt2.Opt2(ast);

    

    GenCodeVisitor code(outfile);

    try {
        code.gencode(ast);
    } catch (const std::exception& e) {
        cerr << "Error al generar código: " << e.what() << endl;
        return 1;
    }

    outfile.close();
    cout << "Codigo x86 generado en: " << nombreSalida << endl;

    return 0;
}
