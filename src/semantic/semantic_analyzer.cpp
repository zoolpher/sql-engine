// src/semantic/semantic_analyzer.cpp

#include "../../include/ast.h"
#include "../../include/catalog.h"

#include <stdexcept>

class SemanticAnalyzer {
private:
    ASTNode* root;
    Catalog& catalog;

public:
    SemanticAnalyzer(ASTNode* root, Catalog& catalog) : root(root), catalog(catalog) {}

    void analyze() {
        ASTNode* ast_node = root;
        TableSchema* table = nullptr;

        // Pass 1 — find and validate table
        while (ast_node != nullptr) {
            if (FromNode* f = dynamic_cast<FromNode*>(ast_node)) {
                table = catalog.getTable(f->table);
                if (table == nullptr) {
                    throw std::runtime_error("Error: table '" + f->table + "' does not exist");
                }
            }
            ast_node = ast_node->child;
        }

        // Pass 2 — validate columns
        ast_node = root;
        while (ast_node != nullptr) {
            if (SelectNode* s = dynamic_cast<SelectNode*>(ast_node)) {
                for (const std::string& col : s->columns) {
                    bool found = false;
                    for (const ColumnSchema& c : table->columns) {
                        if (c.col_name == col) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        throw std::runtime_error("Error: column '" + col + "' does not exist in table '" + table->table_name + "'");
                    }
                }
            }
            ast_node = ast_node->child;
        }

        // Pass 3 — validate WHERE condition
        ast_node = root;
        while (ast_node != nullptr) {
            if (WhereNode* w = dynamic_cast<WhereNode*>(ast_node)) {
                
                // BUG: if no WHERE clause, condition.left will be empty string
                // we should skip validation in that case
                if (w->condition.left.empty()) {
                    ast_node = ast_node->child;
                    continue;
                }

                const std::string& col = w->condition.left;
                bool found = false;
                for (const ColumnSchema& c : table->columns) {
                    if (c.col_name == col) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    throw std::runtime_error("Error: column '" + col + "' does not exist in table '" + table->table_name + "'");
                }
            }
            ast_node = ast_node->child;
        }
    }
};