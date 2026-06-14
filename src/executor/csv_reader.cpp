
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

        // first line = column names
        if (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string col;
            while (std::getline(ss, col, ',')) {
                table.columns.push_back(col);
            }
        }

        // remaining lines = rows
        while (std::getline(file, line)) {
            std::vector<std::string> row;
            std::stringstream ss(line);
            std::string val;
            while (std::getline(ss, val, ',')) {
                row.push_back(val);
            }
            table.rows.push_back(row);
        }

        return table;
    }
};