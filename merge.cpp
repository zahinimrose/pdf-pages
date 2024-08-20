#include <filesystem>

#include "pdf.h"

string default_output = "merged.pdf";

int main() {
    Pdf merged;

    int count = 0;
    namespace fs = std::filesystem;
    for (const auto& entry : fs::directory_iterator(".")) {
        if (entry.is_regular_file() && entry.path().extension() == ".pdf") {
            string name = entry.path().filename().string();
            cout << "Merging " << name << "\n";
            Pdf p(name);
            merged.merge(p);
            count++;
        }
    }

    if(count == 0) {
        cout << "No pdf files found in current directory" << endl;
        exit(1);
    }

    merged.output(default_output);
    cout << "Successfully merged " << count << " files!" << endl;
    cout << "Output: " << default_output << endl;
}