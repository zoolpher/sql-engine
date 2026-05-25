// src/catalog/catalog.cpp

#include "../../include/catalog.h"

TableSchema* Catalog::getTable(std::string name) {
    for (TableSchema &table : tables) {
        if (table.table_name == name) {
            return &table;
        }
    }
    printf("[ERROR] Table '%s' not found in catalog\n", name.c_str());
    return nullptr;
}