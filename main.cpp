#include <iostream>
#include <fstream>
#include <vector>

const std::string file_path = "hello.pdf";

int main() {
    std::ifstream file_stream(file_path, std::ios::in | std::ios::binary);
    char ch;
    std::vector<char> data;

    while(file_stream) {
        file_stream.get(ch);
        data.push_back(ch);
    }

    std::cout << data[10761];
}