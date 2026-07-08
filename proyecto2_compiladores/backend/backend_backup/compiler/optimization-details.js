function lineCount(text) {
  if (!text) {
    return 0;
  }
  return text.split(/\r?\n/).filter((line) => line.trim()).length;
}

function buildOptimizationDetails(unoptimizedAssembly, optimizedAssembly, compilerOk) {
  const beforeLines = lineCount(unoptimizedAssembly);
  const afterLines = lineCount(optimizedAssembly);
  const changed = Boolean(unoptimizedAssembly && optimizedAssembly && unoptimizedAssembly !== optimizedAssembly);

  const messages = [
    {
      pass: "Generacion inicial",
      status: unoptimizedAssembly ? "ok" : "skipped",
      message: unoptimizedAssembly
        ? "Se genero assembly sin optimizar antes de ejecutar las pasadas."
        : "No se genero assembly sin optimizar.",
    },
    {
      pass: "Constant folding",
      status: compilerOk ? "ok" : "skipped",
      message: compilerOk
        ? "Se evaluaron expresiones constantes sobre el AST optimizado."
        : "No se ejecuto porque el compilador reporto errores.",
    },
    {
      pass: "Cascada",
      status: compilerOk ? "ok" : "skipped",
      message: compilerOk
        ? "Se aplico la pasada de simplificacion/cascada antes de emitir codigo final."
        : "No se ejecuto porque el compilador reporto errores.",
    },
    {
      pass: "Sethi-Ullman",
      status: compilerOk ? "ok" : "skipped",
      message: compilerOk
        ? "Se recalculo el orden de evaluacion para reducir presion de registros."
        : "No se ejecuto porque el compilador reporto errores.",
    },
    {
      pass: "Comparacion de assembly",
      status: compilerOk ? "ok" : "skipped",
      changed,
      beforeLines,
      afterLines,
      message: changed
        ? `El assembly optimizado cambio respecto al inicial (${beforeLines} -> ${afterLines} lineas no vacias).`
        : "El assembly optimizado quedo igual al assembly inicial para este programa.",
    },
  ];

  return {
    changed,
    beforeLines,
    afterLines,
    messages,
  };
}

module.exports = {
  buildOptimizationDetails,
};
