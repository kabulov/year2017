//
// Created by kkabulov on 12/11/2017.
//

// TODO: include treap.h code here before sending to system

#include "treap.h"
#include <list>
#include <set>
#include <map>
#include <string>
#include <algorithm>

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
                ptr new_first = merge(one.first, two.second);
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
                        ptr check_found_second = data[iter].get_iterator(found_second, found_second);
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
                add_tree_edge_to_level(edge->edge_start, edge->edge_end, level + 1); // just merges treaps and adds iterator
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
                    ptr node_next_level = data[level + 1].get_iterator(node->edge_start, node->edge_start);
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
            if (first_iterator == nullptr && second_iterator == nullptr ) {
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

int solve () {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int nodes, edges;
    std::cin >> nodes >> edges;

    DynamicGraph graph(nodes);
    for (int iter = 0; iter < edges; ++iter) {
        std::string op;
        std::cin >> op;
        int first, second;
        if (op[0] != '?') {
            std::cin >> first >> second;
            --first;
            --second;
        }
        // std::cout << op[0] << " " << first << " " << second << "\n";
        if (op[0] == '+') {
            graph.AddEdge(first, second);
        } else if (op[0] == '-') {
            graph.RemoveEdge(first, second);
        } else { // op[0] == '?'
            std::cout << graph.GetComponentsNumber() << "\n";
        }
    }

    return 0;
}
