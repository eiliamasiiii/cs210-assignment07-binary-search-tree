#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

struct Node {
    int key;
    Node* left = nullptr;
    Node* right = nullptr;
    explicit Node(int value) : key(value) {}
};

class BinarySearchTree {
    Node* root = nullptr;

    static bool insertNode(Node*& node, int key) {
        if (!node) {
            node = new Node(key);
            return true;
        }
        if (key < node->key) return insertNode(node->left, key);
        if (key > node->key) return insertNode(node->right, key);
        return false; // This tree stores distinct integers.
    }
    static bool removeNode(Node*& node, int key) {
        if (!node) return false;
        if (key < node->key) return removeNode(node->left, key);
        if (key > node->key) return removeNode(node->right, key);
        if (!node->left || !node->right) {
            // Leaf: replacement is null. One child: promote that child.
            Node* old = node;
            node = node->left ? node->left : node->right;
            delete old;
        } else {
            // The inorder successor is the smallest key on the right.
            Node* successor = node->right;
            while (successor->left) successor = successor->left;
            node->key = successor->key;
            removeNode(node->right, successor->key);
        }
        return true;
    }
    static void inorderNode(const Node* node, std::vector<int>& out) {
        if (!node) return;
        inorderNode(node->left, out);
        out.push_back(node->key);
        inorderNode(node->right, out);
    }
    static void preorderNode(const Node* node, std::vector<int>& out) {
        if (!node) return;
        out.push_back(node->key);
        preorderNode(node->left, out);
        preorderNode(node->right, out);
    }
    static void postorderNode(const Node* node, std::vector<int>& out) {
        if (!node) return;
        postorderNode(node->left, out);
        postorderNode(node->right, out);
        out.push_back(node->key);
    }
    static int heightNode(const Node* node) {
        if (!node) return -1; // Height counts edges, so a leaf has height 0.
        const int left = heightNode(node->left);
        const int right = heightNode(node->right);
        return 1 + (left > right ? left : right);
    }
    static void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }
public:
    BinarySearchTree() = default;
    ~BinarySearchTree() { destroy(root); }
    // Prevent shallow copies of the owning root pointer.
    BinarySearchTree(const BinarySearchTree&) = delete;
    BinarySearchTree& operator=(const BinarySearchTree&) = delete;

    bool insert(int key) { return insertNode(root, key); }
    bool remove(int key) { return removeNode(root, key); }
    bool search(int key) const {
        const Node* current = root;
        while (current) {
            if (key == current->key) return true;
            current = key < current->key ? current->left : current->right;
        }
        return false;
    }
    std::vector<int> inorder() const {
        std::vector<int> out;
        inorderNode(root, out);
        return out;
    }
    std::vector<int> preorder() const {
        std::vector<int> out;
        preorderNode(root, out);
        return out;
    }
    std::vector<int> postorder() const {
        std::vector<int> out;
        postorderNode(root, out);
        return out;
    }
    int height() const { return heightNode(root); }
};

int checks = 0;
void check(bool condition, const std::string& label) {
    ++checks;
    std::cout << (condition ? "PASS: " : "FAIL: ") << label << '\n';
    if (!condition) throw std::runtime_error(label);
}
bool isSorted(const std::vector<int>& values) {
    for (std::size_t i = 1; i < values.size(); ++i)
        if (values[i - 1] > values[i]) return false;
    return true;
}
void print(const std::string& label, const std::vector<int>& values) {
    std::cout << label;
    for (int value : values) std::cout << ' ' << value;
    std::cout << '\n';
}
void demonstrateRemoval(BinarySearchTree& tree, int key,
                        const std::string& type,
                        const std::vector<int>& expected) {
    std::cout << "\nRemove " << key << " (" << type << ")\n";
    check(tree.remove(key), "removal succeeds");
    const auto actual = tree.inorder();
    print("Inorder:", actual);
    check(isSorted(actual), "inorder remains sorted");
    check(actual == expected && !tree.search(key),
          "exact remaining keys verified; removed key is absent");
    std::cout << "Height: " << tree.height() << '\n';
}
int main() {
    BinarySearchTree tree;
    check(tree.height() == -1 && tree.inorder().empty() &&
          tree.preorder().empty() && tree.postorder().empty() &&
          !tree.search(42) && !tree.remove(42), "empty tree operations");
    const std::vector<int> input =
        {48, 26, 72, 14, 35, 61, 90, 8, 19, 31, 40, 66};
    print("Insertion order:", input);
    bool inserted = true;
    for (int key : input) inserted &= tree.insert(key);
    check(inserted, "12 distinct integers inserted");
    print("Inorder:", tree.inorder());
    print("Preorder:", tree.preorder());
    print("Postorder:", tree.postorder());
    check(tree.inorder() == std::vector<int>(
        {8,14,19,26,31,35,40,48,61,66,72,90}), "inorder matches expected");
    check(tree.preorder() == std::vector<int>(
        {48,26,14,8,19,35,31,40,72,61,66,90}), "preorder matches expected");
    check(tree.postorder() == std::vector<int>(
        {8,19,14,31,40,35,26,66,61,90,72,48}), "postorder matches expected");
    std::cout << "Initial height: " << tree.height() << '\n';
    check(tree.height() == 3, "initial height is 3 edges");
    check(tree.search(66), "search 66: found");
    check(!tree.search(55), "search 55: not found");
    check(!tree.insert(48) && tree.inorder().size() == 12,
          "duplicate insertion rejected");
    demonstrateRemoval(tree, 8, "leaf",
        {14,19,26,31,35,40,48,61,66,72,90});
    demonstrateRemoval(tree, 61, "one child: 66",
        {14,19,26,31,35,40,48,66,72,90});
    demonstrateRemoval(tree, 48, "two children; successor: 66",
        {14,19,26,31,35,40,66,72,90});
    const auto before = tree.inorder();
    check(!tree.remove(999) && tree.inorder() == before,
          "missing removal leaves tree unchanged");
    print("Final preorder:", tree.preorder());
    print("Final postorder:", tree.postorder());
    check(tree.preorder() == std::vector<int>(
        {66,26,14,19,35,31,40,72,90}), "final preorder matches expected");
    check(tree.postorder() == std::vector<int>(
        {19,14,31,40,35,26,90,72,66}), "final postorder matches expected");

    std::cout << "\nAdditional edge cases\n";
    BinarySearchTree single;
    single.insert(5);
    check(single.height() == 0 && single.remove(5) && single.height() == -1,
          "remove the only node");
    single.insert(5);
    single.insert(3);
    check(single.remove(5) && single.inorder() == std::vector<int>({3}),
          "remove root with only a left child");
    BinarySearchTree chain;
    for (int i = 1; i <= 10; ++i) chain.insert(i);
    std::cout << "Height after sorted insertion of 1..10: "
              << chain.height() << '\n';
    check(chain.height() == 9 && chain.search(10),
          "sorted input creates a chain of height 9");
    check(chain.remove(1) && chain.search(2) && chain.height() == 8,
          "remove root with only a right child");
    bool emptied = true;
    for (int i = 2; i <= 10; ++i) emptied &= chain.remove(i);
    check(emptied && chain.inorder().empty() && chain.height() == -1,
          "remove all nodes");
    std::cout << "\nAll " << checks << " checks passed.\n";
}
