function buildAst(tokens) {
  const parser = new AstParser(tokens)
  return parser.parseProgram()
}

class AstParser {
  constructor(tokens) {
    this.tokens = tokens.filter((token) => token.type !== "END")
    this.current = 0
  }

  parseProgram() {
    const declarations = []
    while (!this.isAtEnd()) declarations.push(this.parseTopLevelDeclaration())

    return {
      type: "Program",
      declarations,
      summary: {
        declarations: declarations.length,
        functions: declarations.filter((node) => node.type === "FunctionDeclaration").length,
        structs: declarations.filter((node) => node.type === "StructDeclaration").length,
        globals: declarations.filter((node) => ["VariableDeclaration", "ConstDeclaration"].includes(node.type)).length,
      },
    }
  }

  parseTopLevelDeclaration() {
    if (this.match("VAR")) return this.parseVariableDeclaration("VariableDeclaration", true)
    if (this.match("CONST")) return this.parseVariableDeclaration("ConstDeclaration", true)
    if (this.match("STRUCT")) return this.parseStructDeclaration("StructDeclaration")
    if (this.match("UNION")) return this.parseStructDeclaration("UnionDeclaration")
    if (this.match("COMPTIME")) return { type: "ComptimeBlock", body: this.parseBlock() }
    if (this.match("PUB")) {
      this.consume("FN")
      return this.parseFunctionDeclaration({ public: true })
    }
    if (this.match("FN")) return this.parseFunctionDeclaration({ public: false })
    return this.parseUnknown("TopLevelDeclaration")
  }

  parseFunctionDeclaration(meta) {
    const name = this.consume("ID")?.lexeme || "<anonymous>"
    const template = this.match("MENOR") ? this.parseTemplateSuffix() : null
    this.consume("LPAREN")
    const params = this.parseParams()
    this.consume("RPAREN")
    this.match("DOSPUNTOS")
    const returnType = this.parseTypeUntil(["LBRACE"])
    const body = this.parseBlock()

    return {
      type: "FunctionDeclaration",
      name,
      public: meta.public,
      template,
      params,
      returnType,
      body,
    }
  }

  parseTemplateSuffix() {
    const parts = []
    let depth = 1
    while (!this.isAtEnd() && depth > 0) {
      if (this.match("MENOR")) {
        depth += 1
        parts.push("<")
        continue
      }
      if (this.match("MAYOR")) {
        depth -= 1
        if (depth > 0) parts.push(">")
        continue
      }
      const token = this.advance()
      parts.push(token?.lexeme || token?.type || "")
    }
    return { type: "TemplateParameters", text: parts.join(" ").trim() }
  }

  parseParams() {
    const params = []
    while (!this.check("RPAREN") && !this.isAtEnd()) {
      const comptime = this.match("COMPTIME")
      const name = this.consume("ID")?.lexeme || "<param>"
      this.consume("DOSPUNTOS")
      const paramType = this.parseTypeUntil(["COMA", "RPAREN"])
      params.push({ type: "Parameter", name, paramType, comptime })
      this.match("COMA")
    }
    return params
  }

  parseStructDeclaration(type) {
    const name = this.consume("ID")?.lexeme || "<type>"
    const fields = []
    if (this.match("LPAREN")) {
      while (!this.check("RPAREN") && !this.isAtEnd()) this.advance()
      this.consume("RPAREN")
    }
    this.consume("LBRACE")
    while (!this.check("RBRACE") && !this.isAtEnd()) {
      const fieldName = this.consume("ID")?.lexeme || "<field>"
      this.consume("DOSPUNTOS")
      const fieldType = this.parseTypeUntil(["SEMICOL", "RBRACE"])
      this.match("SEMICOL")
      fields.push({ type: "FieldDeclaration", name: fieldName, fieldType })
    }
    this.consume("RBRACE")
    return { type, name, fields }
  }

  parseBlock() {
    const statements = []
    this.consume("LBRACE")
    while (!this.check("RBRACE") && !this.isAtEnd()) statements.push(this.parseStatement())
    this.consume("RBRACE")
    return { type: "BlockStatement", statements }
  }

  parseStatement() {
    if (this.match("COMA")) return { type: "EmptyStatement" }
    if (this.match("VAR")) return this.parseVariableDeclaration("VariableDeclaration", false)
    if (this.match("CONST")) return this.parseVariableDeclaration("ConstDeclaration", false)
    if (this.match("RETURN")) return this.parseReturnStatement()
    if (this.match("PRINT")) return this.parsePrintStatement()
    if (this.match("FREE", "DELETE")) return this.parseDeleteStatement()
    if (this.match("BREAK")) return this.finishSimpleStatement("BreakStatement")
    if (this.match("CONTINUE")) return this.finishSimpleStatement("ContinueStatement")
    if (this.match("IF")) return this.parseIfStatement()
    if (this.match("WHILE")) return this.parseWhileStatement()
    if (this.match("FOR")) return this.parseForStatement()
    if (this.match("SWITCH")) return this.parseSwitchStatement()
    return this.parseAssignmentOrExpressionStatement()
  }

  parseVariableDeclaration(type, topLevel) {
    const name = this.consume("ID")?.lexeme || "<variable>"
    let valueType = null
    if (this.match("DOSPUNTOS")) valueType = this.parseTypeUntil(["ASSIGN"])
    this.consume("ASSIGN")
    const initializer = this.parseExpression()
    this.match("SEMICOL")
    return { type, name, valueType, initializer, scope: topLevel ? "global" : "local" }
  }

  parseReturnStatement() {
    if (this.check("SEMICOL") || this.check("RBRACE")) {
      this.match("SEMICOL")
      return { type: "ReturnStatement", argument: null }
    }
    const argument = this.parseExpression()
    this.match("SEMICOL")
    return { type: "ReturnStatement", argument }
  }

  parsePrintStatement() {
    this.consume("LPAREN")
    const argument = this.parseExpression()
    this.consume("RPAREN")
    this.match("SEMICOL")
    return { type: "PrintStatement", argument }
  }

  parseDeleteStatement() {
    const argument = this.parseExpression()
    this.match("SEMICOL")
    return { type: "DeleteStatement", argument }
  }

  parseIfStatement() {
    this.consume("LPAREN")
    const test = this.parseExpression()
    this.consume("RPAREN")
    this.match("THEN")
    const consequent = this.parseBlock()
    let alternate = null
    if (this.match("ELSE")) {
      this.match("THEN")
      alternate = this.parseBlock()
    }
    return { type: "IfStatement", test, consequent, alternate }
  }

  parseWhileStatement() {
    this.consume("LPAREN")
    const test = this.parseExpression()
    this.consume("RPAREN")
    return { type: "WhileStatement", test, body: this.parseBlock() }
  }

  parseForStatement() {
    this.consume("LPAREN")
    const initializer = this.parseExpression()
    this.consume("RPAREN")
    return { type: "ForStatement", initializer, body: this.parseBlock() }
  }

  parseSwitchStatement() {
    this.consume("LPAREN")
    const discriminant = this.parseExpression()
    this.consume("RPAREN")
    return { type: "SwitchStatement", discriminant, body: this.parseBlock() }
  }

  parseAssignmentOrExpressionStatement() {
    const start = this.current
    const expression = this.parseExpression()
    if (this.match("ASSIGN")) {
      const value = this.parseExpression()
      this.match("SEMICOL")
      return { type: "AssignmentStatement", target: expression, value }
    }
    this.match("SEMICOL")
    if (this.current === start && !this.isAtEnd()) {
      const token = this.advance()
      return { type: "UnknownStatement", raw: token?.lexeme || token?.type || "" }
    }
    return { type: "ExpressionStatement", expression }
  }

  finishSimpleStatement(type) {
    this.match("SEMICOL")
    return { type }
  }

  parseExpression() {
    const parts = []
    let depth = 0
    while (!this.isAtEnd()) {
      const token = this.peek()
      if (depth === 0 && ["SEMICOL", "COMA", "RPAREN", "RBRACE"].includes(token.type)) break
      if (["LPAREN", "LCORCHETE", "LBRACE"].includes(token.type)) depth += 1
      if (["RPAREN", "RCORCHETE", "RBRACE"].includes(token.type)) depth = Math.max(0, depth - 1)
      parts.push(this.advance().lexeme || token.type)
    }
    return { type: "Expression", raw: parts.join(" ").trim() }
  }

  parseTypeUntil(stopTypes) {
    const parts = []
    let bracketDepth = 0
    while (!this.isAtEnd()) {
      const token = this.peek()
      if (bracketDepth === 0 && stopTypes.includes(token.type)) break
      if (token.type === "LCORCHETE") bracketDepth += 1
      if (token.type === "RCORCHETE") bracketDepth = Math.max(0, bracketDepth - 1)
      parts.push(this.advance().lexeme || token.type)
    }
    return { type: "TypeReference", name: parts.join(" ").trim() || "void" }
  }

  parseUnknown(type) {
    const tokens = []
    while (!this.isAtEnd() && !this.check("SEMICOL") && !this.check("RBRACE")) tokens.push(this.advance().lexeme)
    this.match("SEMICOL")
    return { type: `Unknown${type}`, raw: tokens.join(" ") }
  }

  match(...types) {
    if (!types.includes(this.peek()?.type)) return false
    this.advance()
    return true
  }

  consume(type) {
    if (this.check(type)) return this.advance()
    return null
  }

  check(type) {
    return this.peek()?.type === type
  }

  advance() {
    if (!this.isAtEnd()) this.current += 1
    return this.previous()
  }

  previous() {
    return this.tokens[this.current - 1]
  }

  peek() {
    return this.tokens[this.current]
  }

  isAtEnd() {
    return this.current >= this.tokens.length
  }
}

module.exports = {
  buildAst,
}
