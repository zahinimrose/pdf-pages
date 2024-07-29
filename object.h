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
    static unordered_map<int, Object>& table;

public:
    virtual Data serialize() const {};
    
};

Object* direct_parse(const Data& data, Idx& i);

class Reference : public Object {
    int ref_no;
};

class Name_object: public Object {
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
        n.name = std::string(tok.begin() + 1, tok.end());

        return n;

    }
    virtual Data serialize() const {
        Data d;
        d.push_back('/');
        d.insert(d.end(), name.begin(), name.end());

        return d;
    };

};

template<> struct std::hash<Name_object> {
    std::size_t operator()(Name_object const& s) const noexcept {
        std::size_t h1 = std::hash<string>{}(s.name);
        return h1;
    }
};

class Array_object: public Object {
    std::vector<Object> list;
};

class Dict_object : public Object {
public:
    std::unordered_map<Name_object, Object*> map;

    static Dict_object parse(const Data& data, Idx& i) {
        Lexer l(data, i);

        auto tok = l.read_next_tok();
        if(!Lexer::equalsString(tok, "<<")) {
            cout << "ERROR: Dict object must start with <<";
            exit(1);
        }
        Dict_object dict;
        tok = l.peek_next_tok();
        while(!Lexer::equalsString(tok, ">>")) {
            Name_object key = Name_object::parse(data, i);
            Object* o = direct_parse(data, i);
            dict.map[key] = o;

            tok = l.peek_next_tok();
        }
        l.read_next_tok();

        return dict;
    }

    Data serialize() const {
        Data d;
        d.push_back('<');
        d.push_back('<');
        d.push_back('\n');
        for(auto& e: map) {
            auto key = e.first.serialize();
            d.insert(d.end(), key.begin(), key.end());
            d.push_back(' ');
            auto value = e.second->serialize();
            d.insert(d.end(), value.begin(), value.end());
            d.push_back('\n');
        }
        d.push_back('>');
        d.push_back('>');

        return d;
    };
};

class Stream_object: public Object {
    Dict_object dict;
    Data data;
};

class String_object : public Object {
public:
    Data str;
    static String_object parse(const Data& data, Idx& i) {
        Lexer l(data, i);
        auto tok = l.read_next_tok();
        if(tok[0] !='(') {
            cout << "Error: String object must start with ( " << (int)data[i] << "\n";
            exit(1);
        }
        bool escape = false;
        int left_parens = 1;
        String_object str_obj;
        str_obj.str.push_back('(');
        while(left_parens > 0) {
            str_obj.str.push_back(data[i]);
            if(!escape) {
                if(data[i] == '(') {
                    left_parens++;
                }
                else if(data[i] == ')') {
                    left_parens--;
                }
                else if(data[i] == '\\') {
                    escape = true;
                }
            }
            else {
                escape = false;
            }
            i++;
        }
        return str_obj;
    }

    Data serialize() const {
        return str;
    }
};

Object* direct_parse(const Data& data, Idx& i) {
        Lexer l(data, i);
        auto tok = l.peek_next_tok();
        if (Lexer::equalsString(tok, "<<")) {
            Dict_object dict = Dict_object::parse(data, i);
            auto ret = new Dict_object;
            *ret = dict;
            return ret;
        }
        else if(isdigit(tok[0])) {
            cout << "Error: Not implemented";
            exit(1);
            // tok = l.read_next_tok();
        }
        else if(Lexer::equalsString(tok, "(")) {
            String_object s = String_object::parse(data, i);
            auto ret = new String_object;
            *ret = s;
            return ret;
        }
}

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

        return obj;
    }