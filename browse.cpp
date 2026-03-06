#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include "bigint.hpp"

// ── shared constants ──────────────────────────────────────────────────────────
static const std::string ALPHABET = "abcdefghijklmnopqrstuvwxyz, . ";
static const int         BASE     = 30;
static const int         PAGE_LEN = 3200;
static const BigInt      A        = BigInt("6364136223846793005");
static const BigInt      C        = BigInt("1442695040888963407");

// ── LCG page generator ───────────────────────────────────────────────────────
std::string make_page(const BigInt& seed) {
    std::string result;
    BigInt state = seed;
    for (int i = 0; i < PAGE_LEN; ++i) {
        state  = mod_pow2(A * state + C, 20000);
        result += ALPHABET[(int)(state % (uint32_t)BASE)];
    }
    return result;
}

std::vector<std::string> page_lines(const BigInt& seed) {
    std::string text = make_page(seed);
    std::vector<std::string> lines;
    for (int i = 0; i < PAGE_LEN; i += 80)
        lines.push_back(text.substr(i, 80));
    return lines;
}

// ── address ───────────────────────────────────────────────────────────────────
BigInt coords_to_seed(const std::string& hex, int wall, int shelf, int vol, int page) {
    return parse36(hex)        * BigInt(262400ULL)
         + BigInt((uint64_t)(wall  - 1)) * BigInt(65600ULL)
         + BigInt((uint64_t)(shelf - 1)) * BigInt(13120ULL)
         + BigInt((uint64_t)(vol   - 1)) * BigInt(410ULL)
         + BigInt((uint64_t)(page  - 1));
}

// ── helpers ───────────────────────────────────────────────────────────────────
void save(const std::string& h, int w, int s, int v, int p,
          const std::vector<std::string>& lines) {
    std::string fname = "babel_browse_" + std::to_string(std::time(nullptr)) + ".txt";
    std::ofstream f(fname);
    if (!f) { std::cerr << "Could not write file.\n"; return; }
    f << "HEX: " << h << "\nW: " << w << " | S: " << s
      << " | V: " << v << " | P: " << p << "\n\n";
    for (auto& l : lines) f << l << "\n";
    std::cout << "Saved to " << fname << "\n";
}

int main() {
    while (true) {
        std::cout << "\n--- LIBRARY BROWSER ---\n";
        std::string h;
        int w, s, v, p;
        std::cout << "Hex: ";          if (!(std::cin >> h)) break;
        std::cout << "Wall (1-4): ";   if (!(std::cin >> w)) break;
        std::cout << "Shelf (1-5): ";  if (!(std::cin >> s)) break;
        std::cout << "Volume (1-32): ";if (!(std::cin >> v)) break;
        std::cout << "Page (1-410): "; if (!(std::cin >> p)) break;
        std::cin.ignore();

        if (w<1||w>4||s<1||s>5||v<1||v>32||p<1||p>410) {
            std::cout << "Invalid input.\n"; continue;
        }

        int cur = p;
        while (true) {
            auto lines = page_lines(coords_to_seed(h, w, s, v, cur));
            std::cout << "\n--- HEX " << h << " | W:" << w << " S:" << s
                      << " V:" << v << " | PAGE " << cur << " ---\n";
            for (auto& l : lines) std::cout << l << "\n";

            std::cout << "\n[n] Next  [p] Prev  [s] Save  [b] Back  [q] Quit: ";
            std::string cmd; std::getline(std::cin, cmd);
            if      (cmd=="n" && cur<410) cur++;
            else if (cmd=="p" && cur>1)   cur--;
            else if (cmd=="s") save(h, w, s, v, cur, lines);
            else if (cmd=="b") break;
            else if (cmd=="q") return 0;
        }
    }
    return 0;
}