function compilerErrorLines(text) {
  if (!text) {
    return [];
  }

  return text
    .split(/\r?\n/)
    .map((line) => line.trim())
    .filter((line) => /error|Error|Se esperaba|Caracter invalido|no exitoso/i.test(line));
}

function buildCompileResult({
  sourceCode,
  fileName,
  compilerExitCode,
  stdout,
  stderr,
  tokens,
  tokensText,
  ast,
  unoptimizedAssembly,
  optimizedAssembly,
  optimization,
  buildError,
}) {
  const compilerOk = !buildError && compilerExitCode === 0 && Boolean(optimizedAssembly);
  const errors = buildError
    ? [buildError]
    : [...compilerErrorLines(stderr), ...compilerErrorLines(stdout)];

  const optimizationMessages = optimization.messages;

  return {
    success: compilerOk,
    message: compilerOk ? "Compilacion exitosa." : "El compilador reporto errores.",
    fileName,
    sourceCode,
    parseStatus: compilerOk ? "Parseo exitoso" : "Parseo no exitoso",
    compilerExitCode,
    errors,
    errorMessages: errors,
    stdout,
    stderr,
    tokens,
    tokensText,
    ast,
    assembly: unoptimizedAssembly || "",
    unoptimizedAssembly: unoptimizedAssembly || "",
    assemblyUnoptimized: unoptimizedAssembly || "",
    optimizedAssembly: optimizedAssembly || "",
    assemblyOptimized: optimizedAssembly || "",
    optimized_assembly: optimizedAssembly || "",
    optimizationMessages,
    optimizations: optimization,
  };
}

module.exports = {
  buildCompileResult,
};
