
#pragma once

#include <string>
#include <vector>

struct Table {
    std::vector<std::string> columns;              // ["name", "age", "city"]
    std::vector<std::vector<std::string>> rows;    // all the data
};