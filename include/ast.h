#pragma once

#include <string>
#include <vector>

struct ASTNode {
    ASTNode* child = nullptr;  // written once, shared by all
    virtual ~ASTNode() = default;
};

struct SelectNode : public ASTNode {
    std::vector<std::string> columns;  // only what's unique to SELECT
};

struct FromNode : public ASTNode {
    std::string table;  // only what's unique to FROM
};

struct ConditionNode {  // age >= 20
    std::string left;        // age
    std::string op;          // >=
    std::string right;       // 20
};

struct WhereNode : public ASTNode {
    ConditionNode condition;  // only what's unique to WHERE
};

struct OrderByNode : public ASTNode {
    std::string column;  // only what's unique to ORDER BY
};

struct GroupByNode : public ASTNode {
    std::string column;  // only what's unique to GROUP BY
};

