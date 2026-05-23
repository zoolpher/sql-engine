#include "../../include/token.h"
#include <vector>
#include <string>
#include <stdexcept>

class Lexer {
private:
    std::string input;
    int pos;

public:
    Lexer(std::string input) {
        this -> input = input;
        this -> pos = 0;
    }

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;

        for (int i = 0; i < input.length(); i++) {

            if (input[i] == ' ') continue;

            else if (isalpha(input[i])) {
                std::string word;
                while (i < input.length() && isalpha(input[i])) {
                    word.push_back(input[i]);
                    i++;
                }
                i--; // because for loop will i++ again
                // now check if word is a keyword or identifier

                if (word == "SELECT") tokens.push_back({TokenType::SELECT, word});
                else if (word == "FROM") tokens.push_back({TokenType::FROM, word});
                else if (word == "WHERE") tokens.push_back({TokenType::WHERE, word});
                else if (word == "ORDER") tokens.push_back({TokenType::ORDER, word});
                else if (word == "BY") tokens.push_back({TokenType::BY, word});
                else if (word == "GROUP") tokens.push_back({TokenType::GROUP, word});
                else tokens.push_back({TokenType::IDENTIFIER, word});
            }

            else if (isdigit(input[i])) {
                std::string num;
                TokenType type = TokenType::INTEGER;
                while (i < input.length() && (isdigit(input[i]) || input[i] == '.')) {
                    num.push_back(input[i]);
                    if (input[i] == '.') type = TokenType::FLOAT;
                    i++;
                }
                i--; // because for loop will i++ again
                tokens.push_back({type, num});
            }

            else if (input[i] == '>') {
                if (i + 1 < input.length() && input[i + 1] == '=') {
                    tokens.push_back({TokenType::GTE, ">="});
                    i++;
                } else {
                    tokens.push_back({TokenType::GT, ">"});
                }
            }

            else if (input[i] == '<') {
                if (i + 1 < input.length() && input[i + 1] == '=') {
                    tokens.push_back({TokenType::LTE, "<="});
                    i++;
                } else {
                    tokens.push_back({TokenType::LT, "<"});
                }
            }

            else if (input[i] == '=') {
                tokens.push_back({TokenType::EQ, "="});
            }

            else if (input[i] == '!' && i + 1 < input.length() && input[i + 1] == '=') {
                tokens.push_back({TokenType::NEQ, "!="});
                i++;
            }

            else if (input[i] == ',') {
               tokens.push_back({TokenType::COMMA, ","});
            }

            else if (input[i] == ';') {
                tokens.push_back({TokenType::SEMICOLON, ";"});
            }

            else if (input[i] == '(') {
                tokens.push_back({TokenType::LPAREN, "("});
            }

            else if (input[i] == ')') {
                tokens.push_back({TokenType::RPAREN, ")"});
            }

            else if (input[i] == '\'') {
                std::string str;
                i++; // skip opening quote
                while (i < input.length() && input[i] != '\'') {
                    str.push_back(input[i]);
                    i++;
                }
                // FIX BUG: detect unterminated string
                if (i >= input.length()) {
                    throw std::runtime_error("Lexer error: unterminated string literal");
                }
                tokens.push_back({TokenType::STRING, str});
            }

            else {
                // Handle unrecognized characters or throw an error
            }
        }

        tokens.push_back({TokenType::END_OF_FILE, ""});
        return tokens;
    }
};