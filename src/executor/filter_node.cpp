
#include "../../include/table.h"

#include "../../include/plan.h"
#include <vector>
#include <string>
#include <stdexcept>

class FilterNode {
private:
    PlanNode plan;
public:
    FilterNode(PlanNode plan) : plan(plan) {}

    Table execute(Table& input) {
        Table result;
        result.columns = input.columns;

        // find index of condition column
        int colIndex = -1;
        for (int i = 0; i < input.columns.size(); i++) {
            if (input.columns[i] == plan.condition.left) {
                colIndex = i;
                break;
            }
        }

        if (colIndex == -1) throw std::runtime_error("Column not found: " + plan.condition.left);

        for (auto& row : input.rows) {
            std::string val = row[colIndex];
            std::string op = plan.condition.op;
            std::string right = plan.condition.right;

            bool pass = false;
            try {
                float lhs = std::stof(val);
                float rhs = std::stof(right);
                if (op == ">")  pass = lhs > rhs;
                if (op == "<")  pass = lhs < rhs;
                if (op == ">=") pass = lhs >= rhs;
                if (op == "<=") pass = lhs <= rhs;
                if (op == "=")  pass = lhs == rhs;
                if (op == "!=") pass = lhs != rhs;
            } catch (...) {
                // string comparison
                if (op == "=")  pass = val == right;
                if (op == "!=") pass = val != right;
            }

            if (pass) result.rows.push_back(row);
        }

        return result;
    }
};