#include <iostream>
#include <fstream>
#include "object.h"

class Pdf_page {
public:
    unordered_map<int, Object*>& table;
    Dict_object page;

    Pdf_page(unordered_map<int, Object*>& table, Dict_object page) : table(table), page(page) {}

    Data render(Data& out, int& cur_obj, unordered_map<int, int>& obj_loc) {
        return page.write(out, cur_obj, obj_loc, table);
    }
};

class Pdf {
private:
    Data pdf_data;
public:
    vector<Pdf_page> pages;
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
        // cout << "Parsed obj num " << obj_num << std::endl;
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
    Pdf() = default;
    Pdf(string file_path) {
        std::ifstream file_stream(file_path, std::ios::in | std::ios::binary);
        char ch;

        while(file_stream) {
            file_stream.get(ch);
            pdf_data.push_back(ch);
        }
        file_stream.close();

        // auto ctx = data_context(17578);
        // print_tok(ctx);
        // exit(1);

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
        unordered_map<int, int> obj_loc;
        vector<Data> page_datas;
        int page_count = pages.size();

        append(pdf, "%PDF-1.4\n%����\n");
        for(auto& page : pages) {
            auto page_data = page.render(pdf, cur_obj, obj_loc);
            page_datas.push_back(page_data);
        }

        int root_pages_num =  cur_obj + page_count;
        Array_object* kids = new Array_object;

        for(auto& data : page_datas) {
            int i = 0;
            Dict_object page_dict = Dict_object::parse(data, i);
            Reference* r = new Reference;
            r->ref_no = root_pages_num;
            r->gen_no = 0;
            page_dict.map[Name_object("Parent")] = r;

            int pos = pdf.size();
            obj_loc[cur_obj] = pos;
            append(pdf, to_string(cur_obj));
            append(pdf, " 0 obj\n");
            append(pdf, page_dict.serialize());
            append(pdf, "\nendobj\n");


            Reference* c = new Reference;
            c->ref_no = cur_obj;
            c->gen_no = 0;
            kids->list.push_back(c);

            cur_obj++;
        }
        

        Dict_object root_pages;

        Name_object* type = new Name_object("Pages");
        root_pages.map[Name_object("Type")] = type;

        Single_object* count = new Single_object(page_count);
        root_pages.map[Name_object("Count")] = count;

        root_pages.map[Name_object("Kids")] = kids;

        int pos = pdf.size();
        obj_loc[cur_obj] = pos;
        append(pdf, to_string(cur_obj));
        append(pdf, " 0 obj\n");
        append(pdf, root_pages.serialize());
        append(pdf, "\nendobj\n");
        
        int pages_no = cur_obj;
        cur_obj++;

        Dict_object catalog;
        type = new Name_object("Catalog");
        catalog.map[Name_object("Type")] = type;
        Reference* c = new Reference;
        c->ref_no = pages_no;
        c->gen_no = 0;
        catalog.map[Name_object("Pages")] = c;

        pos = pdf.size();
        obj_loc[cur_obj] = pos;
        append(pdf, to_string(cur_obj));
        append(pdf, " 0 obj\n");
        append(pdf, catalog.serialize());
        append(pdf, "\nendobj\n");

        int startxref = pdf.size();
        append(pdf, "xref\n");
        append(pdf, "0 ");
        append(pdf, to_string(cur_obj + 1));
        append(pdf, "\n");
        for(int i = 1; i <= cur_obj; i++) {
            string offset = to_string(obj_loc[i]);
            while(offset.length() < 10) {
                offset.insert(offset.begin(), '0');
            }
            pdf.insert(pdf.end(), offset.begin(), offset.end());
            append(pdf, " 00000 n\n");
        }

        append(pdf, "trailer\n");

        Dict_object trailer;
        trailer.map[Name_object("Size")] = new Single_object(cur_obj);

        Reference* r = new Reference;
        r->ref_no = cur_obj;
        r->gen_no = 0;
        trailer.map[Name_object("Root")] = r;

        append(pdf, trailer.serialize());
        append(pdf, "startxref\n");
        string str_xref = to_string(startxref);
        pdf.insert(pdf.end(), str_xref.begin(), str_xref.end());
        append(pdf, "\n%%EOF");



        return pdf;
    }

    void output(string file_path) {
        std::ofstream file_stream(file_path, std::ios::out | std::ios::binary);
        auto d = render();
        for(char ch: d) {
            file_stream.put(ch);
        }
    }
    Data data_context(int i) {
        int radius = 10;
        std::ofstream file_stream("debug", std::ios::out | std::ios::binary);

        auto d = Data(pdf_data.begin() + i - radius, pdf_data.begin() + i + radius);
        for(char ch: d) {
            file_stream.put(ch);
        }

        return d;
    }
private:
    void add_pages(Dict_object* root, vector<Pdf_page>& pages) {
        auto root_type = ((Name_object*)(root->get("Type")))->name;
        if(root_type == "Page") {
            Name_object n;
            n.name = "Parent";
            root->map.erase(n);
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