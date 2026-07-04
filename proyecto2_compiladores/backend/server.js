const http = require("http")
const { buildAst } = require("./compiler/ast-builder")
const { buildCompileResult, calculateStats, normalizeSourceCode } = require("./compiler/compile-result")
const { runCompiler } = require("./compiler/compiler-runner")
const { buildOptimizationDetails, buildOptimizationFlags } = require("./compiler/optimization-details")
const { tokenize } = require("./compiler/tokenizer")

const PORT = Number(process.env.PORT || 3001)

function sendJson(res, statusCode, payload) {
  const body = JSON.stringify(payload)
  res.writeHead(statusCode, {
    "Content-Type": "application/json; charset=utf-8",
    "Content-Length": Buffer.byteLength(body),
    "Access-Control-Allow-Origin": "*",
    "Access-Control-Allow-Methods": "GET,POST,OPTIONS",
    "Access-Control-Allow-Headers": "Content-Type, Authorization",
  })
  res.end(body)
}

function readJsonBody(req) {
  return new Promise((resolve, reject) => {
    let raw = ""
    req.setEncoding("utf8")
    req.on("data", (chunk) => {
      raw += chunk
      if (raw.length > 1024 * 1024) {
        reject(new Error("El cuerpo de la solicitud es demasiado grande."))
        req.destroy()
      }
    })
    req.on("end", () => {
      if (!raw.trim()) return resolve({})
      try {
        resolve(JSON.parse(raw))
      } catch {
        reject(new Error("JSON invalido."))
      }
    })
    req.on("error", reject)
  })
}

async function compileSource(sourceCode) {
  const scanner = tokenize(sourceCode)
  const stats = calculateStats(sourceCode, scanner.tokens)
  const ast = scanner.success ? buildAst(scanner.tokens) : null

  if (!scanner.success) {
    return buildCompileResult({
      tokens: scanner.tokens,
      scannerStatus: "Error",
      scannerSuccess: false,
      stats,
      ast,
      errors: scanner.errors,
    })
  }

  const compilerResult = await runCompiler(sourceCode)
  const optimizationDetails = buildOptimizationDetails({
    sourceCode,
    success: compilerResult.success,
    assembly: compilerResult.assembly,
    optimizedAssembly: compilerResult.optimizedAssembly,
  })

  return buildCompileResult({
    success: compilerResult.success,
    tokens: scanner.tokens,
    scannerStatus: "Correcto",
    scannerSuccess: true,
    parseStatus: compilerResult.parseStatus,
    parseSuccess: compilerResult.parseSuccess,
    assembly: compilerResult.assembly,
    optimizedAssembly: compilerResult.optimizedAssembly,
    ast,
    stats,
    errors: [...scanner.errors, ...compilerResult.errors],
    optimizationDetails,
    optimizations: buildOptimizationFlags(optimizationDetails),
  })
}

async function handleCompile(req, res) {
  try {
    const body = await readJsonBody(req)
    const sourceCode = normalizeSourceCode(body.sourceCode ?? body.source ?? body.code ?? body.input)
    const result = await compileSource(sourceCode)
    sendJson(res, 200, result)
  } catch (error) {
    sendJson(res, 500, buildCompileResult({
      errors: [error.message || "Error interno del backend."],
    }))
  }
}

const server = http.createServer(async (req, res) => {
  if (req.method === "OPTIONS") {
    return sendJson(res, 204, {})
  }

  if (req.method === "GET" && req.url === "/health") {
    return sendJson(res, 200, { ok: true, service: "MiniZig compiler backend" })
  }

  if (req.method === "POST" && (req.url === "/compile" || req.url === "/api/compile")) {
    return handleCompile(req, res)
  }

  sendJson(res, 404, { error: "Ruta no encontrada." })
})

if (require.main === module) {
  server.listen(PORT, () => {
    console.log(`Backend MiniZig escuchando en http://localhost:${PORT}`)
  })
}

module.exports = {
  compileSource,
  PORT,
  server,
}
