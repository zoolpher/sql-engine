
#pragma once
#include "ast.h"
#include <string>
#include <vector>

enum class PlanType {
    SCAN,     // read all rows from a table
    FILTER,   // apply WHERE condition
    SORT,     // apply ORDER BY
    PROJECT   // pick only the requested columns
};

struct PlanNode {
    PlanType type;       // what operation?
    std::string table;   // for SCAN — which table?
    std::string column;  // for SORT — which column?
    ConditionNode condition;  // for FILTER — what condition?
    std::vector<std::string> columns;  // for PROJECT — which columns?
};