// src/parser/parser.cpp

#include "../../include/ast.h"
#include "../../include/token.h"
#include <vector>

class Parser {
private:
    std::vector<Token> tokens;
    int pos;

public:
    Parser(std::vector<Token> tokens) {
        this->tokens = tokens;
        this->pos = 0;
    }

    Token peek() {
        return tokens[pos];
    }

    Token consume() {
        return tokens[pos++];
    }

    ASTNode* parse() {
        SelectNode* selectNode = new SelectNode();
        FromNode* fromNode = new FromNode();
        WhereNode* whereNode = new WhereNode();

        while (peek().type != TokenType::END_OF_FILE) {

            if (peek().type == TokenType::SELECT) {
                consume();
                while (peek().type != TokenType::FROM &&
                       peek().type != TokenType::END_OF_FILE) {
                    Token t = consume();
                    if (t.type == TokenType::IDENTIFIER) {
                        selectNode->columns.push_back(t.value);
                    }
                }
            }

            if (peek().type == TokenType::FROM) {
                consume();
                while (peek().type != TokenType::WHERE &&
                       peek().type != TokenType::ORDER &&
                       peek().type != TokenType::GROUP &&
                       peek().type != TokenType::END_OF_FILE) {
                    Token t = consume();
                    if (t.type == TokenType::IDENTIFIER) {
                        fromNode->table = t.value;
                    }
                }
            }

            if (peek().type == TokenType::WHERE) {
                consume();
                ConditionNode condition;
                condition.left  = consume().value;
                condition.op    = consume().value;
                condition.right = consume().value;
                whereNode->condition = condition;
            }

            if (peek().type == TokenType::ORDER) {
                consume();
                consume();
                OrderByNode* orderNode = new OrderByNode();
                orderNode->column = consume().value;
                fromNode->child = orderNode;
            }

            if (peek().type == TokenType::GROUP) {
                consume();
                consume();
                GroupByNode* groupNode = new GroupByNode();
                groupNode->column = consume().value;
                fromNode->child = groupNode;
            }
        }

        // [BUG FIX] : missing WHERE clause
        // Handle case where there's no WHERE clause
        if (!whereNode->condition.left.empty()) {
            selectNode->child = whereNode;
            whereNode->child = fromNode;
        } 
        
        // [BUG FIX] : missing FROM clause
        // Handle case where there's no FROM clause
        else if (!fromNode->table.empty()) {
            selectNode->child = fromNode;
        } else {
            selectNode->child = nullptr;
        }

        return selectNode;
    }
};


