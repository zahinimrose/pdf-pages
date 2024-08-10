#include <iostream>
#include <fstream>
#include "object.h"

class Pdf {
private:
    Data pdf_data;
    unordered_map<int, Object*> table;
public:
    Dict_object trailer;

    Object* indirect_parse(const Data& data, Idx& i) {
        Lexer l(data, i);
        auto tok = l.read_next_tok();
        int obj_num = stoi(Lexer::toString(tok));
        tok = l.read_next_tok();
        tok = l.read_next_tok();
        if(!Lexer::equalsString(tok, "obj")) {
            cout << "ERROR: Indirect reference must have obj";
        }
        auto obj =  direct_parse(data, i);
        tok = l.read_next_tok();
        if(!Lexer::equalsString(tok, "endobj")) {
            cout << "ERROR: Indirect reference must end with endobj";
        }

        table[obj_num] = obj;
        cout << "Parsed obj num " << obj_num << std::endl;
        return obj;
    }

    void parse_indirect_objects(const Data& data, Idx& i) {
    Lexer l(data, i);
    auto tok = l.peek_next_tok();
    while(!Lexer::equalsString(tok, "xref")) {
        indirect_parse(data, i);
        tok = l.peek_next_tok();
    }
}

public:
    Pdf(string file_path) {
        std::ifstream file_stream(file_path, std::ios::in | std::ios::binary);
        char ch;

        while(file_stream) {
            file_stream.get(ch);
            pdf_data.push_back(ch);
        }
        file_stream.close();

        Idx i = 0;
        Lexer l(pdf_data, i);
        l.read_next_tok();
        l.read_next_tok();
        parse_indirect_objects(pdf_data, i);
        Lexer n(pdf_data, i);
        auto tok = n.read_next_tok();
        while(!Lexer::equalsString(tok, "trailer")) {
            tok = n.read_next_tok();
        }

        trailer = Dict_object::parse(pdf_data, i);
    }
};