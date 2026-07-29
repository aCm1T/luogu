#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Definition {
    string symbol;
    int offset;
};

struct CodeLine {
    int count;
    vector<string> tokens;
};

struct Module {
    vector<Definition> definitions;
    vector<string> externals;
    vector<CodeLine> code;
    int base = 0;
    int length = 0;
};

struct SymbolInfo {
    bool defined = false;
    bool multiple = false;
    int address = 0;
};

static int hex_value(const string &s) {
    int value = 0;
    for (char ch : s) {
        if (isspace(static_cast<unsigned char>(ch))) continue;
        value *= 16;
        if ('0' <= ch && ch <= '9') value += ch - '0';
        else value += ch - 'A' + 10;
    }
    return value;
}

static string hex4(int value) {
    stringstream ss;
    ss << uppercase << hex << setw(4) << setfill('0') << (value & 0xFFFF);
    return ss.str();
}

static bool is_case_terminator(const string &line) {
    for (char ch : line) {
        if (!isspace(static_cast<unsigned char>(ch))) return ch == '$';
    }
    return false;
}

static vector<Module> parse_case(const string &first_line, istream &in) {
    vector<Module> modules;
    Module current;
    bool have_module = false;
    string line = first_line;

    while (true) {
        if (is_case_terminator(line)) break;

        stringstream ss(line);
        char kind;
        ss >> kind;
        if (kind == 'D') {
            have_module = true;
            string symbol, offset;
            ss >> symbol >> offset;
            current.definitions.push_back({symbol, hex_value(offset)});
        } else if (kind == 'E') {
            have_module = true;
            string symbol;
            ss >> symbol;
            current.externals.push_back(symbol);
        } else if (kind == 'C') {
            have_module = true;
            string count_token;
            ss >> count_token;
            CodeLine code;
            code.count = hex_value(count_token);
            string token;
            while (ss >> token) code.tokens.push_back(token);
            current.length += code.count;
            current.code.push_back(code);
        } else if (kind == 'Z') {
            modules.push_back(current);
            current = Module();
            have_module = false;
        }

        if (!getline(in, line)) break;
    }

    if (have_module) modules.push_back(current);
    return modules;
}

static void update_checksum(int &checksum, int byte_value) {
    checksum = ((checksum << 1) & 0xFFFF) | ((checksum >> 15) & 1);
    checksum = (checksum + byte_value) & 0xFFFF;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string line;
    int tc = 1;
    while (getline(cin, line)) {
        if (line.empty()) continue;
        if (is_case_terminator(line)) break;

        vector<Module> modules = parse_case(line, cin);
        map<string, SymbolInfo> symbols;

        int next_address = 0x100;
        for (Module &module : modules) {
            module.base = next_address;
            next_address += module.length;

            for (const string &symbol : module.externals) {
                symbols[symbol];
            }
            for (const Definition &def : module.definitions) {
                SymbolInfo &info = symbols[def.symbol];
                if (!info.defined) {
                    info.defined = true;
                    info.address = module.base + def.offset;
                } else {
                    info.multiple = true;
                }
            }
        }

        int checksum = 0;
        for (const Module &module : modules) {
            for (const CodeLine &code : module.code) {
                int produced = 0;
                for (size_t i = 0; i < code.tokens.size() && produced < code.count; ++i) {
                    if (code.tokens[i] == "$") {
                        int ext_index = hex_value(code.tokens[++i]);
                        int value = 0;
                        if (0 <= ext_index && ext_index < static_cast<int>(module.externals.size())) {
                            const string &symbol = module.externals[ext_index];
                            auto it = symbols.find(symbol);
                            if (it != symbols.end() && it->second.defined) value = it->second.address;
                        }
                        update_checksum(checksum, (value >> 8) & 0xFF);
                        update_checksum(checksum, value & 0xFF);
                        produced += 2;
                    } else {
                        update_checksum(checksum, hex_value(code.tokens[i]) & 0xFF);
                        ++produced;
                    }
                }
            }
        }

        if (tc > 1) cout << '\n';
        cout << "Case " << tc++ << ": checksum = " << hex4(checksum) << '\n';
        cout << " SYMBOL   ADDR\n";
        cout << "--------  ----\n";
        for (const auto &entry : symbols) {
            cout << left << setw(8) << setfill(' ') << entry.first << "  ";
            if (entry.second.defined) cout << hex4(entry.second.address);
            else cout << "????";
            if (entry.second.multiple) cout << " M";
            cout << '\n';
        }
    }

    return 0;
}
