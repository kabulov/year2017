
#include <string>
#include <algorithm>
#include <list>
#include <set>
#include <map>
#include <iostream>
#include <utility>
#include <random>
#include <chrono>
#include <cassert>
#include <vector>
#include <limits>


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


class level_data {
public:
    level_data() {}

    void init(int sz) {
        size = sz;
        // neighbors.resize(size);
        // tree_neighbors.resize(size);
        component_count = size;
    }

    ~level_data() {
        for (auto it = iterators.begin(); it != iterators.end(); ++it) {
            assert(it->second); // TODO : remove;
            delete it->second;
        }
    }

    int GetComponentCount() const {
        return component_count;
    }

    void set_iterator(int first, int second, ptr it) {
        iterators.insert(std::make_pair(std::make_pair(first, second), it));
    }

    void remove_iterator(int first, int second) {
        iterators.erase(std::make_pair(first, second));
    }

    ptr get_iterator(int first, int second) {
        auto it = iterators.find(std::make_pair(first, second));
        if (it == iterators.end()) {
            return nullptr;
        }
        return it->second;
    }

    void dec_component_count() {
        --component_count;
    }

    void inc_component_count() {
        ++component_count;
    }

    void add_neighbor(int node, int neighbor) {
        auto it = neighbors.find(node);
        if (it == neighbors.end()) {
            neighbors.insert(std::make_pair(node, std::set<int>()));
            it = neighbors.find(node);
        }
        it->second.insert(neighbor);
    }

    bool has_neighbor(int node, int neighbor) {
        auto it = neighbors.find(node);
        if (it == neighbors.end()) {
            return false;
        }
        return it->second.find(neighbor) != it->second.end();
    }

    void remove_neighbor(int node, int neighbor) {
        neighbors[node].erase(neighbor);
        if (neighbors[node].empty()) {
            neighbors.erase(node);
        }
    }

    int get_next_neighbor(int node) {
        auto it = neighbors.find(node);
        if (it == neighbors.end()) {
            return -1;
        }
        if (it->second.empty()) {
            return -1;
        } else {
            return *(it->second.begin());
        }
    }

    bool has_tree_neighbor(int node, int neighbor) {
        auto it = tree_neighbors.find(node);
        if (it == tree_neighbors.end()) {
            return false;
        }
        return it->second.find(neighbor) != it->second.end();
    }

    void add_tree_neighbor(int node, int neighbor) {
        auto it = tree_neighbors.find(node);
        if (it == tree_neighbors.end()) {
            tree_neighbors.insert(std::make_pair(node, std::set<int>()));
            it = tree_neighbors.find(node);
        }
        it->second.insert(neighbor);
    }

    void remove_tree_neighbor(int node, int neighbor) {
        tree_neighbors[node].erase(neighbor);
        if (tree_neighbors[node].empty()) {
            tree_neighbors.erase(node);
        }
    }

private:
    int size;
    int component_count;
    // std::vector<std::set<int>> neighbors; // TODO : ML
    std::map<int, std::set<int>> neighbors;
    std::map<std::pair<int, int>, ptr> iterators;
    std::map<int, std::set<int>> tree_neighbors;
    // std::vector<std::set<int>> tree_neighbors; // TODO : ML ???
};

class DynamicGraph {
public:
    explicit DynamicGraph(int sz) {
        size = sz;
        levels = get_pow(size);
        data.resize(levels);
        generators.resize(levels);
        for (int i = 0; i < levels; ++i) {
            data[i].init(size);
        }
    }

    void AddEdge(int first, int second) {
        if (isbugedge(first, second)) { // TODO : DEBUG
            std::swap(first, first);
        }
        ptr first_iterator = data[0].get_iterator(first, first);
        ptr second_iterator = data[0].get_iterator(second, second);
        if (first_iterator == nullptr || second_iterator == nullptr) {
            if (first_iterator == nullptr && second_iterator == nullptr) {
                uint64_t pri = generators[0].Toss();
                ptr new_ff = new tree_node(first, first, pri);
                data[0].set_iterator(first, first, new_ff);

                pri = generators[0].Toss();
                ptr new_ss = new tree_node(second, second, pri);
                data[0].set_iterator(second, second, new_ss);

                pri = generators[0].Toss();
                ptr new_fs = new tree_node(first, second, pri);
                new_fs->adjacent = 1;
                new_fs->sum = 1;
                data[0].set_iterator(first, second, new_fs);
                data[0].add_tree_neighbor(first, second);

                pri = generators[0].Toss();
                ptr new_sf = new tree_node(second, first, pri);
                new_sf->adjacent = 1;
                new_sf->sum = 1;
                data[0].set_iterator(second, first, new_sf);
                data[0].add_tree_neighbor(second, first);

                merge(merge(new_ff, new_fs), merge(new_ss, new_sf));
                // TODO: treap_update for sum and ajdacent for added items ?
                data[0].dec_component_count();
            } else {
                if (first_iterator == nullptr) {
                    std::swap(first, second);
                    std::swap(first_iterator, second_iterator);
                }
                ptr first_reroot = reroot(first_iterator);

                uint64_t pri = generators[0].Toss();
                ptr new_fs = new tree_node(first, second, pri);
                new_fs->adjacent = 1;
                new_fs->sum = 1;
                data[0].set_iterator(first, second, new_fs);
                data[0].add_tree_neighbor(first, second);

                pri = generators[0].Toss();
                ptr new_ss = new tree_node(second, second, pri);
                data[0].set_iterator(second, second, new_ss);

                pri = generators[0].Toss();
                ptr new_sf = new tree_node(second, first, pri);
                new_sf->adjacent = 1;
                new_sf->sum = 1;
                data[0].set_iterator(second, first, new_sf);
                data[0].add_tree_neighbor(second, first);

                merge(merge(first_reroot, new_fs), merge(new_ss, new_sf));
                // TODO: treap_update for sum and ajdacent for added items ?
                data[0].dec_component_count();
            }
        } else {
            ptr first_root = get_root(first_iterator);
            ptr second_root = get_root(second_iterator);
            if (first_root != second_root) {
                ptr first_reroot = reroot(first_iterator, first_root);
                ptr second_reroot = reroot(second_iterator, second_root);

                uint64_t pri = generators[0].Toss();
                ptr new_fs = new tree_node(first, second, pri);
                new_fs->adjacent = 1;
                new_fs->sum = 1;
                data[0].set_iterator(first, second, new_fs);
                data[0].add_tree_neighbor(first, second);

                pri = generators[0].Toss();
                ptr new_sf = new tree_node(second, first, pri);
                new_sf->adjacent = 1;
                new_sf->sum = 1;
                data[0].set_iterator(second, first, new_sf);
                data[0].add_tree_neighbor(second, first);

                merge(merge(first_reroot, new_fs), merge(second_reroot, new_sf));
                data[0].dec_component_count();
            } else {
                first_iterator->aux_adjacent += 1; // TODO : aux_sum ?
                treap_update(first_iterator);
                second_iterator->aux_adjacent += 1; // TODO : aux_sum ?
                treap_update(second_iterator);

                assert(!data[0].has_neighbor(first, second));
                assert(!data[0].has_neighbor(second, first));
                data[0].add_neighbor(first, second);
                data[0].add_neighbor(second, first);
            }
        }
        // add to level 0 :
        // find left root
        // find right root
        // if in same tree:
        // add to aux[0]
        // else
        // reroot both and link at level[0]
    }

    bool isbugedge(int first, int second) {
        if (first > second) {
            std::swap(first, second);
        }
        return first == 0 && second == 6;
    }

    void RemoveEdge(int first, int second) {
        if (isbugedge(first, second)) { // TODO: DEBUG
            std::swap(first, first);
        }
        ptr is_tree_edge = data[0].get_iterator(first, second);
        if (is_tree_edge == nullptr) {
            int level = -1;
            for (int iter = 0; iter < levels; ++iter) {
                if (data[iter].has_neighbor(first, second)) {
                    data[iter].remove_neighbor(first, second);
                    data[iter].remove_neighbor(second, first);
                    level = iter;
                    break;
                }
            }
            assert(level >= 0);
            ptr first_iterator = data[level].get_iterator(first, first);
            ptr second_iterator = data[level].get_iterator(second, second);
            assert(first_iterator->aux_adjacent > 0);
            first_iterator->aux_adjacent -= 1;
            treap_update(first_iterator);
            assert(second_iterator->aux_adjacent > 0);
            second_iterator->aux_adjacent -= 1;
            treap_update(second_iterator);
        } else {
            int level = -1;
            for (int iter = 0; iter < levels; ++iter) {
                if (data[iter].has_tree_neighbor(first, second)) {
                    level = iter;
                    break;
                }
            }
            assert(level >= 0);
            assert(data[level].has_tree_neighbor(first, second));
            assert(data[level].has_tree_neighbor(second, first));
            data[level].remove_tree_neighbor(first, second);
            data[level].remove_tree_neighbor(second, first);
            for (int iter = 0; iter <= level; ++iter) { // TODO: < ?
                ptr first_node = data[iter].get_iterator(first, first);
                assert(first_node != nullptr);
                ptr first_reroot = reroot(first_node);
                assert(first_reroot != nullptr);
                ptr first_iterator = data[iter].get_iterator(first, second);
                ptr second_iterator = data[iter].get_iterator(second, first);
                assert(first_iterator != nullptr);
                assert(second_iterator != nullptr);
                int pos_fs = get_pos(first_iterator);
                int pos_sf = get_pos(second_iterator);
                assert(pos_fs < pos_sf);
                assert(0 <= pos_fs && 0 <= pos_sf);
                ptr first_root = get_root(first_iterator);
                ptr second_root = get_root(second_iterator);
                assert(first_root != nullptr);
                assert(second_root != nullptr);
                ptrpair one = split(first_root, pos_fs - 1);
                ptrpair two = split(one.second, pos_sf - pos_fs + 1);
                merge(one.first, two.second);
                ptrpair three = split(two.first, pos_sf - pos_fs);
                ptrpair four = split(three.first, 1);
                ptr new_second = four.second;
                assert(get_size(new_second) >= 1);
                // four.first == 1
                // three.second == 1
                // new_second >= 1
                // new_first >= 1
                assert(get_size(four.first) == 1);
                assert(four.first->edge_start == first);
                assert(four.first->edge_end == second);
                data[iter].remove_iterator(four.first->edge_start, four.first->edge_end);
                delete four.first;
                assert(get_size(three.second) == 1);
                assert(three.second->edge_start == second);
                assert(three.second->edge_end == first);
                data[iter].remove_iterator(three.second->edge_start, three.second->edge_end);
                delete three.second;
                data[iter].inc_component_count();
                // TODO: what to do with below ?????
//                if (get_size(new_second) == 1) {
//                    assert(new_second->edge_start == new_second->edge_end);
//                    data[iter].remove_iterator(new_second->edge_start, new_second->edge_end);
//                }
//                if (get_size(new_first) == 1) {
//                    assert(new_first->edge_start == new_first->edge_end);
//                    data[iter].remove_iterator(new_first->edge_start, new_first->edge_end);
//                }
                // TODO: all invariants and data structures ?
            }

            bool found = false;
            int found_level = level;
            int found_first = -1;
            int found_second = -1;
            for (int iter = level; iter >= 0; --iter) {
                // if found does nothing, just returns true
                found = replace(first, second, iter, found_first, found_second);
                if (found) {
                    if (0) {
                        ptr check_first = data[iter].get_iterator(first, first);
                        ptr check_found_first = data[iter].get_iterator(found_first, found_first);
                        ptr check_second = data[iter].get_iterator(second, second);
                        ptr check_found_second =
                            data[iter].get_iterator(found_second, found_second);
                        assert(get_root(check_first) == get_root(check_found_first));
                        assert(get_root(check_second) == get_root(check_found_second));
                    }
                    found_level = iter;
                    break;
                }
                ptr fst = data[iter].get_iterator(first, first);
                ptr scd = data[iter].get_iterator(second, second);
                ptr root_fst = get_root(fst);
                ptr root_scd = get_root(scd);
                // return here in the end
                // TODO: is this OK ?
                // TODO: is this OK ?
                // TODO: is this OK ?
                // TODO: is this OK ?
                if (get_size(root_fst) == 1) {
                    assert(fst == root_fst);
                    assert(fst->aux_adjacent == 0);
                    assert(root_fst->edge_start == root_fst->edge_end);
                    data[iter].remove_iterator(first, first);
                    delete fst;
                }
                if (get_size(root_scd) == 1) {
                    assert(scd == root_scd);
                    assert(scd->aux_adjacent == 0);
                    assert(root_scd->edge_start == root_scd->edge_end);
                    data[iter].remove_iterator(second, second);
                    delete scd;
                }
            }

            if (!found) {
                // DO NOTHING
            } else {
                if (isbugedge(found_first, found_second)) { // TODO: DEBUG
                    std::swap(found_first, found_first);
                }
                assert(data[found_level].has_neighbor(found_first, found_second));
                data[found_level].remove_neighbor(found_first, found_second);
                assert(data[found_level].has_neighbor(found_second, found_first));
                data[found_level].remove_neighbor(found_second, found_first);
                assert(!data[found_level].has_tree_neighbor(found_first, found_second));
                data[found_level].add_tree_neighbor(found_first, found_second);
                assert(!data[found_level].has_tree_neighbor(found_second, found_first));
                data[found_level].add_tree_neighbor(found_second, found_first);
                ptr one = data[found_level].get_iterator(found_first, found_first);
                ptr two = data[found_level].get_iterator(found_second, found_second);
                assert(one->aux_adjacent > 0);
                one->aux_adjacent -= 1;
                treap_update(one);
                assert(two->aux_adjacent > 0);
                two->aux_adjacent -= 1;
                treap_update(two);
                // TODO: may reroot one node treap

                one = reroot(one);
                two = reroot(two);

                uint64_t pri = generators[found_level].Toss();
                ptr new_one = new tree_node(found_first, found_second, pri);
                new_one->adjacent = 1;
                new_one->sum = 1;
                data[found_level].set_iterator(found_first, found_second, new_one);

                pri = generators[found_level].Toss();
                ptr new_two = new tree_node(found_second, found_first, pri);
                new_two->adjacent = 1;
                new_two->sum = 1;
                data[found_level].set_iterator(found_second, found_first, new_two);

                merge(merge(one, new_one), merge(two, new_two));
                data[found_level].dec_component_count();

                for (int iter = found_level - 1; iter >= 0; --iter) {
                    one = data[iter].get_iterator(found_first, found_first);
                    two = data[iter].get_iterator(found_second, found_second);
                    assert(one != nullptr);
                    assert(two != nullptr);
                    assert(get_root(one) != get_root(two));

                    one = reroot(one);
                    two = reroot(two);

                    uint64_t pri = generators[iter].Toss();
                    ptr new_one = new tree_node(found_first, found_second, pri);
                    data[iter].set_iterator(found_first, found_second, new_one);

                    pri = generators[iter].Toss();
                    ptr new_two = new tree_node(found_second, found_first, pri);
                    data[iter].set_iterator(found_second, found_first, new_two);

                    merge(merge(one, new_one), merge(two, new_two));
                    data[iter].dec_component_count();
                }
            }
        }
        // find level i
        // remove from level <= i
        // pick smaller from splitted parts
        // push tree edges of smaller to level i + 1
        // rebalance edge = null
        // while (i >= 0 && !level[i].rebalance(i, smaller part, bigger part, rabalance_edge)) {
        //      --i;
        // }
        // if (i >= 0) {
        //      remove rebalance edge from aux of level i
        //      add rebalance edge to tree of level <= i
        // }
    }

    int GetComponentsNumber() const {
        return data[0].GetComponentCount();
    }

private:
    int size;
    int levels;

    int get_pow(int size) {
        int power = 0;
        int result = 1;
        while (result < size) {
            result *= 2;
            ++power;
        }
        return power + 1;  // TODO : result ?
    }

    std::vector<level_data> data;
    std::vector<Generator> generators;

    bool replace(int first, int second, int level, int& found_first, int& found_second) {
        ptr one = data[level].get_iterator(first, first);
        ptr two = data[level].get_iterator(second, second);
        ptr one_root = get_root(one);
        ptr two_root = get_root(two);
        int one_size = get_size(one_root);
        int two_size = get_size(two_root);
        if (one_size > two_size) {
            std::swap(first, second);
            std::swap(one, two);
            std::swap(one_root, two_root);
            std::swap(one_size, two_size);
        }

        std::vector<ptr> tree_edges;
        find_tree_edges(one_root, tree_edges);
        for (size_t iter = 0; iter < tree_edges.size(); ++iter) {
            ptr edge = tree_edges[iter];
            if (isbugedge(edge->edge_start, edge->edge_end)) { // TODO: DEBUG
                std::swap(edge, edge);
            }
            assert(edge->adjacent == 1);
            edge->adjacent = 0;
            treap_update(edge);
            data[level].remove_tree_neighbor(edge->edge_start, edge->edge_end);
            data[level + 1].add_tree_neighbor(edge->edge_start, edge->edge_end);
            if (!data[level + 1].has_tree_neighbor(edge->edge_end, edge->edge_start)) {
                add_tree_edge_to_level(edge->edge_start, edge->edge_end, level + 1);
                // just merges treaps and adds iterator
            }
        }
        // get tree edges of current level
        // remove from tree of current level
        // add to tree of next level
        // add to neighbors of next level
        // remove from neighbors of current level
        // adjust iterators in both levels

        // get aux edges of current level of current tree
        std::vector<ptr> aux_edges;
        find_aux_edges(one_root, aux_edges);
        for (size_t iter = 0; iter < aux_edges.size(); ++iter) {
            ptr node = aux_edges[iter];
            // assert(node->aux_adjacent > 0); we may iterate it twice
            assert(node->edge_start == node->edge_end);
            assert(get_root(node) == one_root);
            int other_node = data[level].get_next_neighbor(node->edge_start);
            assert(node->edge_start != other_node);
            // assert(node->aux_adjacent > 0 && other_node != -1); // TODO:  TODO: TODO: CHECK
            if (node->aux_adjacent > 0) {
                assert(other_node != -1);
            }
            while (other_node != -1) {
                if (isbugedge(node->edge_start, other_node)) { // TODO: DEBUG
                    std::swap(other_node, other_node);
                }
                ptr other_ptr = data[level].get_iterator(other_node, other_node);
                assert(other_ptr != nullptr);
                assert(other_ptr->aux_adjacent > 0);
                ptr other_root = get_root(other_ptr);
                if (other_root == one_root) {
                    // assert(data[level].has_neighbor(node->edge_start, other_node));
                    data[level].remove_neighbor(node->edge_start, other_node);
                    // assert(!data[level].has_neighbor(node->edge_start, other_node));
                    data[level].remove_neighbor(other_node, node->edge_start);
                    // assert(!data[level + 1].has_neighbor(node->edge_start, other_node));
                    data[level + 1].add_neighbor(node->edge_start, other_node);
                    // assert(data[level + 1].has_neighbor(node->edge_start, other_node));
                    data[level + 1].add_neighbor(other_node, node->edge_start);
                    assert(node->aux_adjacent > 0);
                    int old_value = get_aux_sum(node); // remove
                    int old_root_value = get_aux_sum(other_root); // remove
                    node->aux_adjacent -= 1;
                    treap_update(node);
                    assert(old_value > get_aux_sum(node));
                    assert(old_root_value > get_aux_sum(other_root));
                    assert(other_ptr->aux_adjacent > 0);
                    other_ptr->aux_adjacent -= 1;
                    treap_update(other_ptr);
                    ptr node_next_level =
                        data[level + 1].get_iterator(node->edge_start, node->edge_start);
                    ptr other_ptr_next_level = data[level + 1].get_iterator(other_node, other_node);
                    assert(node_next_level != nullptr);
                    assert(other_ptr_next_level != nullptr);
                    node_next_level->aux_adjacent += 1;
                    treap_update(node_next_level);
                    other_ptr_next_level->aux_adjacent += 1;
                    treap_update(other_ptr_next_level);
                } else {
                    found_first = node->edge_start;
                    found_second = other_node;
                    return true;
                }
                other_node = data[level].get_next_neighbor(node->edge_start);
            }
        }
        return false;
    }

    void add_tree_edge_to_level(int first, int second, int level) {
        // just merges treaps and adds iterator
        // 1. first is null
        // 2. second is null
        // 3. both not null : check - must be not connected
        ptr first_iterator = data[level].get_iterator(first, first);
        ptr second_iterator = data[level].get_iterator(second, second);
        if (first_iterator == nullptr || second_iterator == nullptr) {
            if (first_iterator == nullptr && second_iterator == nullptr) {
                uint64_t pri = generators[level].Toss();
                ptr new_ff = new tree_node(first, first, pri);
                data[level].set_iterator(first, first, new_ff);

                pri = generators[level].Toss();
                ptr new_ss = new tree_node(second, second, pri);
                data[level].set_iterator(second, second, new_ss);

                pri = generators[level].Toss();
                ptr new_fs = new tree_node(first, second, pri);
                new_fs->adjacent = 1;
                new_fs->sum = 1;
                data[level].set_iterator(first, second, new_fs);

                pri = generators[level].Toss();
                ptr new_sf = new tree_node(second, first, pri);
                new_sf->adjacent = 1;
                new_sf->sum = 1;
                data[level].set_iterator(second, first, new_sf);

                merge(merge(new_ff, new_fs), merge(new_ss, new_sf));
                // TODO: treap_update for sum and ajdacent for added items ?
                data[level].dec_component_count();
            } else {
                if (first_iterator == nullptr) {
                    std::swap(first, second);
                    std::swap(first_iterator, second_iterator);
                }
                ptr first_reroot = reroot(first_iterator);

                uint64_t pri = generators[level].Toss();
                ptr new_fs = new tree_node(first, second, pri);
                new_fs->adjacent = 1;
                new_fs->sum = 1;
                data[level].set_iterator(first, second, new_fs);

                pri = generators[level].Toss();
                ptr new_ss = new tree_node(second, second, pri);
                data[level].set_iterator(second, second, new_ss);

                pri = generators[level].Toss();
                ptr new_sf = new tree_node(second, first, pri);
                new_sf->adjacent = 1;
                new_sf->sum = 1;
                data[level].set_iterator(second, first, new_sf);

                merge(merge(first_reroot, new_fs), merge(new_ss, new_sf));
                // TODO: treap_update for sum and ajdacent for added items ?
                data[level].dec_component_count();
            }
        } else {
            ptr first_root = get_root(first_iterator);
            ptr second_root = get_root(second_iterator);
            assert(first_root != second_root);

            ptr first_reroot = reroot(first_iterator, first_root);
            ptr second_reroot = reroot(second_iterator, second_root);

            uint64_t pri = generators[level].Toss();
            ptr new_fs = new tree_node(first, second, pri);
            new_fs->adjacent = 1;
            new_fs->sum = 1;
            data[level].set_iterator(first, second, new_fs);

            pri = generators[level].Toss();
            ptr new_sf = new tree_node(second, first, pri);
            new_sf->adjacent = 1;
            new_sf->sum = 1;
            data[level].set_iterator(second, first, new_sf);

            merge(merge(first_reroot, new_fs), merge(second_reroot, new_sf));
            data[level].dec_component_count();
        }
    }
};
