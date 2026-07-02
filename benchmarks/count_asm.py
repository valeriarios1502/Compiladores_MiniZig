# Cuenta instrucciones "reales" en un archivo .s.


import sys


def contar_instrucciones(texto):
    contador = 0
    for linea in texto.splitlines():
        l = linea.strip()
        if not l:
            continue
        if l.startswith("."):        
            continue
        if l.endswith(":"):          
            continue
        if l.startswith("#") or l.startswith("//") or l.startswith(";"):
            continue
        contador += 1
    return contador


def main():
    if len(sys.argv) != 2:
        print("Uso: python3 count_asm.py archivo.s")
        sys.exit(1)

    with open(sys.argv[1]) as f:
        texto = f.read()

    n = contar_instrucciones(texto)
    print(f"{sys.argv[1]}: {n} instrucciones")


if __name__ == "__main__":
    main()
