# sql-engine

A SQL query engine built from scratch in C++. No libraries. No shortcuts. Raw systems engineering.

## Follow the Build

I'm documenting everything as I build — code, bugs, decisions, and breakdowns.

- 🎥 YouTube — [Stage 1: Lexer / Tokenizer](https://youtu.be/hpChXe3dRUw?si=cEWNURj-0tVlM-Y2)
- 📝 Medium — [Building a SQL Engine From Scratch](https://medium.com/@zoolpher)
- 🐦 X — [@aryanmh0](https://x.com/aryanmh0)

## What is this?

Most developers use SQL engines — this project builds one. From a raw string like:

```sql
SELECT name FROM users WHERE age > 20 ORDER BY name;
```

...to actual query results, every step is implemented by hand: tokenization, parsing, semantic validation, query planning, optimization, and execution.

## Pipeline

```
Raw SQL string
     ↓
Lexer          → breaks string into tokens
     ↓
Parser         → builds an Abstract Syntax Tree (AST)
     ↓
Semantic Analyzer → validates tables, columns, types
     ↓
Query Planner  → converts AST into a logical execution plan
     ↓
Optimizer      → picks the cheapest way to execute
     ↓
Executor       → fetches data, returns results
```

## Project Structure

```
sql-engine/
├── main.cpp                         # entry point, wires all components together
├── CMakeLists.txt                   # build system
│
├── include/                         # shared headers accessible by ALL components
│   ├── token.h                      # defines Token struct and TokenType enum
│   ├── ast.h                        # defines all AST node types
│   ├── catalog.h                    # stores table/column metadata (schema)
│   └── common.h                     # shared enums, typedefs, constants
│
├── src/
│   ├── lexer/
│   │   └── lexer.cpp                # reads raw SQL string, outputs list of tokens
│   │
│   ├── parser/
│   │   └── parser.cpp               # reads tokens, builds AST
│   │
│   ├── semantic/
│   │   └── semantic_analyzer.cpp    # validates AST (tables exist? columns valid? types match?)
│   │
│   ├── planner/
│   │   └── query_planner.cpp        # converts AST into a logical query plan
│   │
│   ├── optimizer/
│   │   ├── optimizer.cpp                  # orchestrates all optimization rules
│   │   ├── rule_predicate_pushdown.cpp    # pushes WHERE filter closer to data source
│   │   └── rule_projection_pruning.cpp    # drops unused columns early to save memory
│   │
│   ├── executor/
│   │   ├── executor.cpp             # orchestrates execution of the query plan
│   │   ├── scan_node.cpp            # reads rows from storage (full table scan)
│   │   ├── filter_node.cpp          # applies WHERE condition on rows
│   │   └── sort_node.cpp            # handles ORDER BY and GROUP BY sorting
│   │
│   └── storage/
│       ├── table.cpp                # in-memory table representation
│       └── csv_reader.cpp           # reads CSV files as table data source
│
└── tests/
    ├── test_lexer.cpp
    ├── test_parser.cpp
    └── test_executor.cpp
```

## SQL Support

**Currently supported:**
- `SELECT` with multiple columns
- `FROM` with a single table
- `WHERE` with comparison operators (`>`, `<`, `=`, `!=`, `>=`, `<=`)
- `ORDER BY`
- `GROUP BY`

**Planned:**
- Aggregate functions (`COUNT`, `SUM`, `AVG`)
- Subqueries
- `JOIN`

## Build

```bash
mkdir build && cd build
cmake ..
make
./sql-engine
```

## Stage Progress

### Stage 1 — Lexer / Tokenizer ✅

Implemented in `include/token.h` and `src/lexer/lexer.cpp`.

- Defined `TokenType` enum covering all SQL token categories: keywords (`SELECT`, `FROM`, `WHERE`, `ORDER`, `BY`, `GROUP`), identifiers, operators (`>`, `<`, `=`, `!=`, `>=`, `<=`), literals (`INTEGER`, `FLOAT`, `STRING`), symbols (`,`, `;`, `(`, `)`), and `END_OF_FILE`
- Defined `Token` struct holding a `TokenType` and a `std::string` value
- Lexer walks the raw SQL string character by character using maximal munch — reads full words, full numbers, and multi-character operators before deciding the token type
- Handles: keyword vs identifier distinction, integer and float literals, single-quoted strings, all comparison operators, and symbols
- Appends `END_OF_FILE` token at the end so the parser knows when to stop

## Bug Found in Stage 1 — Unterminated String Literal

While testing the lexer, a silent failure was identified in string literal handling.

**The input:**
```sql
SELECT name FROM users WHERE city = 'Delhi
```

**What happened:**

The lexer produced a valid-looking `STRING` token with value `Delhi` and moved on. No error, no crash. The missing closing quote was completely ignored.

**Why this is dangerous:**

A silent failure here poisons everything downstream. The parser receives what looks like a well-formed token stream and has no way to know the original query was broken. Errors like this should be caught at the earliest possible stage — the lexer — not discovered three layers deep in the parser or executor.

**The fix:**

After the string scanning loop, explicitly check if the loop exited because it ran out of input rather than finding a closing quote:

```cpp
else if (input[i] == '\'') {
    std::string str;
    i++; // skip opening quote
    while (i < input.length() && input[i] != '\'') {
        str.push_back(input[i]);
        i++;
    }

    // FIX: detect unterminated string
    if (i >= input.length()) {
        throw std::runtime_error("Lexer error: unterminated string literal");
    }

    tokens.push_back({TokenType::STRING, str});
}
```

**The principle:**

> Fail loudly and early. A lexer that silently swallows malformed input makes debugging hell for every stage that comes after it.

This is demonstrated live in `tests/test_lexer.cpp`.

- [x] Stage 1 — Lexer / Tokenizer
- [ ] Stage 2 — Parser (AST Builder)
- [ ] Stage 3 — Semantic Analyzer
- [ ] Stage 4 — Query Planner
- [ ] Stage 5 — Optimizer
- [ ] Stage 6 — Execution Engine
- [ ] Stage 7 — Storage Layer
- [ ] Stage 8 — REPL Interface

---

Built by [zoolpher](https://github.com/zoolpher) — B.Tech CS, systems engineering track.
