#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>

#include "pdf.h"

using std::cout;
using std::string;
using std::vector;
using std::unordered_map;



// void dump(Data& data) {
//     string header = "%PDF-1.4\n";
//     data.insert(data.end(), header.begin(), header.end());
//     unordered_map<int, int> pos;
//     for(auto e: Object::table) {
//         pos[e.first] = data.size();
//         string obj_num = to_string(e.first);
//         data.insert(data.end(), obj_num.begin(), obj_num.end());
//         string rest = " 0 obj\n";
//         data.insert(data.end(), rest.begin(), rest.end());
//         auto obj = e.second->serialize();
//         data.insert(data.end(), obj.begin(), obj.end());
//         data.push_back('\n');
//         string str = "endobj\n";
//         data.insert(data.end(), str.begin(), str.end());
//     }
//     int startxref = data.size();
//     string str = "xref\n";
//     data.insert(data.end(), str.begin(), str.end());
//     str = "0 12\n";
//     data.insert(data.end(), str.begin(), str.end());
//     str = "0000000000 65535 f\n";
//     data.insert(data.end(), str.begin(), str.end());

//     str = " 00000 n\n";
//     for(int i = 0; i < pos.size();i++) {
//         string position = to_string(pos[i + 1]);
//         while (position.length() < 10) {
//             position.insert(position.begin(), '0');
//         }
//         data.insert(data.end(), position.begin(), position.end());
//         data.insert(data.end(), str.begin(), str.end());
//     }

//     str = "trailer\n";
//     data.insert(data.end(), str.begin(), str.end());
//     str = "<</Size 12\n/Root 7 0 R\n/Info 1 0 R>>\nstartxref\n";
//     data.insert(data.end(), str.begin(), str.end());
//     str = to_string(startxref);
//     data.insert(data.end(), str.begin(), str.end());
//     str = "\n%%EOF\n";
//     data.insert(data.end(), str.begin(), str.end());
// }

int main() {
    const std::string file_path = "sample/hello.pdf";

    Pdf p(file_path);


    // Dict_object page_obj = *(Dict_object*)p.table[2];
    // Name_object n;
    // n.name = "Parent";
    // page_obj.map.erase(n);

    // print_tok(page_obj.serialize());
    // Pdf_page page(p.table, page_obj);
    // Data d;
    // int i = 1;
    // page.render(d, i);

    print_tok(p.catalog.serialize());


}