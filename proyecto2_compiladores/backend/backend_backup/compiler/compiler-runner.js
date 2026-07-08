const fs = require("fs");
const os = require("os");
const path = require("path");
const { spawnSync } = require("child_process");
const { parseTokens } = require("./tokenizer");
const { buildAst } = require("./ast-builder");
const { buildOptimizationDetails } = require("./optimization-details");
const { buildCompileResult } = require("./compile-result");

const PROJECT_ROOT = path.resolve(__dirname, "..", "..");
const COMPILER_EXE = path.join(PROJECT_ROOT, "Proyecto2.exe");
const SOURCE_FILES = [
  "main.cpp",
  "scanner.cpp",
  "token.cpp",
  "parser.cpp",
  "ast.cpp",
  "visitor.cpp",
  "Typechecker.cpp",
];

function readFileIfExists(filePath) {
  if (!fs.existsSync(filePath)) {
    return "";
  }
  return fs.readFileSync(filePath, "utf8");
}

function newestSourceTime() {
  return SOURCE_FILES.reduce((newest, sourceFile) => {
    const sourcePath = path.join(PROJECT_ROOT, sourceFile);
    if (!fs.existsSync(sourcePath)) {
      return newest;
    }
    return Math.max(newest, fs.statSync(sourcePath).mtimeMs);
  }, 0);
}

function needsRebuild() {
  if (!fs.existsSync(COMPILER_EXE)) {
    return true;
  }
  return newestSourceTime() > fs.statSync(COMPILER_EXE).mtimeMs;
}

function ensureCompiler() {
  if (!needsRebuild()) {
    return null;
  }

  const result = spawnSync(
    "g++",
    ["-std=c++14", "-o", COMPILER_EXE, ...SOURCE_FILES],
    {
      cwd: PROJECT_ROOT,
      encoding: "utf8",
      timeout: 30000,
      windowsHide: true,
    },
  );

  if (result.error) {
    return result.error.message;
  }

  if (result.status !== 0) {
    return [result.stdout, result.stderr].filter(Boolean).join("\n").trim()
      || "No se pudo compilar Proyecto2.exe.";
  }

  return null;
}

function safeInputFileName(fileName) {
  const baseName = path.basename(fileName || "input.txt");
  if (!/\.[A-Za-z0-9]+$/.test(baseName)) {
    return `${baseName}.txt`;
  }
  return baseName;
}

function runCompiler(inputPath) {
  return spawnSync(COMPILER_EXE, [inputPath], {
    cwd: PROJECT_ROOT,
    encoding: "utf8",
    timeout: 30000,
    windowsHide: true,
  });
}

function compileSource(sourceCode, options = {}) {
  const fileName = safeInputFileName(options.fileName);
  const ast = buildAst(sourceCode, []);
  const buildError = ensureCompiler();

  if (buildError) {
    const optimization = buildOptimizationDetails("", "", false);
    return buildCompileResult({
      sourceCode,
      fileName,
      compilerExitCode: null,
      stdout: "",
      stderr: buildError,
      tokens: [],
      tokensText: "",
      ast,
      unoptimizedAssembly: "",
      optimizedAssembly: "",
      optimization,
      buildError,
    });
  }

  const tempDir = fs.mkdtempSync(path.join(os.tmpdir(), "minizig-backend-"));
  const inputPath = path.join(tempDir, fileName);
  fs.writeFileSync(inputPath, sourceCode, "utf8");

  const inputBase = inputPath.replace(/\.[^.\\\/]+$/, "");
  const tokensPath = `${inputBase}_tokens.txt`;
  const unoptimizedPath = `${inputBase}_unoptimized.s`;
  const optimizedPath = `${inputBase}.s`;

  let compilerRun;
  try {
    compilerRun = runCompiler(inputPath);

    const tokensText = readFileIfExists(tokensPath);
    const tokens = parseTokens(tokensText);
    const astWithTokens = buildAst(sourceCode, tokens);
    const unoptimizedAssembly = readFileIfExists(unoptimizedPath);
    const optimizedAssembly = readFileIfExists(optimizedPath);
    const compilerOk = compilerRun.status === 0 && Boolean(optimizedAssembly);
    const optimization = buildOptimizationDetails(unoptimizedAssembly, optimizedAssembly, compilerOk);

    return buildCompileResult({
      sourceCode,
      fileName,
      compilerExitCode: compilerRun.status,
      stdout: compilerRun.stdout || "",
      stderr: compilerRun.stderr || compilerRun.error?.message || "",
      tokens,
      tokensText,
      ast: astWithTokens,
      unoptimizedAssembly,
      optimizedAssembly,
      optimization,
      buildError: null,
    });
  } finally {
    if (process.env.MINIZIG_KEEP_TEMP !== "1") {
      fs.rmSync(tempDir, { recursive: true, force: true });
    }
  }
}

module.exports = {
  compileSource,
};
