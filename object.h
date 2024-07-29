#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

#include "lexer.h"

typedef std::vector<char> Data;
typedef int Idx;

const char white_space[6] = {0, 9, 10, 12, 13, 32};

bool is_white_space(char ch) {
    for(int i = 0; i < 6; i++) {
        if(ch == white_space[i]) {
            return true;
        }
    }
    return false;
}

inline void consume_white_space(const Data& data, Idx& i) {
    while(is_white_space(data[i])) i++;
}



class Object {
    bool indirect;
};

class Data_object : public Object {
public:
    Data_object() = default;
    Data data;
    Data_object(Data data) : data(data) {};
};

class Name_object: public Data_object {
public:
    Name_object() = default;
    std::string name;

    bool operator==(const Name_object &other) const { 
        return name == other.name;
    }

    static Name_object parse(const Data& data, Idx& i) {
        Lexer l(data, i);
        auto tok = l.read_next_tok();
        if(tok[0] != '/') {
            cout<< "ERROR: Name_object must start with /";
            exit(1);
        }
        Name_object n;
        n.data = tok;
        n.name = std::string(tok.begin() + 1, tok.end());

        return n;

    }

};

template<> struct std::hash<Name_object> {
    std::size_t operator()(Name_object const& s) const noexcept {
        std::size_t h1 = std::hash<string>{}(s.name);
        return h1;
    }
};

class Ref_object: public Object {};

class Array_object: public Ref_object {
    std::vector<Object> list;
};

class Dict_object : public Ref_object {
public:
    std::unordered_map<Name_object, Object> map;

    static Dict_object parse(const Data& data, Idx& i) {
        Lexer l(data, i);

        auto tok = l.read_next_tok();
        if(!Lexer::equalsString(tok, "<<")) {
            cout << "ERROR: Dict object must start with <<";
            exit(1);
        }
        Dict_object dict;
        tok = l.read_next_tok();
        while(!Lexer::equalsString(tok, ">>")) {
            auto key = Name_object::parse(data, i);

        }

        
    }
};

class Stream_object: public Ref_object {
    Dict_object dict;
    Data data;
};