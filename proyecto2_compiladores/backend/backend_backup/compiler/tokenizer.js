function parseTokenLine(line, index) {
  const trimmed = line.trim();
  const match = trimmed.match(/^TOKEN\(([^,\)]+)(?:,\s*"((?:\\"|[^"])*)")?\)$/);
  if (!match) {
    return null;
  }

  return {
    index,
    type: match[1],
    lexeme: match[2] ?? "",
    raw: trimmed,
  };
}

function parseTokens(tokensText) {
  const tokens = [];
  const lines = tokensText.split(/\r?\n/);

  for (const line of lines) {
    const token = parseTokenLine(line, tokens.length);
    if (token) {
      tokens.push(token);
    }
  }

  return tokens;
}

module.exports = {
  parseTokens,
};
