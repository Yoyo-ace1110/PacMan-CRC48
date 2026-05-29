#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <string_view>

const std::map<std::string_view, char> MorseTable = {
    {".-",    'A'}, 
    {"-...",  'B'}, 
    {"-.-.",  'C'}, 
    {"-..",   'D'},
    {".",     'E'}, 
    {"..-.",  'F'}, 
    {"--.",   'G'}, 
    {"....",  'H'},
    {"..",    'I'}, 
    {".---",  'J'}, 
    {"-.-",   'K'}, 
    {".-..",  'L'},
    {"--",    'M'}, 
    {"-.",    'N'}, 
    {"---",   'O'}, 
    {".--.",  'P'},
    {"--.-",  'Q'}, 
    {".-.",   'R'}, 
    {"...",   'S'}, 
    {"-",     'T'},
    {"..-",   'U'}, 
    {"...-",  'V'}, 
    {".--",   'W'}, 
    {"-..-",  'X'},
    {"-.--",  'Y'}, 
    {"--..",  'Z'}
};

std::vector<std::string> split(std::string_view str, char separator) {
    std::vector<std::string> result = {};
    if (str.size() == 0) return result;
    // remove spaces
    while (str.size() > 0 && str.front() == separator) {
        str.remove_prefix(1);
    }
    while (str.size() > 0 && str.back()  == separator) {
        str.remove_suffix(1);
    }
    // split by space
    std::string current = "";
    while (str.size() > 0) {
        // split space
        if (str.front() == separator) {
            if (current == "") continue;
            result.push_back(current);
            current.clear();
        } else {
            // normal char
            current += str.front();
        }
        str.remove_prefix(1);
    }
    // adding the last slice to result
    if (current != "") result.push_back(current);
    return result;
}

std::string decode_morse_code(std::vector<std::string> slices) {
    std::string result = "";
    for (const std::string& slice_str : slices) {
        std::string_view slice = slice_str;
        // check if slice exist
        if (!MorseTable.contains(slice)) {
            std::cout << "Error: Invalid input of Morse code\n";
            std::cout << "Unexpected slice: " << std::string(slice) << '\n';
        }
        // append the letter to result
        result += MorseTable.at(slice);
    }
    return result;
}

int main() {
    while (true) {
        // input
        std::string input_str = "";
        std::cout << "Input any Morse code: ";
        std::getline(std::cin, input_str);
        // split
        std::string_view temp = input_str;
        std::vector<std::string> slices;
        slices = split(temp, ' ');
        // convert and cout
        std::cout << decode_morse_code(slices) << std::endl;
    }
    return 0;
}
