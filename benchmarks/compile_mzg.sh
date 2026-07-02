# Corre el pipeline completo: Proyecto2.exe -> extrae assembly optimizado -> ensambla con gcc..

set -e

ENTRADA="$1"
SALIDA="$2"
EJECUTABLE="./Proyecto2.exe"
TMP_BASE="/tmp/mzg_build_$$"

if [ -z "$ENTRADA" ] || [ -z "$SALIDA" ]; then
    echo "Uso: $0 <archivo_entrada.txt> <nombre_binario_salida>"
    exit 1
fi

python3 extract_asm.py "$EJECUTABLE" "$ENTRADA" "$TMP_BASE" > /dev/null

gcc "${TMP_BASE}_opt.s" -o "$SALIDA"

rm -f "${TMP_BASE}_unopt.s" "${TMP_BASE}_opt.s"
