#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
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
std::string random_hex(int len = 50) {
    const std::string chars = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, (int)chars.size() - 1);
    std::string s; for (int i = 0; i < len; ++i) s += chars[dist(rng)];
    return s;
}

void save(const std::string& h, int w, int s, int v, int p,
          const std::vector<std::string>& lines) {
    std::string fname = "babel_rand_" + std::to_string(std::time(nullptr)) + ".txt";
    std::ofstream f(fname);
    if (!f) { std::cerr << "Could not write file.\n"; return; }
    f << "HEX: " << h << "\nW: " << w << " | S: " << s
      << " | V: " << v << " | P: " << p << "\n\n";
    for (auto& l : lines) f << l << "\n";
    std::cout << "Saved to " << fname << "\n";
}

int main() {
    std::mt19937 rng(std::random_device{}());
    std::string h = random_hex();
    int w = std::uniform_int_distribution<int>(1,   4)(rng);
    int s = std::uniform_int_distribution<int>(1,   5)(rng);
    int v = std::uniform_int_distribution<int>(1,  32)(rng);
    int p = std::uniform_int_distribution<int>(1, 410)(rng);

    auto lines = page_lines(coords_to_seed(h, w, s, v, p));

    std::cout << "--- RANDOM LOCATION ---\n"
              << "HEX: " << h << "\n"
              << "W: " << w << " | S: " << s << " | V: " << v << " | P: " << p << "\n\n";
    for (auto& l : lines) std::cout << l << "\n";

    std::cout << "\nSave to .txt? [y/n]: ";
    std::string ans; std::getline(std::cin, ans);
    if (!ans.empty() && (ans[0]=='y' || ans[0]=='Y'))
        save(h, w, s, v, p, lines);
    return 0;
}