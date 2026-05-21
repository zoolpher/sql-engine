*sql-engine*

sql-engine/
├── main.cpp                         # entry point, wires all components together
├── CMakeLists.txt                   # build system, tells compiler how to compile project
│
├── include/                         # shared headers accessible by ALL components
│   ├── token.h                      # defines Token struct and TokenType enum
│   ├── ast.h                        # defines all AST node types (SelectNode, WhereNode etc)
│   ├── catalog.h                    # stores table/column metadata (names, types, schema)
│   └── common.h                     # shared enums, typedefs, constants used everywhere
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
│       ├── table.cpp                # in-memory table representation, holds rows/columns
│       └── csv_reader.cpp           # reads CSV files as table data source
│
└── tests/
    ├── test_lexer.cpp               # unit tests for lexer
    ├── test_parser.cpp              # unit tests for parser
    └── test_executor.cpp            # unit tests for executor