function countChar(text, char) {
  return [...text].filter((current) => current === char).length;
}

function cleanStatement(line) {
  return line.replace(/;\s*$/, "").trim();
}

function parseParams(rawParams) {
  const text = rawParams.trim();
  if (!text) {
    return [];
  }

  return text.split(",").map((param) => {
    const [name, type] = param.split(":").map((part) => part.trim());
    return {
      type: "Param",
      name: name || param.trim(),
      dataType: type || null,
    };
  });
}

function expressionNode(expression) {
  return {
    type: "Expression",
    source: cleanStatement(expression),
  };
}

function statementFromLine(line) {
  const source = cleanStatement(line);
  if (!source || source === "}") {
    return null;
  }

  let match = source.match(/^print\s*\((.*)\)$/);
  if (match) {
    return {
      type: "PrintStmt",
      expression: expressionNode(match[1]),
      source,
    };
  }

  match = source.match(/^return(?:\s+(.+))?$/);
  if (match) {
    return {
      type: "ReturnStmt",
      expression: match[1] ? expressionNode(match[1]) : null,
      source,
    };
  }

  match = source.match(/^(var|const)\s+([A-Za-z_][A-Za-z0-9_]*)(?:\s*:\s*([^=]+?))?\s*=\s*(.+)$/);
  if (match) {
    return {
      type: match[1] === "var" ? "VarDecl" : "ConstDecl",
      name: match[2],
      dataType: match[3]?.trim() || null,
      initializer: expressionNode(match[4]),
      source,
    };
  }

  match = source.match(/^([A-Za-z_][A-Za-z0-9_]*(?:\.[A-Za-z_][A-Za-z0-9_]*)?)\s*=\s*(.+)$/);
  if (match) {
    return {
      type: "AssignStmt",
      target: match[1],
      expression: expressionNode(match[2]),
      source,
    };
  }

  if (/^if\b/.test(source)) {
    return { type: "IfStmt", condition: expressionNode(source.replace(/^if\b/, "").replace(/\{$/, "")), source };
  }

  if (/^while\b/.test(source)) {
    return { type: "WhileStmt", condition: expressionNode(source.replace(/^while\b/, "").replace(/\{$/, "")), source };
  }

  if (/^for\b/.test(source)) {
    return { type: "ForStmt", header: source.replace(/^for\b/, "").replace(/\{$/, "").trim(), source };
  }

  if (/^switch\b/.test(source)) {
    return { type: "SwitchStmt", expression: expressionNode(source.replace(/^switch\b/, "").replace(/\{$/, "")), source };
  }

  if (/^break\b/.test(source)) {
    return { type: "BreakStmt", source };
  }

  if (/^continue\b/.test(source)) {
    return { type: "ContinueStmt", source };
  }

  return {
    type: "RawStmt",
    source,
  };
}

function declarationFromLine(line) {
  const source = cleanStatement(line);

  let match = source.match(/^(pub\s+)?fn\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(([^)]*)\)\s*([A-Za-z_][A-Za-z0-9_\*\?]*)?/);
  if (match) {
    return {
      type: "Fundec",
      visibility: match[1] ? "pub" : "private",
      name: match[2],
      params: parseParams(match[3]),
      returnType: match[4] || null,
      body: [],
      source,
    };
  }

  match = source.match(/^struct\s+([A-Za-z_][A-Za-z0-9_]*)/);
  if (match) {
    return {
      type: "Structdec",
      name: match[1],
      fields: [],
      source,
    };
  }

  match = source.match(/^(var|const)\s+([A-Za-z_][A-Za-z0-9_]*)(?:\s*:\s*([^=]+?))?\s*=\s*(.+)$/);
  if (match) {
    return {
      type: match[1] === "var" ? "VarDec" : "ConstDec",
      name: match[2],
      dataType: match[3]?.trim() || null,
      initializer: expressionNode(match[4]),
      source,
    };
  }

  if (source) {
    return {
      type: "TopLevelRaw",
      source,
    };
  }

  return null;
}

function buildAst(sourceCode, tokens = []) {
  const program = {
    type: "Programa",
    declarations: [],
    tokenCount: tokens.length,
  };

  const stack = [];
  const lines = sourceCode.split(/\r?\n/);

  lines.forEach((originalLine, lineIndex) => {
    const line = originalLine.trim();
    if (!line) {
      return;
    }

    while (line.startsWith("}") && stack.length > 0) {
      stack.pop();
      if (line === "}") {
        return;
      }
      break;
    }

    const current = stack[stack.length - 1];
    if (!current) {
      const declaration = declarationFromLine(line);
      if (!declaration) {
        return;
      }
      declaration.line = lineIndex + 1;
      program.declarations.push(declaration);

      if (declaration.type === "Fundec" || declaration.type === "Structdec") {
        const opened = countChar(line, "{");
        const closed = countChar(line, "}");
        if (opened > closed) {
          stack.push(declaration);
        }
      }
      return;
    }

    if (current.type === "Structdec") {
      const field = line.match(/^([A-Za-z_][A-Za-z0-9_]*)\s*:\s*([^,;]+)[,;]?$/);
      if (field) {
        current.fields.push({
          type: "Field",
          name: field[1],
          dataType: field[2].trim(),
          line: lineIndex + 1,
        });
      }
    } else {
      const statement = statementFromLine(line);
      if (statement) {
        statement.line = lineIndex + 1;
        current.body.push(statement);
      }
    }

    const opened = countChar(line, "{");
    const closed = countChar(line, "}");
    if (closed > opened && stack.length > 0) {
      stack.pop();
    }
  });

  return program;
}

module.exports = {
  buildAst,
};
