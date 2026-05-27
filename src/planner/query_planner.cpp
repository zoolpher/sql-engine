
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

        while (ast_node != nullptr) {
            if (FromNode* f = dynamic_cast<FromNode*>(ast_node)) {
                plan_nodes.push_back({PlanType::SCAN, f->table, "", {}, {}});
            } 
            
            else if (WhereNode* w = dynamic_cast<WhereNode*>(ast_node)) {
                plan_nodes.push_back({PlanType::FILTER, "", "", w->condition, {}});
            } 

            else if (OrderByNode* o = dynamic_cast<OrderByNode*>(ast_node)) {
                plan_nodes.push_back({PlanType::SORT, "", o->column, {}, {}});
            } 
            
            else if (SelectNode* s = dynamic_cast<SelectNode*>(ast_node)) {
                plan_nodes.push_back({PlanType::PROJECT, "", "", {}, s->columns});
            }

            ast_node = ast_node->child;
        }
        
        std::reverse(plan_nodes.begin(), plan_nodes.end());
        
        return plan_nodes;
    }

};

