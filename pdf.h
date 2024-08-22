#include <iostream>
#include <fstream>
#include "object.h"

class Pdf_page {
    shared_ptr<unordered_map<int, Object*>> table;
    Dict_object page;

public:
    Pdf_page() = default;
    Pdf_page(shared_ptr<unordered_map<int, Object*>> table, Dict_object page) : table(table), page(page) {}
    Pdf_page& operator=(const Pdf_page& that) {
        table = that.table;
        page = that.page;

        return *this;
    }

    Data render(Data& out, int& cur_obj, unordered_map<int, int>& obj_loc) {
        return page.write(out, cur_obj, obj_loc, table);
    }
};

class Pdf {
public:
    const static inline string default_output = "output.pdf";

    Pdf() = default;
    Pdf(string file_path) {
        read_file_into_data(file_path);

        // auto c = data_context(512);
        // print_tok(c);
        // exit(1);

        parse_obj_and_trailer();
        construct_page_list();
    }
    Pdf(Pdf_page page) {
        pages.push_back(page);
    }

//Outputs as default file name
    void output() {
        output(default_output);
    }

//Outputs the Pdf object into file name specified
    void output(string file_path) {
        std::ofstream file_stream(file_path, std::ios::out | std::ios::binary);
        auto d = render();
        for(char ch: d) {
            file_stream.put(ch);
        }
    }

    inline int get_page_count() {
        return pages.size();
    }

    Pdf_page get_page(int i) {
        if(i > pages.size() || i < 1) {
            cerr << "Page index " << i <<  " out of bounds\n";
            exit(1);
        }
        return pages[i - 1];
    }

    void append_page(Pdf_page page) {
        pages.push_back(page);
    }

    void merge(const Pdf& p) {
        pages.insert(pages.end(), p.pages.begin(), p.pages.end());
    }

private:
    Data pdf_data;
    vector<Pdf_page> pages;
    shared_ptr<unordered_map<int, Object*>> ref_table;
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
            exit(1);
        }
        auto obj =  direct_parse(data, i);
        tok = l.read_next_tok();
        if(!Lexer::equalsString(tok, "endobj")) {
            cout << "ERROR: Indirect reference must end with endobj";
        }

        (*ref_table)[obj_num] = obj;
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

    Data data_context(int i) {
        int radius = 3;
        std::ofstream file_stream("debug", std::ios::out | std::ios::binary);

        auto d = Data(pdf_data.begin() + i - radius, pdf_data.begin() + i + radius);
        for(char ch: d) {
            file_stream.put(ch);
        }

        return d;
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
        Array_object kids;

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
            kids.list.push_back(c);

            cur_obj++;
        }
        

        Dict_object root_pages;

        Name_object* type = new Name_object("Pages");
        root_pages.map[Name_object("Type")] = type;

        Single_object* count = new Single_object(page_count);
        root_pages.map[Name_object("Count")] = count;

        root_pages.map[Name_object("Kids")] = &kids;

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

    void add_pages(Dict_object* root, vector<Pdf_page>& pages) {
        auto root_type = ((Name_object*)(root->get("Type")))->name;
        if(root_type == "Page") {
            root->map.erase(Name_object("Parent"));
            
            //Annots entry is being removed to remove circular reference to page
            //TODO: Fix circular depedence issue on page serializing
            root->map.erase(Name_object("Annots"));

            pages.push_back(Pdf_page(ref_table, *root));
            return;
        }
        else if(root_type == "Pages") {
            auto kids = (Array_object*)root->get("Kids");
            for(auto kid: kids->list) {
                auto node = (Dict_object*)(((Reference*)kid)->deref(ref_table));
                add_pages(node, pages);
            }
            return;
        }
        cout << "ERROR: Page_node type must be page or pages" << endl;
        exit(1);
    }

    void read_file_into_data(string file_path) {
        std::ifstream file_stream;
        file_stream.open(file_path, std::ios::in | std::ios::binary);
        if(file_stream.fail()) {
            cout <<  file_path << " not found" << endl;
            exit(1);
        }
        
        char ch;

        while(file_stream) {
            file_stream.get(ch);
            pdf_data.push_back(ch);
        }
        file_stream.close();
    }

    void parse_obj_and_trailer() {
        validate_pdf_header();
        ref_table = make_shared<unordered_map<int, Object*>>();

        Idx i = 0;
        parse_indirect_objects(pdf_data, i);
        Lexer n(pdf_data, i);
        auto tok = n.read_next_tok();
        while(!Lexer::equalsString(tok, "trailer")) {
            tok = n.read_next_tok();
        }

        trailer = Dict_object::parse(pdf_data, i);
    }

    void validate_pdf_header() {
        //TODO: implement pdf header validation
        // Lexer l(pdf_data, i);
        // auto tok = l.read_next_tok();

        // l.read_next_tok();
    }

    void construct_page_list() {
        catalog = *(Dict_object*)trailer.get_deref("Root", ref_table);
        Dict_object* pages_root = (Dict_object*)catalog.get_deref("Pages", ref_table);

        add_pages(pages_root, pages);
    };
};