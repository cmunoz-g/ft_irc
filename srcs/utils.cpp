#include "IRC.hpp"

int stringToInt(const std::string& str) {
    std::stringstream ss(str);
    int num = 0;
    ss >> num;
    return num;
}
