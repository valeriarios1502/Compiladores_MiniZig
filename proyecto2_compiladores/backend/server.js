const http = require("http");
const { compileSource } = require("./compiler/compiler-runner");

const PORT = Number(process.env.PORT || 3001);
const HOST = process.env.HOST || "0.0.0.0";
const MAX_BODY_BYTES = Number(process.env.MAX_BODY_BYTES || 1024 * 1024);

function sendJson(res, statusCode, payload) {
  const body = JSON.stringify(payload, null, 2);
  res.writeHead(statusCode, {
    "Content-Type": "application/json; charset=utf-8",
    "Content-Length": Buffer.byteLength(body),
    "Access-Control-Allow-Origin": "*",
    "Access-Control-Allow-Methods": "GET,POST,OPTIONS",
    "Access-Control-Allow-Headers": "Content-Type, Authorization",
  });
  res.end(body);
}

function readRequestBody(req) {
  return new Promise((resolve, reject) => {
    const chunks = [];
    let total = 0;

    req.on("data", (chunk) => {
      total += chunk.length;
      if (total > MAX_BODY_BYTES) {
        reject(new Error("El cuerpo de la solicitud excede el limite permitido."));
        req.destroy();
        return;
      }
      chunks.push(chunk);
    });

    req.on("end", () => resolve(Buffer.concat(chunks).toString("utf8")));
    req.on("error", reject);
  });
}

async function handleCompile(req, res) {
  let payload;
  try {
    const rawBody = await readRequestBody(req);
    payload = rawBody ? JSON.parse(rawBody) : {};
  } catch (error) {
    sendJson(res, 400, {
      success: false,
      message: "JSON invalido.",
      errors: [error.message],
    });
    return;
  }

  const sourceCode = payload.sourceCode ?? payload.code ?? payload.source ?? "";
  if (typeof sourceCode !== "string" || sourceCode.trim() === "") {
    sendJson(res, 200, {
      success: false,
      message: "No se recibio codigo fuente para compilar.",
      errors: ["El campo sourceCode, code o source es obligatorio."],
      tokens: [],
      ast: null,
      assembly: "",
      unoptimizedAssembly: "",
      optimizedAssembly: "",
      optimizationMessages: [],
    });
    return;
  }

  try {
    const result = compileSource(sourceCode, {
      fileName: typeof payload.fileName === "string" ? payload.fileName : "input.txt",
    });
    sendJson(res, 200, result);
  } catch (error) {
    sendJson(res, 200, {
      success: false,
      message: "Error interno al ejecutar el backend del compilador.",
      errors: [error.stack || error.message],
      tokens: [],
      ast: null,
      assembly: "",
      unoptimizedAssembly: "",
      optimizedAssembly: "",
      optimizationMessages: [],
    });
  }
}

const server = http.createServer((req, res) => {
  const url = new URL(req.url, `http://${req.headers.host || "localhost"}`);

  if (req.method === "OPTIONS") {
    sendJson(res, 204, {});
    return;
  }

  if (req.method === "GET" && url.pathname === "/health") {
    sendJson(res, 200, {
      status: "ok",
      service: "minizig-backend",
      endpoints: ["/compile", "/api/compile"],
    });
    return;
  }

  if (req.method === "POST" && (url.pathname === "/compile" || url.pathname === "/api/compile")) {
    handleCompile(req, res);
    return;
  }

  sendJson(res, 404, {
    success: false,
    message: "Ruta no encontrada.",
  });
});

server.on("error", (error) => {
  if (error.code === "EADDRINUSE") {
    console.error(`Puerto ${PORT} ocupado. Cierra el proceso anterior o usa PORT=otro_puerto node server.js.`);
    process.exit(1);
  }

  console.error("No se pudo iniciar el servidor:", error);
  process.exit(1);
});

server.listen(PORT, HOST, () => {
  console.log(`MiniZig backend escuchando en http://${HOST}:${PORT}`);
});
