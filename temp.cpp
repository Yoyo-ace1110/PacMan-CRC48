#include <map>
#include <vector>
#include <string>
#include <utility>
#include <iostream>

using Str = char const [6];

std::map<Str, char> Table = {
    {".-\0   ", 'A'}, 
    {"-...\0 ", 'B'}, 
};

std::vector<Str> split() {
    
}

int main() {
    std::string pool = "";
    std::cout << "input:";
    std::cin >> pool;

    return 0;
}
