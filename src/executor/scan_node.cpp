
#include "../../include/table.h"
#include "../storage/csv_reader.cpp"
#include "../../include/plan.h"

#include <vector>
#include <string>

class ScanNode {
private:
    PlanNode plan;
public:
    ScanNode(PlanNode plan) : plan(plan) {}

    Table execute() {
        CSVReader reader("data/" + plan.table + ".csv");  

        return reader.read();
    }
};