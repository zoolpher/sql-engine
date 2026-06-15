
#include "../../include/table.h"

#include "../../include/plan.h"
#include "../storage/csv_reader.cpp"
#include <vector>
#include <string>

class ScanNode {
private:
    PlanNode plan;
public:
    ScanNode(PlanNode plan) : plan(plan) {}

    Table execute() {
        CSVReader reader("data/" + plan.table + ".csv");
        Table table = reader.read();

        // projection pruning — only keep requested columns
        if (!plan.columns.empty()) {
            // find indices of requested columns
            std::vector<int> indices;
            for (const std::string& col : plan.columns) {
                for (int i = 0; i < table.columns.size(); i++) {
                    if (table.columns[i] == col) {
                        indices.push_back(i);
                        break;
                    }
                }
            }

            // rebuild table with only requested columns
            Table pruned;
            for (int idx : indices) pruned.columns.push_back(table.columns[idx]);
            for (auto& row : table.rows) {
                std::vector<std::string> newRow;
                for (int idx : indices) newRow.push_back(row[idx]);
                pruned.rows.push_back(newRow);
            }
            return pruned;
        }

        return table;
    }
};