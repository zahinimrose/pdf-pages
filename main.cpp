#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>

#include "object.h"
// #include "lexer.h"

using std::cout;
using std::string;
using std::vector;
using std::unordered_map;


// Data read_prev_token(const Data& data, Idx& end) {
//     int i = end;
//     i--;
//     while(i >= 0 && is_white_space(data[i])) {
//         i--;
//     }
//     if(i < 0) return {};
//     Idx tok_end = i;
//     while(i >= 0 && !is_white_space(data[i])) {
//         i--;
//     }
//     i++;
//     end = i;

//     Data tok;
//     for(; i <= tok_end; i++) {
//         tok.push_back(data[i]);
//     }

//     return tok;
// }

void dump(Data& data) {
    string header = "%PDF-1.4\n";
    data.insert(data.end(), header.begin(), header.end());
    unordered_map<int, int> pos;
    for(auto e: table) {
        pos[e.first] = data.size();
        string obj_num = to_string(e.first);
        data.insert(data.end(), obj_num.begin(), obj_num.end());
        string rest = " 0 obj\n";
        data.insert(data.end(), rest.begin(), rest.end());
        auto obj = e.second->serialize();
        data.insert(data.end(), obj.begin(), obj.end());
        data.push_back('\n');
        string str = "endobj\n";
        data.insert(data.end(), str.begin(), str.end());
    }
    int startxref = data.size();
    string str = "xref\n";
    data.insert(data.end(), str.begin(), str.end());
    str = "0 12\n";
    data.insert(data.end(), str.begin(), str.end());
    str = "0000000000 65535 f\n";
    data.insert(data.end(), str.begin(), str.end());

    str = " 00000 n\n";
    for(int i = 0; i < pos.size();i++) {
        string position = to_string(pos[i + 1]);
        while (position.length() < 10) {
            position.insert(position.begin(), '0');
        }
        data.insert(data.end(), position.begin(), position.end());
        data.insert(data.end(), str.begin(), str.end());
    }

    str = "trailer\n";
    data.insert(data.end(), str.begin(), str.end());
    str = "<</Size 12\n/Root 7 0 R\n/Info 1 0 R>>\nstartxref\n";
    data.insert(data.end(), str.begin(), str.end());
    str = to_string(startxref);
    data.insert(data.end(), str.begin(), str.end());
    str = "\n%%EOF\n";
    data.insert(data.end(), str.begin(), str.end());
}

int main() {
    const std::string file_path = "dump.pdf";
    std::ifstream file_stream(file_path, std::ios::in | std::ios::binary);
    char ch;
    std::vector<char> data;

    while(file_stream) {
        file_stream.get(ch);
        data.push_back(ch);
    }
    file_stream.close();

    Idx i = 0;
    Lexer l(data, i);
    auto tok = l.read_next_tok();
    // print_tok(tok);
    // tok = l.read_next_tok();
    // print_tok(tok);

    parse_indirect_objects(data, i);

    Data out;
    dump(out);

    std::ofstream dump("dump2.pdf", std::ios::out | std::ios::binary);

    for(char ch: out) {
        dump.put(ch);
    }

    cout<< data[15];

}