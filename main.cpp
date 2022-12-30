#include <iostream>

#include "reader.hpp"

int main() {
    Reader reader("..\\in.lc3");

    for (auto& inst : reader.readFile()) {
        std::cout << "DEBUG: " << inst.label << " " << static_cast<int>(inst.type) << " ";
        for (auto op : inst.operands) {
            std::cout << op << " ";
        }
        std::cout << std::endl;
    }
    return 0;
}