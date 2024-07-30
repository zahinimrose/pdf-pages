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


void print_tok(Data tok) {
    for (char ch: tok) {
        cout << ch;
    }
    cout << "\n";
}

class Object {
    bool indirect;
    static unordered_map<int, Object>& table;

public:
    virtual Data serialize() const {};
    
};

unordered_map<int, Object*> table;
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
public:
    Dict_object dict;
    Data data;

    static Stream_object parse(const Data& data, Idx& i, Dict_object dict) {
        Lexer l(data, i);
        auto tok = l.read_next_tok();
        if(!Lexer::equalsString(tok, "stream")) {
            std::cout << "Cannot parse non streams" << std::endl;
            exit(1);
        }

        Name_object n;
        n.name = "Length";
        if (!dict.map.count(n)) {
            std::cout << "Stream dict must have Length Field" <<std::endl;
        }
        int stream_length = stoi(Lexer::toString(dict.map[n]->serialize()));
        cout << "Length: " << stream_length << std::endl;

        Stream_object obj;
        obj.dict = dict;

        if(data[i] == 10) {
            i++;
        }
        else if(data[i] == 13 && data[i + 1] == 10) {
            i+=2;
        }
        else {
            cout << "ERROR: Invalid EOF after stream keyword";
            exit(1);
        }
        for(int j = 0; j < stream_length; j++) {
            obj.data.push_back(data[i]);
            i++;
        }

        cout<< obj.data[0] << "\n";
        Lexer m(data, i);
        tok = m.read_next_tok();

        if(!Lexer::equalsString(tok, "endstream")) {
            std::cout << "Stream data must end with endstream" << std::endl;
            exit(1);
        }

        return obj;

    }

    Data serialize() const {
        auto d = dict.serialize();
        d.push_back(' ');

        string str = "stream";
        d.push_back(' ');
        d.insert(d.end(), str.begin(), str.end());
        d.push_back(10);
        d.insert(d.end(), data.begin(), data.end());
        d.push_back(10);
        str = "endstream";
        d.insert(d.end(), str.begin(), str.end());
        d.push_back(10);

        return d;
    }
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

class Num_object : public Object {
public:
    Data num;
    static Num_object parse(const Data& data, Idx& i) {
        Lexer l(data, i);
        auto tok = l.read_next_tok();
        if(!isdigit(tok[0])) {
            cout << "Not a numeric object" << std::endl;
            exit(1);
        }

        Num_object num_obj;
        num_obj.num = tok;
        return num_obj;
    }

    Data serialize() const {
        
        return num;
    }
};

Object* direct_parse(const Data& data, Idx& i) {
        Lexer l(data, i);
        auto tok = l.peek_next_tok();
        if (Lexer::equalsString(tok, "<<")) {
            Dict_object dict = Dict_object::parse(data, i);
            tok = l.peek_next_tok();

            if(Lexer::equalsString(tok, "stream")) {
                

                auto stream = Stream_object::parse(data, i, dict);
                auto ret = new Stream_object;
                *ret = stream;

                return ret;
            }


            auto ret = new Dict_object;
            *ret = dict;
            return ret;
        }
        else if(isdigit(tok[0])) {
            auto next = l.peek_next_next_tok();
            if(isdigit(next[0])) {
                cout << "Parsing references not implemented";
                exit(1);
            }
            else {
                Num_object num = Num_object::parse(data, i);
                auto ret = new Num_object;
                *ret = num;

                return ret;
            }
        }
        else if(Lexer::equalsString(tok, "(")) {
            String_object s = String_object::parse(data, i);
            auto ret = new String_object;
            *ret = s;
            return ret;
        }
        else if(tok[0] == '/') {
            Name_object s = Name_object::parse(data, i);
            auto ret = new Name_object;
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

        table[obj_num] = obj;
        cout << "Parsed obj num " << obj_num << std::endl;
        return obj;
}

void parse_indirect_objects(const Data& data, Idx& i) {
    Lexer l(data, i);
    auto tok = l.peek_next_tok();
    while(!Lexer::equalsString(tok, "xref")) {
        indirect_parse(data, i);
    }
}