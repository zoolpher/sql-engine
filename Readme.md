# sql-engine

A SQL query engine built from scratch in C++. No libraries. No shortcuts. Raw systems engineering.

## Follow the Build

I'm documenting everything as I build — code, bugs, decisions, and breakdowns.

- 🎥 YouTube — [Stage 1: Lexer / Tokenizer](https://youtu.be/A9iKONyU0yU?si=O592VJJSCR695r2a)
- 🎥 YouTube — [Stage 2: Parser](https://youtu.be/8oS6S5Kggxc)
- 🎥 YouTube — [Stage 3: Semantic Analyzer](https://youtu.be/geoK6kt073M)
- 🎥 YouTube — [Stage 4: Query Planner](coming soon)
  
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

- [x] Stage 1 — Lexer / Tokenizer
- [x] Stage 2 — Parser (AST Builder)
- [x] Stage 3 — Semantic Analyzer
- [x] Stage 4 — Query Planner
- [ ] Stage 5 — Optimizer
- [ ] Stage 6 — Execution Engine
- [ ] Stage 7 — Storage Layer
- [ ] Stage 8 — REPL Interface


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

---

### Stage 2 — Parser / AST Builder ✅
 
Implemented in `include/ast.h` and `src/parser/parser.cpp`.
 
- Defined a base `ASTNode` struct with a `child` pointer (initialized to `nullptr`) and a virtual destructor to enable safe `dynamic_cast` across the tree
- Defined node types via inheritance from `ASTNode`: `SelectNode` (stores list of column names), `FromNode` (stores table name), `WhereNode` (stores a `ConditionNode`), `OrderByNode`, and `GroupByNode`
- `ConditionNode` is a separate struct holding `left`, `op`, and `right` as strings — represents expressions like `age > 20`
- Parser walks the token stream using `peek()` (look at current token) and `consume()` (read and advance) — same maximal munch principle as the lexer but at the token level
- Builds the AST by connecting nodes via `child` pointers: `SelectNode → WhereNode → FromNode → OrderByNode/GroupByNode`
- Handles optional clauses gracefully — queries without `WHERE` or `FROM` don't crash; missing nodes are detected and tree is connected accordingly
- Guards all inner loops with `END_OF_FILE` checks to prevent out-of-bounds crashes on malformed queries
## Bug Found in Stage 2 — Uninitialized Child Pointer
 
`ASTNode::child` was declared without initialization, leaving it as a garbage pointer. Any traversal of the tree (in the executor or debug printer) could crash or read invalid memory.
 
**The fix:** Initialize at declaration:
 
```cpp
struct ASTNode {
    ASTNode* child = nullptr;
    virtual ~ASTNode() = default;
};
```
 
**The principle:**
 
> Always initialize pointers. An uninitialized pointer is a ticking time bomb — it won't crash immediately, it'll crash three stages later in the hardest place to debug.

---

### Stage 3 — Semantic Analyzer ✅

Implemented in `include/catalog.h` and `src/semantic/semantic_analyzer.cpp`.

- Defined `ColumnSchema` struct holding `col_name` and `col_type`, and `TableSchema` struct holding `table_name` and a vector of `ColumnSchema`
- Defined `Catalog` struct as the in-memory schema registry — stores all tables and exposes `getTable()` for lookup by name
- Semantic analyzer walks the AST in three passes: (1) find and validate the table in `FromNode`, (2) validate all columns in `SelectNode` against the table schema, (3) validate the `WHERE` condition column in `WhereNode`
- Each pass traverses the full AST via `child` pointer chain using `dynamic_cast` to identify node types
- Throws `std::runtime_error` immediately on first violation — unknown table, unknown column in SELECT, or unknown column in WHERE condition

## Bug Found in Stage 3 — Empty WhereNode Always Validated

The parser always attaches a `WhereNode` to the AST, even for queries with no `WHERE` clause. This means `condition.left` is an empty string `""` for queries like `SELECT name FROM users`.

Pass 3 of the semantic analyzer then tries to validate `""` as a column name — and throws an error on a perfectly valid query.

**The fix:** Skip WHERE validation if the condition is empty:

```cpp
if (WhereNode* w = dynamic_cast<WhereNode*>(ast_node)) {
    if (w->condition.left.empty()) {
        ast_node = ast_node->child;
        continue;
    }
    // proceed with validation
}
```

**The principle:**

> Don't validate what isn't there. An empty node is not an error — treating it as one is.

This is demonstrated live in `tests/test_semantic.cpp`.

---

### Stage 4 — Query Planner ✅
 
Implemented in `include/plan.h` and `src/planner/query_planner.cpp`.
 
- Defined `PlanType` enum covering four operation types: `SCAN` (fetch rows), `FILTER` (apply WHERE), `SORT` (apply ORDER BY), `PROJECT` (select columns)
- Defined `PlanNode` struct holding a `PlanType`, table name, sort column, `ConditionNode` for filter, and column list for projection
- Query planner walks the AST via `child` pointer chain using `dynamic_cast` to identify node types — same traversal pattern as the semantic analyzer
- Converts each AST node into a corresponding `PlanNode` and collects them into a flat `std::vector<PlanNode>`
- Uses `std::reverse()` on the collected nodes to convert AST top-down order into correct bottom-up execution order: `SCAN → FILTER → SORT → PROJECT`
## Bug Found in Stage 4 — Wrong Execution Order (SORT before SCAN)
 
The parser was attaching `OrderByNode` to `fromNode->child` instead of between `WhereNode` and `FromNode`. This meant AST traversal encountered `OrderByNode` before `FromNode`, producing the wrong plan order even after `std::reverse()`.
 
**What came out:**
```
Step 1: SORT   | column: name
Step 2: SCAN   | table: users
Step 3: FILTER | condition: age > 20
Step 4: PROJECT| columns: name
```
 
**What should come out:**
```
Step 1: SCAN   | table: users
Step 2: FILTER | condition: age > 20
Step 3: SORT   | column: name
Step 4: PROJECT| columns: name
```
 
**The fix:** Correct the AST node chain in the parser so `OrderByNode` sits between `WhereNode` and `FromNode`:
 
```
SelectNode → WhereNode → OrderByNode → FromNode
```
 
Not:
 
```
SelectNode → WhereNode → FromNode → OrderByNode
```
 
**The principle:**
 
> The planner is only as correct as the AST it reads. Garbage in, garbage out — fix the structure before fixing the plan.
 
This is demonstrated live in `tests/test_planner.cpp`.
 
---



Built by [zoolpher](https://github.com/zoolpher) — B.Tech CS, systems engineering track.
