#include <iostream>
#include <cstdint>
#include <memory.h>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <sstream>
#include <algorithm>

void* input;
uint64_t saved_length;
uint8_t* end;

uint32_t a;
uint32_t b;
uint32_t c;
uint32_t d;
uint32_t block[16];

const uint32_t t[65] = { 0,
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

std::ifstream::pos_type filesize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

void append_padding_bits() {
    end = static_cast<uint8_t*>(input) + saved_length;
    *end = 0x80;
    ++end;
    while ((end - static_cast<uint8_t*>(input)) % 64 != 56) {
        *end = 0x00;
        ++end;
    }
}

void append_length() {
    uint64_t length = saved_length * 8;
    memcpy(static_cast<void*>(end), &length, 8);
    end += 8;
 
}

void init_buffer() {
    uint8_t _a[4] = { 0x01, 0x23, 0x45, 0x67 };
    uint8_t _b[4] = { 0x89, 0xab, 0xcd, 0xef };
    uint8_t _c[4] = { 0xfe, 0xdc, 0xba, 0x98 };
    uint8_t _d[4] = { 0x76, 0x54, 0x32, 0x10 };
    a = *reinterpret_cast<uint32_t*>(&_a);
    b = *reinterpret_cast<uint32_t*>(&_b);
    c = *reinterpret_cast<uint32_t*>(&_c);
    d = *reinterpret_cast<uint32_t*>(&_d);
}

uint32_t rotate_left(uint32_t x, uint32_t s) { return (x << s) | (x >> (32 - s)); }

uint32_t F(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (~x & z); }
uint32_t G(uint32_t x, uint32_t y, uint32_t z) { return (x & z) | (~z & y); }
uint32_t H(uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; }
uint32_t I(uint32_t x, uint32_t y, uint32_t z) { return y ^ (~z | x); }

void round(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t k, uint32_t s, uint32_t i, uint32_t(*function)(uint32_t, uint32_t, uint32_t)) {
    a += function(b, c, d) + block[k] + t[i];
    a = rotate_left(a, s);
    a += b;
}
std::vector <uint32_t> s1 = { 7,12,17,22 };
std::vector <uint32_t> s2 = { 5,9,14,20 };
std::vector <uint32_t> s3 = { 4,11,16,23 };
std::vector <uint32_t> s4 = { 6,10,15,21 };
void process_block(uint8_t* adress) {
    memcpy(&block, static_cast<void*>(adress), 64);
    uint32_t aa = a, bb = b, cc = c, dd = d; 
    std::vector <uint32_t*> f = { &a,&b,&c, &d };
    for (int i = 0; i < 64; i++) {
        if (i >= 0 && i<=15) {
            round(*f[0], *f[1], *f[2], *f[3], i, s1[i%4], i+1, F);
            f.insert(f.begin(), f.back());
            f.pop_back();
        }
        if (i >= 16 && i <= 31) {
            round(*f[0], *f[1], *f[2], *f[3], (i*5+1)%16, s2[i%4], i+1, G);
            f.insert(f.begin(), f.back());
            f.pop_back();
        }
        if (i >= 32 && i <= 47) {
            round(*f[0], *f[1], *f[2], *f[3], (i*3+5)%16, s3[i%4], i+1, H);
            f.insert(f.begin(), f.back());
            f.pop_back();
        }
        if (i >= 48 && i <= 63) {
            round(*f[0], *f[1], *f[2], *f[3], (i*7)%16, s4[i%4], i+1, I);
            f.insert(f.begin(), f.back());
            f.pop_back();
        }
    }
    a += aa, b += bb, c += cc, d += dd;
}

void process() {
    uint8_t* temp = static_cast<uint8_t*>(input);
    while (temp != end)
        process_block(temp), temp += 64;
}

std::vector<uint8_t> finish() {
    std::vector<uint8_t> hash(16);
    memcpy(&hash[0], &a, 4);
    memcpy(&hash[4], &b, 4);
    memcpy(&hash[8], &c, 4);
    memcpy(&hash[12], &d, 4);
    return hash;
}

std::vector<uint8_t> MD5(void* original_input, uint64_t size) {
    uint8_t* place = new uint8_t[size + 100];

    memcpy(static_cast<void*>(place), original_input, size);
    input = static_cast<void*>(place);
    saved_length = size;
    append_padding_bits();
    append_length();
    init_buffer();
    process();
  
    delete[] place;
    return finish();
}

std::string md5hash_to_string(std::vector<uint8_t> hash) {
    std::string hex_char = "0123456789abcdef";
    std::string ret = "";

    for (int i = 0; i < 16; ++i) {
        ret += hex_char[hash[i] >> 4];
        ret += hex_char[hash[i] & 0x0F];
    }

    return ret;
}

int main() {
    while (true) {
        int c;
        std::cout << "\nGenerate MD5 Examples - 1 / Check - 2 / String - 3 /Hash file - 4 ";
        std::cin >> c;

        if (c == 1) {
            std::ifstream fin("example.txt");
            std::ofstream fout("out.txt");
            std::string s;
            while (std::getline(fin, s)) {
                std::cout << s << " Hash: " << md5hash_to_string(MD5(&s[0], s.size())) << std::endl;
                fout << md5hash_to_string(MD5(&s[0], s.size())) << "\n";
            }
            fin.close();
            fout.close();

        }
        else if (c == 2) {
            std::ifstream fin("example.txt");
            std::ifstream fout("out.txt");
            std::string s1;
            std::string s2;
            while (std::getline(fout, s1)) {
                std::getline(fin, s2);
                s2 = md5hash_to_string(MD5(&s2[0], s2.size()));
                std::cout << s2;
                if (s1.compare(s2) != 0) {
                    std::cout << " - Error\n";
                }
                else {
                    std::cout << " - Good\n";
                }
            }
            fin.close();
            fout.close();
        }
        else if (c == 3) {
            std::string s;
            getchar();
            getline(std::cin, s);
            if (s.empty()) {
                s = "";
            }
            std::cout << s << " Hash: " << md5hash_to_string(MD5(&s[0], s.size())) << std::endl;

        }
        else if (c == 4) {
            std::string name;
            getchar();
            getline(std::cin, name);
            std::ifstream fin(name);
            std::ofstream fout("out.txt");
            std::string s((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
            std::cout << " Hash: " << md5hash_to_string(MD5(&s[0], s.size())) << std::endl;
            fout << md5hash_to_string(MD5(&s[0], s.size())) << "\n";
            fin.close();
            fout.close();
        }
    }
    return 0;
}