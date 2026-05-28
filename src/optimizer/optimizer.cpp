
#include "../../include/plan.h"

#include <vector>


class Optimizer {
private:
    std::vector<PlanNode> plan;

public:
    Optimizer(std::vector<PlanNode> plan) : plan(plan) {}

    std::vector<PlanNode> optimize() {
        // projection pruning
        std::vector<std::string> projectedColumns;

        // Step 1 - find PROJECT node, grab columns
        for (PlanNode& node : plan) {
            if (node.type == PlanType::PROJECT) {
                projectedColumns = node.columns;
            }
        }

        // Step 2 - push columns into SCAN node
        for (PlanNode& node : plan) {
            if (node.type == PlanType::SCAN) {
                node.columns = projectedColumns;
            }
        }
        return plan;
    }
};