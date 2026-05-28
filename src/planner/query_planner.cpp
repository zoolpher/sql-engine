
#include <string>
#include <vector>
#include <algorithm>

#include "../../include/ast.h"
#include "../../include/plan.h"



class QueryPlanner { 
private:
    ASTNode* root;
public:

    QueryPlanner(ASTNode* root) : root(root) {}

    std::vector<PlanNode> plan() {
        std::vector<PlanNode> plan_nodes;
        ASTNode* ast_node = root;

        // collect all nodes first
        SelectNode* selectNode = nullptr;
        FromNode* fromNode = nullptr;
        WhereNode* whereNode = nullptr;
        OrderByNode* orderNode = nullptr;
        GroupByNode* groupNode = nullptr;

        while (ast_node != nullptr) {
            if (auto* n = dynamic_cast<SelectNode*>(ast_node)) selectNode = n;
            else if (auto* n = dynamic_cast<FromNode*>(ast_node)) fromNode = n;
            else if (auto* n = dynamic_cast<WhereNode*>(ast_node)) whereNode = n;
            else if (auto* n = dynamic_cast<OrderByNode*>(ast_node)) orderNode = n;
            else if (auto* n = dynamic_cast<GroupByNode*>(ast_node)) groupNode = n;
            ast_node = ast_node->child;
        }

        // build plan in correct execution order
        if (fromNode)   plan_nodes.push_back({PlanType::SCAN,    fromNode->table, "", {}, {}});
        if (whereNode && !whereNode->condition.left.empty())
                        plan_nodes.push_back({PlanType::FILTER,  "", "", whereNode->condition, {}});
        if (orderNode)  plan_nodes.push_back({PlanType::SORT,    "", orderNode->column, {}, {}});
        if (groupNode)  plan_nodes.push_back({PlanType::SORT,    "", groupNode->column, {}, {}});
        if (selectNode) plan_nodes.push_back({PlanType::PROJECT, "", "", {}, selectNode->columns});

        return plan_nodes;
    }

};

