#include <iostream>

#include "pdf.h"

int main() {
    Pdf file1 = Pdf("samples/sample.pdf");
    std::cout << file1.get_page_count() << "\n";

    Pdf_page p = file1.get_page(2);
    Pdf file2 = Pdf(p);

    file2.output("output.pdf");
}