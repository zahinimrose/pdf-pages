#include <vector>
#include <iostream>
#include <string>

using namespace std;

typedef std::vector<char> Data;
typedef int Idx;

const char white_space[6] = {0, 9, 10, 12, 13, 32};

void append(Data& data, string&& str) {
    data.insert(data.end(), str.begin(), str.end());
}

void append(Data& data, Data str) {
    data.insert(data.end(), str.begin(), str.end());
}


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
    cout << endl;
}

//TODO: Implement cleaner lexer class without Idx
class Lexer {
public:
    Lexer(const Data& data, Idx& start) : data(data), cur_ptr(start) {}
    void print_next_tok() {
        auto tok = read_next_tok();
        for (char ch: tok) {
            cout << ch;
        }
    }
private:
    Idx& cur_ptr;
    const Data& data;

    std::vector<Data> tok_list;

    char curr_char() {
        return data[cur_ptr];
    }
    bool is_white_space() {
        char ch = curr_char();
        const char white_space[6] = {0, 9, 10, 12, 13, 32};
        for(int i = 0; i < 6; i++) {
            if(ch == white_space[i]) {
                return true;
            }
        }
        return false;
    }

    bool is_special() {
        char ch = curr_char();
        const char special[] = {'<', '>', '(', ')', '[', ']', '/', '%'};
        for(int i = 0; i < sizeof(special); i++) {
            if(ch == special[i]) {
                return true;
            }
        }
        return false;
    }

public:
    static string toString(const Data& data) {
        return string(data.begin(), data.end());
    }

    static bool equalsString(const Data& data, string str) {
        return str == toString(data);
    }

    Data read_next_tok() {
        Data tok;

        //Dumb hack to handle comments and white space
        while(is_white_space()) {
            cur_ptr++;
        }
        while(curr_char() == '%') {
            cur_ptr++;
            while(curr_char() != 13 && curr_char() != 10) {
                cur_ptr++;
            }
            while(is_white_space()) {
                cur_ptr++;
            }
        }
        while(is_white_space()) {
            cur_ptr++;
        }

        switch(curr_char()) {
            case '<': {
                cur_ptr++;
                if(curr_char() != '<') {
                    // cout << "ERROR: Lone < at " << cur_ptr <<  "\n";
                    // exit(1);
                    tok.push_back('<');
                    return tok;
                    break;
                }
                cur_ptr++;
                tok.push_back('<');
                tok.push_back('<');
                break;
            }
            case '>': {
                cur_ptr++;
                if(curr_char() != '>') {
                    // cout << "ERROR: Lone >\n";
                    // exit(1);
                    tok.push_back('<');
                    return tok;
                    break;
                }
                cur_ptr++;
                tok.push_back('>');
                tok.push_back('>');
                break;
            }
            case '(': {
                cur_ptr++;
                tok.push_back('(');
                break;
            }
            case ')': {
                cur_ptr++;
                tok.push_back(')');
                break;
            }
            case '[': {
                cur_ptr++;
                tok.push_back('[');
                break;
            }
            case ']': {
                cur_ptr++;
                tok.push_back(']');
                break;
            }
            case '/': {
                cur_ptr++;
                tok.push_back('/');
                break;
            }
            default: {
                while(!is_white_space()  && !is_special()) {
                    tok.push_back(curr_char());
                    cur_ptr++;
                }
                break;
            }
        }
        // print_tok(tok);
        return tok;
    }

    Data peek_next_tok() {
        int x = cur_ptr;
        Data d = read_next_tok();
        cur_ptr = x;
        return d;
    }
    
    Data peek_next_next_tok() {
        int x = cur_ptr;
        Data d = read_next_tok();
        d = read_next_tok();
        cur_ptr = x;
        return d;
    }
    Data peek_next_next_next_tok() {
        int x = cur_ptr;
        Data d = read_next_tok();
        d = read_next_tok();
        d = read_next_tok();
        cur_ptr = x;
        return d;
    }


};