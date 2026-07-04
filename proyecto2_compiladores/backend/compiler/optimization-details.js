const OPTIMIZATION_LABELS = {
  constantFolding: "Constant Folding",
  cascada: "Cascada",
  sethiUllman: "Sethi-Ullman",
  peephole: "Peephole",
}

function hasConstantExpression(sourceCode) {
  return /(?:const|var)\s+\w+(?:\s*:[^=]+)?=\s*(?:\d+|true|false)\s*[+\-*/%&|]|=\s*\([^)]*[+\-*/%][^)]*\)/.test(sourceCode)
}

function hasPropagationCandidate(sourceCode) {
  return /const\s+\w+/.test(sourceCode) || /var\s+\w+[^;]*=\s*(?:\d+|true|false)/.test(sourceCode)
}

function hasRegisterPressureCandidate(sourceCode) {
  return /[+\-*/%].*[+\-*/%]/s.test(sourceCode) || /\w+\s*\([^)]*,[^)]*\)/.test(sourceCode)
}

function normalizeAssembly(value) {
  return String(value || "").replace(/\r\n/g, "\n").trim()
}

function buildOptimizationDetails({ sourceCode, success, assembly, optimizedAssembly }) {
  const before = normalizeAssembly(assembly)
  const after = normalizeAssembly(optimizedAssembly)
  const peepholeApplied = success && before.length > 0 && after.length > 0 && before !== after

  const details = [
    {
      key: "constantFolding",
      name: OPTIMIZATION_LABELS.constantFolding,
      applied: Boolean(success && hasConstantExpression(sourceCode)),
      evidence: "Se detectaron expresiones constantes en el codigo fuente.",
    },
    {
      key: "cascada",
      name: OPTIMIZATION_LABELS.cascada,
      applied: Boolean(success && hasPropagationCandidate(sourceCode)),
      evidence: "Se detectaron constantes o variables inicializadas que pueden propagarse.",
    },
    {
      key: "sethiUllman",
      name: OPTIMIZATION_LABELS.sethiUllman,
      applied: Boolean(success && hasRegisterPressureCandidate(sourceCode)),
      evidence: "Se detectaron expresiones o llamadas donde aplica el analisis de registros.",
    },
    {
      key: "peephole",
      name: OPTIMIZATION_LABELS.peephole,
      applied: peepholeApplied,
      evidence: peepholeApplied
        ? "El assembly optimizado difiere del assembly generado originalmente."
        : "No se observaron cambios entre el assembly original y el optimizado.",
    },
  ]

  return details.map((detail) => ({
    ...detail,
    status: detail.applied ? "Aplicado" : "No aplicado",
  }))
}

function buildOptimizationFlags(details) {
  return details.reduce(
    (flags, detail) => ({
      ...flags,
      [detail.key]: detail.applied,
    }),
    {
      constantFolding: false,
      cascada: false,
      sethiUllman: false,
      peephole: false,
    },
  )
}

module.exports = {
  buildOptimizationDetails,
  buildOptimizationFlags,
}
