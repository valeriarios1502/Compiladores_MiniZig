# Corre el compilador MiniZig (Proyecto2.exe) sobre un archivo de entrada, separa el assembly NO optimizado y OPTIMIZADO que imprime por stdout,
# y los guarda como archivos .s independientes.


import subprocess
import sys
import re


def extraer_bloque(texto, marca_inicio, marca_fin):
    patron = re.escape(marca_inicio) + r"\n(.*?)" + re.escape(marca_fin)
    match = re.search(patron, texto, re.DOTALL)
    if not match:
        return None
    return match.group(1)


def contar_instrucciones(asm_texto):
    """Cuenta líneas que parecen instrucciones reales (no etiquetas, no directivas, no vacías)."""
    contador = 0
    for linea in asm_texto.splitlines():
        l = linea.strip()
        if not l:
            continue
        if l.startswith("."):
            continue
        if l.endswith(":"):         
            continue
        if l.startswith("#") or l.startswith(";"):  
            continue
        contador += 1
    return contador


def main():
    if len(sys.argv) != 4:
        print("Uso: python extract_asm.py <ejecutable> <archivo_entrada> <nombre_base_salida>")
        sys.exit(1)

    ejecutable = sys.argv[1]
    archivo_entrada = sys.argv[2]
    nombre_base = sys.argv[3]

    result = subprocess.run(
        [ejecutable, archivo_entrada],
        capture_output=True,
        text=True
    )

    if result.returncode != 0:
        print("El compilador terminó con error:")
        print(result.stderr)
        sys.exit(1)

    salida = result.stdout

    unopt = extraer_bloque(salida, "__ASSEMBLY_UNOPTIMIZED_BEGIN__", "__ASSEMBLY_UNOPTIMIZED_END__")
    opt = extraer_bloque(salida, "__ASSEMBLY_OPTIMIZED_BEGIN__", "__ASSEMBLY_OPTIMIZED_END__")

    if unopt is None or opt is None:
        print("No se encontraron los marcadores esperados en la salida.")
        print("--- STDOUT crudo ---")
        print(salida)
        sys.exit(1)

    with open(f"{nombre_base}_unopt.s", "w") as f:
        f.write(unopt)

    with open(f"{nombre_base}_opt.s", "w") as f:
        f.write(opt)

    n_unopt = contar_instrucciones(unopt)
    n_opt = contar_instrucciones(opt)
    reduccion = 0.0
    if n_unopt > 0:
        reduccion = 100.0 * (n_unopt - n_opt) / n_unopt

    print(f"{nombre_base}: instrucciones sin optimizar = {n_unopt}, optimizadas = {n_opt}, "
          f"reducción = {reduccion:.1f}%")

    print(f"CSV,{nombre_base},{n_unopt},{n_opt},{reduccion:.1f}")


if __name__ == "__main__":
    main()
