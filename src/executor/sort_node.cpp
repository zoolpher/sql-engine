
#include "../../include/table.h"
#include "../../include/plan.h"

#include <vector>
#include <string>
#include <algorithm>

class SortNode {
private:
    PlanNode plan;
public:
    SortNode(PlanNode plan) : plan(plan) {}

    Table execute(Table& input) {
        // find index of sort column
        int colIndex = -1;
        for (int i = 0; i < input.columns.size(); i++) {
            if (input.columns[i] == plan.column) {
                colIndex = i;
                break;
            }
        }

        if (colIndex == -1) return input;

        Table result = input;
        std::sort(result.rows.begin(), result.rows.end(),
            [colIndex](const std::vector<std::string>& a, const std::vector<std::string>& b) {
                return a[colIndex] < b[colIndex];
            });

        return result;
    }
};