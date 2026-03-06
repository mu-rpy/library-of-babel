#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
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
    std::string int_to_text(BigInt val) {
        std::string result;
        BigInt state = val;
        for (int i = 0; i < page_len; ++i) {
            state = mod_pow2(a * state + c, 20000);
            result += alphabet[(int)(state % (uint32_t)base)];
        }
        return result;
    }

    std::vector<std::string> browse(const BigInt& seed) {
        std::string text = int_to_text(seed);
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

std::string get_random_hex(int length = 50) {
    const std::string chars = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, (int)chars.size() - 1);
    std::string result;
    for (int i = 0; i < length; ++i) result += chars[dist(rng)];
    return result;
}

void save_page(const std::string& h, int w, int s, int v, int p,
               const std::vector<std::string>& lines) {
    std::string fname = "babel_rand_" + std::to_string(std::time(nullptr)) + ".txt";
    std::ofstream f(fname);
    if (!f) { std::cerr << "Could not write file.\n"; return; }
    f << "--- RANDOM LOCATION ---\n"
      << "HEX: " << h << "\nW: " << w << " | S: " << s
      << " | V: " << v << " | P: " << p << "\n\n";
    for (auto& line : lines) f << line << "\n";
    std::cout << "Saved to " << fname << "\n";
}

int main() {
    LibraryOfBabel babel;
    std::mt19937 rng(std::random_device{}());
    std::string h = get_random_hex();
    int w = std::uniform_int_distribution<int>(1,   4)(rng);
    int s = std::uniform_int_distribution<int>(1,   5)(rng);
    int v = std::uniform_int_distribution<int>(1,  32)(rng);
    int p = std::uniform_int_distribution<int>(1, 410)(rng);

    auto lines = babel.browse(babel.coords_to_seed(h, w, s, v, p));

    std::cout << "--- RANDOM LOCATION ---\nHEX: " << h
              << "\nW: " << w << " | S: " << s << " | V: " << v << " | P: " << p << "\n\n";
    for (auto& line : lines) std::cout << line << "\n";

    std::cout << "\nSave this page to a .txt file? [y/n]: ";
    std::string ans; std::getline(std::cin, ans);
    if (!ans.empty() && (ans[0]=='y' || ans[0]=='Y'))
        save_page(h, w, s, v, p, lines);
    return 0;
}
