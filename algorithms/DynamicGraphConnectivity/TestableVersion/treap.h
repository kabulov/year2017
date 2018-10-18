
#ifndef DYNAMIC_CONNECTIVITY_TREAP_H
#define DYNAMIC_CONNECTIVITY_TREAP_H

#include <string>
#include <algorithm>

// TODO: add asserts

struct Generator {
    std::mt19937_64 generator;
    std::uniform_int_distribution<uint64_t> distribution;
    Generator() :
            generator(std::chrono::high_resolution_clock::now().time_since_epoch().count()),
            distribution(0, std::numeric_limits<uint64_t>::max())
    {}

    uint64_t Toss(){
        return distribution(generator);
    }
};

struct tree_node {
    tree_node *left, *right, *parent;
    uint64_t priority;
    int size;
    int adjacent;
    int sum;
    int aux_adjacent;
    int aux_sum;
    int edge_start;
    int edge_end;

    explicit tree_node(int start, int end, uint64_t prior) {
        priority = prior;
        size = 1;
        adjacent = 0;
        sum = 0;
        aux_adjacent = 0;
        aux_sum = 0;
        edge_start = start;
        edge_end = end;
        left = nullptr;
        right = nullptr;
        parent = nullptr;
    }

    // for testing
    char value;
    explicit tree_node(char ch, uint64_t prior) {
        value = ch;
        priority = prior;
        size = 1;
        sum = 0;
        adjacent = 0;
        aux_adjacent = 0;
        aux_sum = 0;
        left = right = parent = nullptr;
    }
};

using ptr = tree_node*;
using ptrpair = std::pair<ptr, ptr>;

int get_size(ptr node) {
    return node == nullptr ? 0 : node->size;
}

int get_sum(ptr node) {
    return node == nullptr ? 0 : node->sum;
}

int get_aux_sum(ptr node) {
    return node == nullptr ? 0 : node->aux_sum;
}

void update(ptr node) {
    if (node != nullptr) {
        node->size = get_size(node->left) + get_size(node->right) + 1;
        node->sum = get_sum(node->left) + get_sum(node->right) + node->adjacent;
        node->aux_sum = get_aux_sum(node->left) + get_aux_sum(node->right) + node->aux_adjacent;
    }
}

//void update_adjacent(ptr node) {
//    if (node != nullptr) {
//        node->sum = get_sum(node->left) + get_sum(node->right) + node->adjacent;
//    }
//}
//
//void update_aux_ajdacent(ptr node) {
//    if (node != nullptr) {
//        node->aux_sum = get_aux_sum(node->left) + get_aux_sum(node->right) + node->aux_adjacent;
//    }
//}

ptr merge(ptr left, ptr right) {
    if (left == nullptr || right == nullptr) {
        return right == nullptr ? left : right;
    }
    if (left->priority > right->priority) {
        left->right = merge(left->right, right);
        update(left);
        if (left->right) {
            left->right->parent = left;
        }
        return left;
    } else {
        right->left = merge(left, right->left);
        update(right);
        if (right->left) {
            right->left->parent = right;
        }
        return right;
    }
}

std::pair<ptr, ptr> split(ptr root, int key) {
    if (root == nullptr) {
        return {nullptr, nullptr};
    }
    int root_key = get_size(root->left) + 1;    //
    if (root_key <= key) {   //
        ptrpair splitted = split(root->right, key - root_key); //
        root->right = splitted.first;
        update(root);   //
        if (splitted.first) {
            splitted.first->parent = root;
        }
        if (splitted.second) {
            splitted.second->parent = nullptr;
        }
        return std::make_pair(root, splitted.second);
    } else {
        ptrpair splitted = split(root->left, key);
        root->left = splitted.second;
        update(root);   //
        if (splitted.second) {
            splitted.second->parent = root;
        }
        if (splitted.first) {
            splitted.first->parent = nullptr;
        }
        return std::make_pair(splitted.first, root);
    }
}

void clear(ptr& root) {
    if (root == nullptr) {
        return;
    }
    clear(root->left);
    clear(root->right);
    delete root;
    root = nullptr;
}

bool is_right_child(ptr parent, ptr child) {
    assert(parent); // remove
    assert(child);  // remove
    return parent->right == child;
}

int get_pos(ptr iterator) {
    assert(iterator);  // remove
    int size = get_size(iterator->left) + 1;
    while (iterator) {
        if (iterator->parent && is_right_child(iterator->parent, iterator)) {
            size += get_size(iterator->parent->left) + 1;
        }
        iterator = iterator->parent;
    }
    return size;
}

ptr get_root(ptr iterator) {
    assert(iterator); // remove
    while (iterator->parent) {
        iterator = iterator->parent;
    }
    return iterator;
}

//ptr find_min(ptr root) {
//    assert(root); // remove
//    while (root->left) {
//        root = root->left;
//    }
//    return root;
//}
//
//ptr find_max(ptr root) {
//    assert(root); // remove
//    while (root->right) {
//        root = root->right;
//    }
//    return root;
//}

// TODO: test reroot
ptr reroot(ptr iterator, ptr root = nullptr) {
    assert(iterator); // remove
    if (!root) {
        root = get_root(iterator);
    }
    // ptr left = find_min(root);
    // if (left == iterator) {
    //    return root;
    // }
    int pos = get_pos(iterator);
    ptrpair splitted = split(root, pos - 1);
    return merge(splitted.second, splitted.first);
}

ptr treap_update(ptr iterator) {
    assert(iterator);
    ptr last;
    while (iterator) {
        update(iterator);
        last = iterator;
        iterator = iterator->parent;
    }
    return last;
}

void find_tree_edges(ptr root, std::vector<ptr>& vec) {
    if (root) {
        assert(root->adjacent >= 0);
        assert(get_sum(root) >= root->adjacent);
        if (root->adjacent) {
            vec.push_back(root);
        }
        if (get_sum(root->left)) {
            find_tree_edges(root->left, vec);
        }
        if (get_sum(root->right)) {
            find_tree_edges(root->right, vec);
        }
    }
}

void find_aux_edges(ptr root, std::vector<ptr>& vec) {
    if (root) {
        assert(root->aux_adjacent >= 0);
        assert(get_aux_sum(root) >= root->aux_adjacent);
        if (root->aux_adjacent) {
            vec.push_back(root);
        }
        if (get_aux_sum(root->left)) {
            find_aux_edges(root->left, vec);
        }
        if (get_aux_sum(root->right)) {
            find_aux_edges(root->right, vec);
        }
    }
}

// for TESTing purposes

int get_depth(ptr root) {
    if (root == nullptr) {
        return 0;
    }
    return std::max(get_depth(root->left), get_depth(root->right)) + 1;
}

ptr insert(ptr &root, int pos, char key, uint64_t pr) {
    ptrpair splitted = split(root, pos - 1);
    ptr new_node = new tree_node(key, pr);
    root = merge(merge(splitted.first, new_node), splitted.second);
    return new_node;
}

ptr insert(ptr &root, int pos, int val, uint64_t pr) {
    ptrpair splitted = split(root, pos - 1);
    ptr new_node = new tree_node('a', pr);
    new_node->adjacent = val;
    new_node->sum = val;
    root = merge(merge(splitted.first, new_node), splitted.second);
    return new_node;
}

ptr insert(ptr &root, int pos, uint64_t pr) {
    ptrpair splitted = split(root, pos - 1);
    ptr new_node = new tree_node('a', pr);
    root = merge(merge(splitted.first, new_node), splitted.second);
    return new_node;
}

void erase(ptr &root, int pos) {
    ptrpair one = split(root, pos - 1);
    ptrpair two = split(one.second, 1);
    root = merge(one.first, two.second);
    delete two.first;
}

char get_char_at(ptr& root, int pos) {
    ptrpair one = split(root, pos - 1);
    ptrpair two = split(one.second, 1);
    char answer = two.first->value;
    root = merge(one.first, merge(two.first, two.second));
    return answer;
}

ptr get_iterator_at(ptr& root, int pos) {
    ptrpair one = split(root, pos - 1);
    ptrpair two = split(one.second, 1);
    ptr answer = two.first;
    root = merge(one.first, merge(two.first, two.second));
    return answer;
}

void walk(ptr root, std::string& str) {
    if (root == nullptr) {
        return;
    }
    walk(root->left, str);
    str.push_back(root->value);
    walk(root->right, str);
}

int sum_interval(ptr& root, int left, int right) {
    ptrpair one = split(root, left - 1);
    ptrpair two = split(one.second, right - left + 1);
    int answer = get_sum(two.first);
    root = merge(one.first, merge(two.first, two.second));
    return answer;
}

bool is_parent(ptr parent, ptr child) {
    if (child && parent) {
        return child->parent == parent;
    }
    return true;
}

bool check_parent_recursion(ptr root) {
    if (root) {
        bool one = is_parent(root, root->left);
        bool two = is_parent(root, root->right);
        if (one && two) {
            one = check_parent_recursion(root->left);
            two = check_parent_recursion(root->right);
            return one && two;
        } else {
            assert(one);
            assert(two);
            return false;
        }
    }
    return true;
}

void check_parent(ptr root) {
    assert(root->parent == nullptr);
    assert(check_parent_recursion(root));
}

ptr generate_random_treap(int size) {
    ptr root = nullptr;
    Generator generator;
    for (int iter = 0; iter < size; ++iter) {
        int pos = (generator.Toss() % (get_size(root) + 1)) + 1;
        insert(root, pos, generator.Toss());
    }
    return root;
}

#endif //DYNAMIC_CONNECTIVITY_TREAP_H
