# 📦 ctoml

Simple single header parsing library for TOML files.

## 🚀 Build & Run

```bash
./build.sh
```

## ❌ Error Handling Strategy

The lexer and parser split error responsibility by layer:

### Lexer — Character-Level Errors

The lexer rejects tokens that are **structurally malformed in isolation**, regardless of position in the grammar.

| Error                     | Example           | Token         |
| ------------------------- | ----------------- | ------------- |
| Invalid character         | bare `\r`, `\x00` | `TOK_INVALID` |
| Invalid escape sequence   | `\q`, `\uDFFF`    | `TOK_INVALID` |
| Unterminated string       | `"hello` at EOF   | `TOK_INVALID` |
| Invalid numeric structure | `23abc`, `1__0`   | `TOK_INVALID` |

### Parser — Sequence-Level Errors

The parser rejects token sequences that are **invalid in context**, where the lexer has no way of knowing whether a token is meaningful without grammar context.

| Error                  | Example                  |
| ---------------------- | ------------------------ |
| Invalid value start    | `key = .7`               |
| Missing `=`            | `key value`              |
| Duplicate keys         | `key = 1` then `key = 1` |
| Malformed table header | `[[key]`                 |

### Rationale

The lexer operates statelessly on individual tokens — it rejects what is **never valid** as a token. The parser operates on sequences — it rejects what is **not valid in position**. `.7` for example is two valid tokens (`TOK_DOT` + `TOK_INTEGER`); only the parser knows that a dot is not a legal value start.
