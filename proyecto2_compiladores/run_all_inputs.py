import os
import subprocess
import shutil
import tempfile

def run_capture(cmd):
    return subprocess.run(
        cmd,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )

# Archivos c++
programa = ["main.cpp", "scanner.cpp", "token.cpp", "parser.cpp", "ast.cpp", "visitor.cpp", "Typechecker.cpp"]
ejecutable = "Proyecto2.exe"

# Compilar
compile = ["g++", "-std=c++14", "-o", ejecutable] + programa
print("Compilando:", " ".join(compile))
result = run_capture(compile)

if result.returncode != 0:
    print("Error en compilación:\n", result.stderr)
    exit(1)

print("Compilación exitosa")

# Ejecutar
input_dir = "inputs"
output_dir = "outputs"
os.makedirs(output_dir, exist_ok=True)

for i in range(1, 11):
    filename = f"input{i}.txt"
    filepath = os.path.join(input_dir, filename)

    if os.path.isfile(filepath):
        print(f"Ejecutando {filename}")
        run_cmd = [os.path.join(".", ejecutable), filepath]
        result = run_capture(run_cmd)
        asm_file = os.path.splitext(filepath)[0] + ".s"
        asm_build = None
        asm_run = None

        if result.returncode == 0 and os.path.isfile(asm_file):
            with tempfile.TemporaryDirectory(prefix=f"minizig_input{i}_") as temp_dir:
                asm_exe = os.path.join(temp_dir, f"input{i}_program.exe")
                asm_build = run_capture(["g++", asm_file, "-o", asm_exe])
                if asm_build.returncode == 0:
                    asm_run = run_capture([asm_exe])

        asm_text = ""
        if os.path.isfile(asm_file):
            with open(asm_file, "r", encoding="utf-8") as f:
                asm_text = f.read()

        stdout_text = ""
        stderr_text = ""

        if result.returncode != 0:
            stderr_text = result.stderr
        elif not os.path.isfile(asm_file):
            stderr_text = "No se genero assembly correctamente.\n"
        else:
            stderr_text = "Assembly generado correctamente.\n"

        # Guardar el codigo x86 generado en el archivo de salida solicitado
        output_file = os.path.join(output_dir, f"output{i}.txt")
        with open(output_file, "w", encoding="utf-8") as f:
            if asm_text:
                f.write(asm_text)
            else:
                f.write("No se genero assembly correctamente.\n")

        # Archivos generados
        tokens_file = os.path.join(input_dir, f"input{i}_tokens.txt")  # se crea en inputs/
        ast_file = "ast.dot"  # se crea en raíz del proyecto

        # Mover archivo de tokens si existe
        if os.path.isfile(tokens_file):
            dest_tokens = os.path.join(output_dir, f"tokens_{i}.txt")
            shutil.move(tokens_file, dest_tokens)

        # Mover y convertir AST si existe
        if os.path.isfile(ast_file):
            dest_ast = os.path.join(output_dir, f"ast_{i}.dot")
            shutil.move(ast_file, dest_ast)

            # Convertir a PNG
            output_img = os.path.join(output_dir, f"ast_{i}.png")
            dot_cmd = ["dot", "-Tpng", dest_ast, "-o", output_img]
            run_capture(dot_cmd)

    else:
        print(filename, "no encontrado en", input_dir)
