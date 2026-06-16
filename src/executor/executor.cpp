
#include "../../include/plan.h"
#include "../../include/table.h"
#include "scan_node.cpp"
#include "filter_node.cpp"
#include "sort_node.cpp"

#include <vector>
#include <iostream>

class Executor {
private:
    std::vector<PlanNode> plan;
public:
    Executor(std::vector<PlanNode> plan) : plan(plan) {}

    Table execute() {
        Table result;

        for (PlanNode& node : plan) {
            if (node.type == PlanType::SCAN) {
                ScanNode scan(node);
                result = scan.execute();
            }
            else if (node.type == PlanType::FILTER) {
                FilterNode filter(node);
                result = filter.execute(result);
            }
            else if (node.type == PlanType::SORT) {
                SortNode sort(node);
                result = sort.execute(result);
            }
            else if (node.type == PlanType::PROJECT) {
                // find indices of requested columns
                Table projected;
                for (const std::string& col : node.columns) {
                    for (int i = 0; i < result.columns.size(); i++) {
                        if (result.columns[i] == col) {
                            projected.columns.push_back(col);
                            for (auto& row : result.rows) {
                                if (projected.rows.size() <= &row - &result.rows[0])
                                    projected.rows.push_back({});
                                projected.rows.back().push_back(row[i]);
                            }
                            break;
                        }
                    }
                }
                result = projected;
            }
        }

        return result;
    }

    void print(Table& table) {
        // print column headers
        for (const std::string& col : table.columns) {
            std::cout << col << "\t";
        }
        std::cout << "\n";

        // print rows
        for (auto& row : table.rows) {
            for (const std::string& val : row) {
                std::cout << val << "\t";
            }
            std::cout << "\n";
        }
    }
};