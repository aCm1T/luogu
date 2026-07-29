#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

struct Node {
    bool leaf = true;
    int color = 0;
    unique_ptr<Node> child[4];
};

static int hex_value(char ch) {
    if ('0' <= ch && ch <= '9') return ch - '0';
    return ch - 'A' + 10;
}

static char hex_digit(int v) {
    return v < 10 ? static_cast<char>('0' + v) : static_cast<char>('A' + v - 10);
}

static string hex_to_bits(const string &hex) {
    string bits;
    bits.reserve(hex.size() * 4);
    for (char ch : hex) {
        int v = hex_value(ch);
        for (int bit = 3; bit >= 0; --bit) {
            bits.push_back(((v >> bit) & 1) ? '1' : '0');
        }
    }
    size_t delimiter = bits.find('1');
    return delimiter == string::npos ? string() : bits.substr(delimiter + 1);
}

static unique_ptr<Node> parse_node(const string &bits, size_t &pos) {
    auto node = make_unique<Node>();
    if (bits[pos++] == '1') {
        node->leaf = true;
        node->color = bits[pos++] - '0';
    } else {
        node->leaf = false;
        for (int i = 0; i < 4; ++i) node->child[i] = parse_node(bits, pos);
    }
    return node;
}

static unique_ptr<Node> clone_node(const Node *node) {
    auto out = make_unique<Node>();
    out->leaf = node->leaf;
    out->color = node->color;
    if (!node->leaf) {
        for (int i = 0; i < 4; ++i) out->child[i] = clone_node(node->child[i].get());
    }
    return out;
}

static unique_ptr<Node> make_leaf(int color) {
    auto node = make_unique<Node>();
    node->leaf = true;
    node->color = color;
    return node;
}

static unique_ptr<Node> intersect_nodes(const Node *a, const Node *b) {
    if (a->leaf && b->leaf) return make_leaf(a->color & b->color);
    if (a->leaf) return a->color == 0 ? make_leaf(0) : clone_node(b);
    if (b->leaf) return b->color == 0 ? make_leaf(0) : clone_node(a);

    auto out = make_unique<Node>();
    out->leaf = false;
    for (int i = 0; i < 4; ++i) {
        out->child[i] = intersect_nodes(a->child[i].get(), b->child[i].get());
    }

    bool homogeneous = true;
    int color = out->child[0]->leaf ? out->child[0]->color : -1;
    for (int i = 0; i < 4; ++i) {
        if (!out->child[i]->leaf || out->child[i]->color != color) {
            homogeneous = false;
            break;
        }
    }
    if (homogeneous) return make_leaf(color);
    return out;
}

static void emit_bits(const Node *node, string &bits) {
    if (node->leaf) {
        bits.push_back('1');
        bits.push_back(static_cast<char>('0' + node->color));
    } else {
        bits.push_back('0');
        for (int i = 0; i < 4; ++i) emit_bits(node->child[i].get(), bits);
    }
}

static string bits_to_hex(string bits) {
    bits.insert(bits.begin(), '1');
    while (bits.size() % 4 != 0) bits.insert(bits.begin(), '0');

    string hex;
    hex.reserve(bits.size() / 4);
    for (size_t i = 0; i < bits.size(); i += 4) {
        int v = 0;
        for (int j = 0; j < 4; ++j) v = v * 2 + (bits[i + j] - '0');
        hex.push_back(hex_digit(v));
    }
    return hex;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string first, second;
    int tc = 1;
    while (cin >> first >> second && !(first == "0" && second == "0")) {
        string bits1 = hex_to_bits(first);
        string bits2 = hex_to_bits(second);
        size_t pos1 = 0, pos2 = 0;
        unique_ptr<Node> a = parse_node(bits1, pos1);
        unique_ptr<Node> b = parse_node(bits2, pos2);
        unique_ptr<Node> result = intersect_nodes(a.get(), b.get());

        string out_bits;
        emit_bits(result.get(), out_bits);

        if (tc > 1) cout << '\n';
        cout << "Image " << tc++ << ":\n" << bits_to_hex(out_bits) << '\n';
    }

    return 0;
}
