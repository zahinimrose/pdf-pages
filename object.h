#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <memory>

#include "lexer.h"

class Object {
public:
    virtual Data serialize() const  = 0;
    virtual Data write(Data& obj_buffer, Idx& cur_obj_no, unordered_map<int, int>& obj_loc, shared_ptr<unordered_map<int, shared_ptr<Object>>> table) const = 0;
    
};

shared_ptr<Object> direct_parse(const Data& data, Idx& i);

class Reference : public Object {
public:
    int ref_no;
    int gen_no;

    static shared_ptr<Reference> parse(const Data& data, Idx& i) {
        Lexer l(data, i);

        auto tok = l.read_next_tok();
        Reference ref;
        ref.ref_no = stoi(Lexer::toString(tok));

        tok = l.read_next_tok();
        ref.gen_no = stoi(Lexer::toString(tok));
        tok = l.read_next_tok();

        if(!Lexer::equalsString(tok, "R")) {
            cout << "ERROR: Reference must End with R. Instead ends with " << Lexer::toString(tok);
            exit(1);
        }

        return make_shared<Reference>(ref);
    }

    Data serialize() const {
        auto s = to_string(ref_no);
        Data d;
        d.insert(d.end(), s.begin(), s.end());
        d.push_back(' ');
        s = to_string(gen_no);
        d.insert(d.end(), s.begin(), s.end());
        d.push_back(' ');
        d.push_back('R');

        return d;
    }
    shared_ptr<Object> deref(shared_ptr<unordered_map<int, shared_ptr<Object>>> table) const {
        return (*table)[ref_no];
    }
    Data write(Data& obj_buffer, Idx& cur_obj_no, unordered_map<int, int>& obj_loc, shared_ptr<unordered_map<int, shared_ptr<Object>>> table) const {
        Data out = deref(table)->write(obj_buffer, cur_obj_no, obj_loc, table);

        int pos = obj_buffer.size();
        obj_loc[cur_obj_no] = pos;
        append(obj_buffer, to_string(cur_obj_no));
        append(obj_buffer, " 0 obj\n");
        append(obj_buffer, out);
        append(obj_buffer, "\nendobj\n");

        Data d;
        append(d, to_string(cur_obj_no));
        append(d, " 0 R");

        cur_obj_no++;

        return d;
    }

};

class Name_object: public Object {
public:
    Name_object() = default;
    Name_object(string name): name(name) {}
    std::string name;

    bool operator==(const Name_object &other) const { 
        return name == other.name;
    }

    static shared_ptr<Name_object> parse(const Data& data, Idx& i) {
        Lexer l(data, i);
        auto tok = l.read_next_tok();
        if(!Lexer::equalsString(tok, "/")) {
            cout<< "ERROR: Name_object must start with / " << i;
            exit(1);
        }
        tok = l.read_next_tok();

        return make_shared<Name_object>(Name_object(string(tok.begin(), tok.end())));

    }
    Data serialize() const {
        Data d;
        d.push_back('/');
        d.insert(d.end(), name.begin(), name.end());

        return d;
    };

    Data write(Data& obj_buffer, Idx& cur_obj_no, unordered_map<int, int>& obj_loc, shared_ptr<unordered_map<int, shared_ptr<Object>>> table) const {
        return serialize();
    };

    int get_num() {
        return stoi(name);
    }
};

template<> struct std::hash<Name_object> {
    std::size_t operator()(Name_object const& s) const noexcept {
        std::size_t h1 = std::hash<string>{}(s.name);
        return h1;
    }
};

class Array_object: public Object {
public:
    std::vector<shared_ptr<Object>> list;
    static shared_ptr<Array_object> parse(const Data& data, Idx& i) {
        Lexer l(data, i);
        auto tok = l.read_next_tok();
        if(!Lexer::equalsString(tok, "[")) {
            cout << "ERROR: Array object must start with <<";
            exit(1);
        }

        Array_object arr;

        tok = l.peek_next_tok();
        while(!Lexer::equalsString(tok, "]")) {
            shared_ptr<Object> o = direct_parse(data, i);
            arr.list.push_back(o);

            tok = l.peek_next_tok();
        }
        l.read_next_tok();

        return make_shared<Array_object>(arr);
    }

    Data serialize() const {

        Data d;
        d.push_back('[');
        d.push_back(' ');
        for(auto& e: list) {
            auto key = e->serialize();
            d.insert(d.end(), key.begin(), key.end());
            d.push_back(' ');
        }
        d.push_back(']');

        return d;
    };

    Data write(Data& obj_buffer, Idx& cur_obj_no, unordered_map<int, int>& obj_loc, shared_ptr<unordered_map<int, shared_ptr<Object>>> table) const {
        Data d;
        d.push_back('[');
        d.push_back(' ');
        for(auto& e: list) {
            auto key = e->write(obj_buffer, cur_obj_no,obj_loc, table);
            d.insert(d.end(), key.begin(), key.end());
            d.push_back(' ');
        }
        d.push_back(']');

        return d;
    };
};

class Dict_object : public Object {
public:
    std::unordered_map<string, shared_ptr<Object>> map;

    static shared_ptr<Dict_object> parse(const Data& data, Idx& i) {
        Lexer l(data, i);

        auto tok = l.read_next_tok();
        if(!Lexer::equalsString(tok, "<<")) {
            cout << "ERROR: Dict object must start with <<";
            exit(1);
        }
        Dict_object dict;
        tok = l.peek_next_tok();
        while(!Lexer::equalsString(tok, ">>")) {
            auto key = Name_object::parse(data, i);
            shared_ptr<Object> o = direct_parse(data, i);
            dict.map[key->name] = o;

            tok = l.peek_next_tok();
        }
        l.read_next_tok();

        return make_shared<Dict_object>(dict);
    }

    Data serialize() const {

        Data d;
        d.push_back('<');
        d.push_back('<');
        // d.push_back('\n');
        for(auto& e: map) {
            auto key = "/" + e.first;
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

    Data write(Data& obj_buffer, Idx& cur_obj_no, unordered_map<int, int>& obj_loc, shared_ptr<unordered_map<int, shared_ptr<Object>>> table) const override {
        Data d;
        d.push_back('<');
        d.push_back('<');
        // d.push_back('\n');
        for(auto& e: map) {
            // auto key = e.first.write(obj_buffer, cur_obj_no, obj_loc, table);
            string key = "/" + e.first;
            d.insert(d.end(), key.begin(), key.end());
            d.push_back(' ');
            auto value = e.second->write(obj_buffer, cur_obj_no,obj_loc, table);
            d.insert(d.end(), value.begin(), value.end());
            d.push_back('\n');
        }
        d.push_back('>');
        d.push_back('>');

        return d;
    }

    shared_ptr<Object> get(string str) {
        if (!map.count(str)) {
            std::cout << "Dict does not have field: " << str <<std::endl;
        }

        return map[str];
    }
    shared_ptr<Object> get_deref(string str, shared_ptr<unordered_map<int, shared_ptr<Object>>> table) {
        auto ref = static_pointer_cast<Reference>(get(str));
        return ref->deref(table);
    }
};

class Stream_object: public Object {
public:
    shared_ptr<Dict_object> dict;
    Data data;

    static shared_ptr<Stream_object> parse(const Data& data, Idx& i, shared_ptr<Dict_object> dict) {
        Lexer l(data, i);
        auto tok = l.read_next_tok();
        if(!Lexer::equalsString(tok, "stream")) {
            std::cout << "Cannot parse non streams" << std::endl;
            exit(1);
        }

        
        int stream_length = stoi(Lexer::toString(dict->get("Length")->serialize()));
        // cout << "Length: " << stream_length << std::endl;

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

        // cout<< obj.data[0] << "\n";
        Lexer m(data, i);
        tok = m.read_next_tok();

        if(!Lexer::equalsString(tok, "endstream")) {
            std::cout << "Stream data must end with endstream" << std::endl;
            exit(1);
        }

        return make_shared<Stream_object>(obj);

    }

    Data serialize() const {
        auto d = dict->serialize();
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

    Data write(Data& obj_buffer, Idx& cur_obj_no, unordered_map<int, int>& obj_loc, shared_ptr<unordered_map<int, shared_ptr<Object>>> table) const {
        auto d = dict->write(obj_buffer, cur_obj_no, obj_loc, table);
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
    static shared_ptr<String_object> parse(const Data& data, Idx& i) {
        Lexer l(data, i);
        auto tok = l.read_next_tok();
        if(tok[0] !='(' && tok[0] !='<') {
            cout << "Error: String object must start with ( or <" << i << "\n";
            exit(1);
        }
        String_object str_obj;

        if(tok[0] == '<') {
            str_obj.str.push_back('<');
            while(data[i] != '>') {
                str_obj.str.push_back(data[i]);
                i++;
            }
            i++;
            str_obj.str.push_back('>');
            return make_shared<String_object>(str_obj);

        }
        bool escape = false;
        int left_parens = 1;
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
        return make_shared<String_object>(str_obj);
    }

    Data serialize() const {
        return str;
    }

    Data write(Data& obj_buffer, Idx& cur_obj_no, unordered_map<int, int>& obj_loc, shared_ptr<unordered_map<int, shared_ptr<Object>>> table) const {
        return str;
    }
};

class Single_object : public Object {
public:
    Data data;
    Single_object() = default;
    Single_object(Data d) : data(d) {}
    Single_object(int num) {
        Data d;
        auto str = to_string(num);
        d.insert(d.end(), str.begin(), str.end());

        data = d;
    }
    static shared_ptr<Single_object> parse(const Data& data, Idx& i) {
        Lexer l(data, i);
        auto tok = l.read_next_tok();

        return make_shared<Single_object>(tok);
    }

    Data serialize() const {
        
        return data;
    }

    Data write(Data& obj_buffer, Idx& cur_obj_no, unordered_map<int, int>& obj_loc, shared_ptr<unordered_map<int, shared_ptr<Object>>> table) const {
        return data;
    }
};

shared_ptr<Object> direct_parse(const Data& data, Idx& i) {
        Lexer l(data, i);
        auto tok = l.peek_next_tok();
        if (Lexer::equalsString(tok, "<<")) {
            auto dict = Dict_object::parse(data, i);
            tok = l.peek_next_tok();

            if(Lexer::equalsString(tok, "stream")) {
                auto stream = Stream_object::parse(data, i, dict);
                return stream;
            }

            return dict;
        }
        else if(isdigit(tok[0])) {
            auto next = l.peek_next_next_tok();
            auto next_next = l.peek_next_next_next_tok();
            if(Lexer::equalsString(next_next, "R") && isdigit(next[0])) {
                auto ref = Reference::parse(data, i);
                return ref;
            }
            else {
                auto obj = Single_object::parse(data, i);

                return obj;
            }
        }
        else if(Lexer::equalsString(tok, "(")) {
            auto s = String_object::parse(data, i);
            return s;
        }
        else if(Lexer::equalsString(tok, "<")) {
            auto s = String_object::parse(data, i);
            return s;
        }
        else if(Lexer::equalsString(tok, "/")) {
            auto name = Name_object::parse(data, i);
            return name;
        }
        else if(Lexer::equalsString(tok, "[")) {
            auto arr = Array_object::parse(data, i);
            return arr;
        }
        else if(Lexer::equalsString(tok, "true")) {
            auto obj = Single_object::parse(data, i);;
            return obj;
        }
        else if(Lexer::equalsString(tok, "false")) {
            auto obj = Single_object::parse(data, i);
            return obj;
        }
        else if(Lexer::equalsString(tok, "null")) {
            auto obj = Single_object::parse(data, i);
            return obj;
        }
        else if(tok[0] == '+' || tok[0] == '-') {
            auto obj = Single_object::parse(data, i);
            return obj;
        }

        cout << "ERROR: cannot detect object type for token " << Lexer::toString(tok) << std::endl;
        exit(1);
        return NULL;
}

