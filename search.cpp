#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include "bigint.hpp"

// ── shared constants ──────────────────────────────────────────────────────────
static const std::string ALPHABET = "abcdefghijklmnopqrstuvwxyz, . ";
static const int         BASE     = 30;
static const int         PAGE_LEN = 3200;
static const BigInt      A        = BigInt("6364136223846793005");
static const BigInt      C        = BigInt("1442695040888963407");
static const BigInt      M        = pow2(20000);

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

// ── search ────────────────────────────────────────────────────────────────────
BigInt text_to_int(const std::string& text) {
    BigInt val(0ULL), b((uint64_t)BASE);
    for (int i = (int)text.size() - 1; i >= 0; --i)
        val = val * b + BigInt((uint64_t)ALPHABET.find(text[i]));
    return val;
}

BigInt search_exact(const std::string& phrase) {
    std::string full = phrase;
    std::transform(full.begin(), full.end(), full.begin(), ::tolower);
    if ((int)full.size() < PAGE_LEN) full.append(PAGE_LEN - full.size(), ' ');
    else full = full.substr(0, PAGE_LEN);
    BigInt target = text_to_int(full);
    BigInt inv    = ext_gcd_inv(A, M);
    BigInt diff   = target - C;
    
    return mod_pow2(inv * diff, 20000);
}

// ── seed → coords ─────────────────────────────────────────────────────────────
struct Coords { std::string hex; int wall, shelf, vol, page; };

Coords seed_to_coords(BigInt seed) {
    int page  = (int)(seed % 410u) + 1; seed = seed / 410u;
    int vol   = (int)(seed %  32u) + 1; seed = seed /  32u;
    int shelf = (int)(seed %   5u) + 1; seed = seed /   5u;
    int wall  = (int)(seed %   4u) + 1; seed = seed /   4u;
    const std::string b36 = "0123456789abcdefghijklmnopqrstuvwxyz";
    std::string hex;
    if (seed.is_zero()) { hex = "0"; }
    else {
        while (!seed.is_zero()) {
            hex += b36[(int)(seed % 36u)];
            seed = seed / 36u;
        }
        std::reverse(hex.begin(), hex.end());
    }
    return {hex, wall, shelf, vol, page};
}

// ── save ──────────────────────────────────────────────────────────────────────
void save(const std::string& phrase, const Coords& co,
          const std::vector<std::string>& lines) {
    std::string fname = "babel_search_" + std::to_string(std::time(nullptr)) + ".txt";
    std::ofstream f(fname);
    if (!f) { std::cerr << "Could not write file.\n"; return; }
    f << "Phrase: " << phrase    << "\n"
      << "HEX: "   << co.hex    << "\n"
      << "W: "     << co.wall   << " | S: " << co.shelf
      << " | V: "  << co.vol    << " | P: " << co.page << "\n\n";
    for (auto& l : lines) f << l << "\n";
    std::cout << "Saved to " << fname << "\n";
}

int main() {
    std::string phrase;
    std::cout << "Enter phrase to find: ";
    std::getline(std::cin, phrase);

    BigInt seed  = search_exact(phrase);
    Coords co    = seed_to_coords(seed);
    auto   lines = page_lines(seed);

    std::cout << "\n--- SEARCH RESULT ---\n"
              << "Phrase: " << phrase  << "\n"
              << "HEX: "   << co.hex   << "\n"
              << "W: "     << co.wall  << " | S: " << co.shelf
              << " | V: "  << co.vol   << " | P: " << co.page << "\n\n";
    for (auto& l : lines) std::cout << l << "\n";

    std::cout << "\nSave to .txt? [y/n]: ";
    std::string ans; std::getline(std::cin, ans);
    if (!ans.empty() && (ans[0]=='y' || ans[0]=='Y'))
        save(phrase, co, lines);
    return 0;
}