#include <filesystem>

#include "pdf.h"

string default_output = "merged.pdf";

int main() {
    Pdf merged;

    namespace fs = std::filesystem;
    for (const auto& entry : fs::directory_iterator(".")) {
        if (entry.is_regular_file() && entry.path().extension() == ".pdf") {
            string name = entry.path().filename().string();
            cout << "Merging " << name << "\n";
            Pdf p(name);
            merged.merge(p);

        }
    }

    merged.output(default_output);
    cout << "Merging succesfully complete!" << endl;
    cout << "Output: " << default_output << endl;
}