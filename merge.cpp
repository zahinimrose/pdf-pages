#include <filesystem>

#include "pdf.h"

namespace fs = std::filesystem;

int main() {
    Pdf merged;

    for (const auto& entry : fs::directory_iterator(".")) {
        if (entry.is_regular_file() && entry.path().extension() == ".pdf") {
            string name = entry.path().filename().string();
            cout << "Merging " << name << "\n";
            Pdf p(name);
            merged.merge(p);

        }
    }

    merged.output("out2.pdf");
}