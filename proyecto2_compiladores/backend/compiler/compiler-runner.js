const fs = require("fs/promises")
const path = require("path")
const os = require("os")
const { execFile } = require("child_process")

async function findCompilerPath() {
  const root = path.join(__dirname, "..", "..")
  const candidates =
    process.platform === "win32"
      ? [
          path.join(root, "Proyecto2-win.exe"),
          path.join(root, "Proyecto2.exe"),
          path.join(root, "build", "Proyecto2.exe"),
        ]
      : [
          path.join(root, "build", "Proyecto2"),
          path.join(root, "Proyecto2"),
        ]

  for (const candidate of candidates) {
    try {
      await fs.access(candidate)
      return candidate
    } catch {}
  }

  throw new Error("No se encontro el ejecutable del compilador.")
}

function extractSection(output, name) {
  const startMarker = `__ASSEMBLY_${name}_BEGIN__`
  const endMarker = `__ASSEMBLY_${name}_END__`
  const start = output.indexOf(startMarker)
  const end = output.indexOf(endMarker)

  if (start === -1 || end === -1 || end < start) return ""

  return output
    .slice(start + startMarker.length, end)
    .replace(/^\r?\n/, "")
    .replace(/\r?\n$/, "")
}

function parseCompilerOutput(stdout) {
  const normalized = stdout.replace(/\r\n/g, "\n")
  const lines = normalized.split("\n")
  const parseSuccess = lines[0]?.trim() === "Parseo exitoso"
  const output = parseSuccess ? lines.slice(1).join("\n").trimStart() : normalized
  const assembly = extractSection(output, "UNOPTIMIZED") || (parseSuccess ? output : "")
  const optimizedAssembly = extractSection(output, "OPTIMIZED") || assembly

  return {
    parseSuccess,
    parseStatus: parseSuccess ? "Correcto" : "Error",
    assembly,
    optimizedAssembly,
    rawOutput: normalized,
  }
}

async function runCompiler(sourceCode) {
  const tempFile = path.join(os.tmpdir(), `minizig-${Date.now()}-${process.pid}.txt`)
  await fs.writeFile(tempFile, sourceCode, "utf8")

  try {
    const compilerPath = await findCompilerPath()

    return await new Promise((resolve) => {
      execFile(compilerPath, [tempFile], { timeout: 5000 }, (error, stdout, stderr) => {
        const parsedOutput = parseCompilerOutput(stdout || "")
        const compilerErrors = [stderr, error?.message].filter(Boolean)
        const success = !error && !stderr && parsedOutput.parseSuccess

        resolve({
          success,
          parseStatus: success ? "Correcto" : parsedOutput.parseStatus,
          parseSuccess: success,
          assembly: success ? parsedOutput.assembly : "",
          optimizedAssembly: success ? parsedOutput.optimizedAssembly : "",
          errors: compilerErrors,
          rawOutput: parsedOutput.rawOutput,
        })
      })
    })
  } finally {
    await fs.unlink(tempFile).catch(() => {})
  }
}

module.exports = {
  findCompilerPath,
  parseCompilerOutput,
  runCompiler,
}
