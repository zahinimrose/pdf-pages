#include <iostream>
#include <fstream>
#include "object.h"


class Pdf_page {
public:

    unordered_map<int, Object*>& table;
    Dict_object page;

    Pdf_page(unordered_map<int, Object*>& table, Dict_object page) : table(table), page(page) {}
    void render(Data& out, int& cur_obj) {
        auto page_data = page.write(out, cur_obj, table);
        append(out, page_data);
    }
};

class Pdf {
private:
    Data pdf_data;
    vector<Pdf_page> pages;
public:
    unordered_map<int, Object*> table;
    Dict_object trailer;
    Dict_object catalog;

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

        catalog = *(Dict_object*)trailer.get_deref("Root", table);
        Dict_object* pages_root = (Dict_object*)catalog.get_deref("Pages", table);

        add_pages(pages_root, pages);
    }

    Data render() {
        Data pdf;
        int cur_obj = 1;
        
        append(pdf, "%PDF-1.4\n%����");
        for(auto& page : pages) {
            page.render(pdf, cur_obj);
        }

        return pdf;
    }
private:
    void add_pages(Dict_object* root, vector<Pdf_page>& pages) {
        auto root_type = ((Name_object*)(root->get("Type")))->name;
        if(root_type == "Page") {
            pages.push_back(Pdf_page(table, *root));
            return;
        }
        else if(root_type == "Pages") {
            auto kids = (Array_object*)root->get("Kids");
            for(auto kid: kids->list) {
                auto node = (Dict_object*)(((Reference*)kid)->deref(table));
                add_pages(node, pages);
            }
            return;
        }
        cout << "ERROR: Page_node type must be page or pages" << endl;
        exit(1);
    }
};