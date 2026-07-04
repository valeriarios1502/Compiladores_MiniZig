const KEYWORDS = new Map([
  ["fn", "FN"],
  ["endfn", "ENDFN"],
  ["return", "RETURN"],
  ["print", "PRINT"],
  ["for", "FOR"],
  ["endfor", "ENDFOR"],
  ["true", "TRUE"],
  ["break", "BREAK"],
  ["false", "FALSE"],
  ["const", "CONST"],
  ["if", "IF"],
  ["else", "ELSE"],
  ["then", "THEN"],
  ["endif", "ENDIF"],
  ["do", "DO"],
  ["while", "WHILE"],
  ["endwhile", "ENDWHILE"],
  ["var", "VAR"],
  ["and", "AND"],
  ["or", "OR"],
  ["not", "NOT"],
  ["struct", "STRUCT"],
  ["new", "NEW"],
  ["delete", "DELETE"],
  ["type", "TYPE"],
  ["pub", "PUB"],
  ["continue", "CONTINUE"],
  ["comptime", "COMPTIME"],
  ["defer", "DEFER"],
  ["switch", "SWITCH"],
  ["case", "CASE"],
  ["try", "TRY"],
  ["catch", "CATCH"],
  ["null", "NULLTOK"],
  ["undefined", "UNDEFINED"],
  ["enum", "ENUM"],
  ["union", "UNION"],
  ["error", "ERROR"],
  ["orelse", "OR"],
  ["free", "FREE"],
])

const SINGLE_CHAR_TOKENS = new Map([
  [",", "COMA"],
  [":", "DOSPUNTOS"],
  [";", "SEMICOL"],
  ["+", "PLUS"],
  ["-", "MINUS"],
  ["*", "STAR"],
  ["/", "DIV"],
  ["(", "LPAREN"],
  [")", "RPAREN"],
  ["[", "LCORCHETE"],
  ["]", "RCORCHETE"],
  ["{", "LBRACE"],
  ["}", "RBRACE"],
  ["%", "MODULO"],
  ["?", "QUESTION"],
])

function isWhitespace(char) {
  return char === " " || char === "\n" || char === "\r" || char === "\t"
}

function isDigit(char) {
  return char >= "0" && char <= "9"
}

function isHexDigit(char) {
  return /^[0-9a-fA-F]$/.test(char)
}

function isIdentifierStart(char) {
  return /^[A-Za-z_]$/.test(char)
}

function isIdentifierPart(char) {
  return /^[A-Za-z0-9_]$/.test(char)
}

function categorizeToken(type) {
  if (type === "ID") return "identifier"
  if ([
    "NUMDECIMAL",
    "NUMFLOTANTE",
    "NUMHEX",
    "NUMBIN",
    "COMILLASDOBLES",
    "COMILLASSIMPLES",
    "TRUE",
    "FALSE",
    "NULLTOK",
    "UNDEFINED",
  ].includes(type)) {
    return "literal"
  }
  if ([...KEYWORDS.values()].includes(type) && type !== "OR") return "keyword"
  if ([
    "PLUS",
    "MINUS",
    "STAR",
    "DIV",
    "MODULO",
    "MENOR",
    "MENORIGUAL",
    "MAYOR",
    "MAYORIGUAL",
    "IGUALIGUAL",
    "DIFERENTE",
    "AND",
    "OR",
    "ASSIGN",
    "NOT",
    "PIPE",
    "REFERENCIA",
    "DOTQUESTION",
  ].includes(type)) {
    return "operator"
  }
  if (type === "ERR") return "error"
  if (type === "END") return "eof"
  return "punctuation"
}

function createToken(type, lexeme, line, column) {
  return {
    type,
    lexeme,
    line,
    column,
    category: categorizeToken(type),
  }
}

function tokenize(sourceCode) {
  const tokens = []
  const errors = []
  let current = 0
  let line = 1
  let column = 1

  const peek = (offset = 0) => sourceCode[current + offset] || ""
  const advance = () => {
    const char = sourceCode[current++]
    if (char === "\n") {
      line += 1
      column = 1
    } else {
      column += 1
    }
    return char
  }

  while (current < sourceCode.length) {
    while (current < sourceCode.length && isWhitespace(peek())) advance()
    if (current >= sourceCode.length) break

    const start = current
    const startLine = line
    const startColumn = column
    const char = peek()

    if (isDigit(char)) {
      advance()
      if (char === "0" && ["x", "X"].includes(peek())) {
        advance()
        while (isHexDigit(peek())) advance()
        tokens.push(createToken("NUMHEX", sourceCode.slice(start, current), startLine, startColumn))
        continue
      }
      if (char === "0" && ["b", "B"].includes(peek())) {
        advance()
        while (peek() === "0" || peek() === "1") advance()
        tokens.push(createToken("NUMBIN", sourceCode.slice(start, current), startLine, startColumn))
        continue
      }
      while (isDigit(peek())) advance()
      if (peek() === ".") {
        advance()
        while (isDigit(peek())) advance()
        tokens.push(createToken("NUMFLOTANTE", sourceCode.slice(start, current), startLine, startColumn))
      } else {
        tokens.push(createToken("NUMDECIMAL", sourceCode.slice(start, current), startLine, startColumn))
      }
      continue
    }

    if (isIdentifierStart(char)) {
      advance()
      while (isIdentifierPart(peek())) advance()
      const lexeme = sourceCode.slice(start, current)
      tokens.push(createToken(KEYWORDS.get(lexeme) || "ID", lexeme, startLine, startColumn))
      continue
    }

    if (char === '"' || char === "'") {
      const quote = char
      advance()
      while (current < sourceCode.length && peek() !== quote) {
        if (peek() === "\\" && current + 1 < sourceCode.length) {
          advance()
          advance()
        } else {
          advance()
        }
      }
      if (current >= sourceCode.length) {
        const label = quote === '"' ? "String" : "Caracter"
        tokens.push(createToken("ERR", sourceCode.slice(start, current), startLine, startColumn))
        errors.push(`${label} sin cerrar en linea ${startLine}, columna ${startColumn}.`)
        break
      }
      advance()
      tokens.push(createToken(quote === '"' ? "COMILLASDOBLES" : "COMILLASSIMPLES", sourceCode.slice(start, current), startLine, startColumn))
      continue
    }

    const twoChar = sourceCode.slice(current, current + 2)
    const twoCharTokens = new Map([
      [".?", "DOTQUESTION"],
      ["!=", "DIFERENTE"],
      ["==", "IGUALIGUAL"],
      ["||", "OR"],
      ["&&", "AND"],
      [">=", "MAYORIGUAL"],
      ["<=", "MENORIGUAL"],
    ])
    if (twoCharTokens.has(twoChar)) {
      advance()
      advance()
      tokens.push(createToken(twoCharTokens.get(twoChar), twoChar, startLine, startColumn))
      continue
    }

    if (SINGLE_CHAR_TOKENS.has(char)) {
      advance()
      tokens.push(createToken(SINGLE_CHAR_TOKENS.get(char), char, startLine, startColumn))
      continue
    }

    const fallbackTokens = new Map([
      [".", "PUNTO"],
      ["!", "NOT"],
      ["=", "ASSIGN"],
      ["|", "PIPE"],
      ["&", "REFERENCIA"],
      [">", "MAYOR"],
      ["<", "MENOR"],
    ])
    if (fallbackTokens.has(char)) {
      advance()
      tokens.push(createToken(fallbackTokens.get(char), char, startLine, startColumn))
      continue
    }

    advance()
    tokens.push(createToken("ERR", char, startLine, startColumn))
    errors.push(`Caracter invalido '${char}' en linea ${startLine}, columna ${startColumn}.`)
    break
  }

  tokens.push(createToken("END", "", line, column))

  return {
    tokens,
    errors,
    success: errors.length === 0,
  }
}

module.exports = {
  tokenize,
}
