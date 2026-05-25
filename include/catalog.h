#pragma once

#include <string>
#include <vector>

// here we define schema of tables and columns

struct ColumnSchema {
    std::string col_name;
    std::string col_type;  // "INT", "STRING", etc.
};

struct TableSchema {
    std::string table_name;
    std::vector<ColumnSchema> columns;
};

struct Catalog {
    std::vector<TableSchema> tables;
    
    TableSchema* getTable(std::string name);
};