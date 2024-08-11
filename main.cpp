#include "pdf.h"

int main() {
    const std::string file_path = "sample/hello2.pdf";

    Pdf p(file_path);

    auto page1 = p.pages[0];
    auto page2 = p.pages[1];
    p.pages.clear();
    p.pages.push_back(page2);
    p.pages.push_back(page1);

    p.output("dump.pdf");

}