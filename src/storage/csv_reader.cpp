
#include "../../include/table.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class CSVReader {
private:
    std::string filepath;
public:
    CSVReader(std::string filepath) : filepath(filepath) {}

    Table read() {
        Table table;
        std::ifstream file(filepath);
        std::string line;

        if (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string col;
            while (std::getline(ss, col, ',')) {
                col.erase(0, col.find_first_not_of(" \t\r\n"));
                col.erase(col.find_last_not_of(" \t\r\n") + 1);
                table.columns.push_back(col);
            }
        }

        while (std::getline(file, line)) {
            std::vector<std::string> row;
            std::stringstream ss(line);
            std::string val;
            while (std::getline(ss, val, ',')) {
                val.erase(0, val.find_first_not_of(" \t\r\n"));
                val.erase(val.find_last_not_of(" \t\r\n") + 1);
                row.push_back(val);
            }
            table.rows.push_back(row);
        }

        return table;
    }

    std::vector<std::string> readHeaders() {
        std::ifstream file(filepath);
        std::string line;
        std::vector<std::string> headers;

        if (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string col;
            while (std::getline(ss, col, ',')) {
                col.erase(0, col.find_first_not_of(" \t\r\n"));
                col.erase(col.find_last_not_of(" \t\r\n") + 1);
                headers.push_back(col);
            }
        }

        return headers;
    }
};