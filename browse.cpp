#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include "bigint.hpp"

class LibraryOfBabel {
    const std::string alphabet = "abcdefghijklmnopqrstuvwxyz, . ";
    const int base     = 30;
    const int page_len = 3200;
    const BigInt m     = pow2(20000);
    const BigInt c     = BigInt("1442695040888963407");
    BigInt a;

public:
    LibraryOfBabel() {
        // browse.py: a = pow(6364136223846793005, 10, m)
        a = powmod(BigInt("6364136223846793005"), BigInt(10ULL), pow2(20000));
    }

    std::string generate_page_text(const BigInt& seed) {
        BigInt state = mod_pow2(a * seed + c, 20000);
        std::string result;
        for (int i = 0; i < page_len; ++i) {
            state = mod_pow2(a * state + c, 20000);
            result += alphabet[(int)(state % (uint32_t)base)];
        }
        return result;
    }

    std::vector<std::string> browse(const BigInt& seed) {
        std::string text = generate_page_text(seed);
        std::vector<std::string> lines;
        for (int i = 0; i < page_len; i += 80)
            lines.push_back(text.substr(i, 80));
        return lines;
    }

    BigInt coords_to_seed(const std::string& hex_addr, int wall, int shelf, int vol, int page) {
        return parse36(hex_addr) * BigInt(262400ULL)
             + BigInt((uint64_t)(wall  - 1)) * BigInt(65600ULL)
             + BigInt((uint64_t)(shelf - 1)) * BigInt(13120ULL)
             + BigInt((uint64_t)(vol   - 1)) * BigInt(410ULL)
             + BigInt((uint64_t)(page  - 1));
    }
};

void save_page(const std::string& h, int w, int s, int v, int page,
               const std::vector<std::string>& lines) {
    std::string fname = "babel_browse_" + std::to_string(std::time(nullptr)) + ".txt";
    std::ofstream f(fname);
    if (!f) { std::cerr << "Could not write file.\n"; return; }
    f << "--- LIBRARY BROWSER ---\n"
      << "HEX " << h << " | W:" << w << " S:" << s << " V:" << v << " | PAGE " << page << "\n\n";
    for (auto& line : lines) f << line << "\n";
    std::cout << "Saved to " << fname << "\n";
}

int main() {
    LibraryOfBabel babel;
    while (true) {
        std::cout << "\n--- LIBRARY BROWSER ---\n";
        std::string h;
        int w, s, v, p;
        std::cout << "Hex: ";         if (!(std::cin >> h)) break;
        std::cout << "Wall (1-4): ";  if (!(std::cin >> w)) break;
        std::cout << "Shelf (1-5): "; if (!(std::cin >> s)) break;
        std::cout << "Volume (1-32): "; if (!(std::cin >> v)) break;
        std::cout << "Page (1-410): "; if (!(std::cin >> p)) break;
        std::cin.ignore();

        if (w<1||w>4||s<1||s>5||v<1||v>32||p<1||p>410) {
            std::cout << "Invalid input.\n"; continue;
        }

        int cur = p;
        while (true) {
            auto lines = babel.browse(babel.coords_to_seed(h, w, s, v, cur));
            std::cout << "\n--- HEX " << h << " | W:" << w << " S:" << s
                      << " V:" << v << " | PAGE " << cur << " ---\n";
            for (auto& line : lines) std::cout << line << "\n";

            std::cout << "\n[n] Next  [p] Prev  [s] Save  [b] Back  [q] Quit: ";
            std::string cmd; std::getline(std::cin, cmd);
            if      (cmd=="n" && cur<410) cur++;
            else if (cmd=="p" && cur>1)   cur--;
            else if (cmd=="s") save_page(h, w, s, v, cur, lines);
            else if (cmd=="b") break;
            else if (cmd=="q") return 0;
        }
    }
    return 0;
}
