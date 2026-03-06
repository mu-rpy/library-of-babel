#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include "bigint.hpp"

class LibraryOfBabel {
    const std::string alphabet = "abcdefghijklmnopqrstuvwxyz, . ";
    const int base     = 30;
    const int page_len = 3200;
    const BigInt m     = pow2(20000);
    const BigInt a     = BigInt("6364136223846793005");
    const BigInt c     = BigInt("1442695040888963407");

public:
    BigInt text_to_int(const std::string& text) {
        BigInt val(0ULL), b((uint64_t)base);
        for (int i = (int)text.size() - 1; i >= 0; --i)
            val = val * b + BigInt((uint64_t)alphabet.find(text[i]));
        return val;
    }

    std::string int_to_text(BigInt val) {
        std::string result;
        BigInt state = val;
        for (int i = 0; i < page_len; ++i) {
            state = mod_pow2(a * state + c, 20000);
            result += alphabet[(int)(state % (uint32_t)base)];
        }
        return result;
    }

    BigInt search_exact(const std::string& text) {
        std::string full = text;
        std::transform(full.begin(), full.end(), full.begin(), ::tolower);
        if ((int)full.size() < page_len) full.append(page_len - full.size(), ' ');
        else full = full.substr(0, page_len);
        BigInt target = text_to_int(full);
        BigInt inv    = ext_gcd_inv(a, m);
        BigInt diff   = target - c;
        if (diff.neg) diff = diff + m;
        return mod_pow2(inv * diff, 20000);
    }

    std::vector<std::string> browse(const BigInt& seed) {
        std::string text = int_to_text(seed);
        std::vector<std::string> lines;
        for (int i = 0; i < page_len; i += 80)
            lines.push_back(text.substr(i, 80));
        return lines;
    }
};

void save_page(const std::string& phrase, const BigInt& seed,
               const std::vector<std::string>& lines) {
    std::string fname = "babel_search_" + std::to_string(std::time(nullptr)) + ".txt";
    std::ofstream f(fname);
    if (!f) { std::cerr << "Could not write file.\n"; return; }
    f << "--- SEARCH RESULT ---\nPhrase: " << phrase << "\nSeed: " << seed << "\n\n";
    for (auto& line : lines) f << line << "\n";
    std::cout << "Saved to " << fname << "\n";
}

int main() {
    LibraryOfBabel babel;
    std::string phrase;
    std::cout << "Enter phrase to find: ";
    std::getline(std::cin, phrase);

    BigInt seed = babel.search_exact(phrase);
    std::cout << "Seed: " << seed << "\n\n";
    auto lines = babel.browse(seed);
    for (auto& line : lines) std::cout << line << "\n";

    std::cout << "\nSave this page to a .txt file? [y/n]: ";
    std::string ans; std::getline(std::cin, ans);
    if (!ans.empty() && (ans[0]=='y' || ans[0]=='Y'))
        save_page(phrase, seed, lines);
    return 0;
}
