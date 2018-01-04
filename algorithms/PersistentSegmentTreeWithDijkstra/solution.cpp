
#include <iostream>
#include <vector>
#include <set>
#include <utility>
#include <algorithm>

#include <cassert>
#include <queue>

const int64_t MOD = static_cast<int64_t>(1e9) + 7;
const int maxpower = static_cast<int>(1e5) + 30;

const std::pair<int64_t, int64_t> hash_value = {2, MOD};
const std::pair<int64_t, int64_t> hash_first = {27, 2000000011LL};
const std::pair<int64_t, int64_t> hash_second = {241, 2038074743LL};

struct node {
    int left_child;
    int right_child;
    int right_most;

    int64_t value;
    std::pair<int64_t, int64_t> hash;

    node() {
        value = 0;
        hash.first = 0;
        hash.second = 0;
    }
};

int nodes;
int edges;
int start;
int finish;

std::vector<int64_t> power_value;
std::vector<int64_t> power_first;
std::vector<int64_t> power_second;

std::vector<int> parent;
std::vector<int> root;
std::vector<node> tree;
std::vector<std::vector<std::pair<int, int>>> graph;

int last;

void read_input() {
    std::cin >> nodes >> edges;
    graph.resize(nodes + 1, std::vector<std::pair<int, int>>());
    for (int iter = 0; iter < edges; ++iter) {
        int fst, scd, dist;
        std::cin >> fst >> scd >> dist;
        graph[fst].push_back({scd, dist});
        graph[scd].push_back({fst, dist});
    }
    std::cin >> start >> finish;
}

void build(int& root_node, int left, int right) {
    ++last;
    root_node = last;
    tree[root_node] = node();
    if (left == right) {
        tree[root_node].right_most = left;
        tree[root_node].left_child = 0;
        tree[root_node].right_child = 0;
    } else {
        int mid = (left + right) / 2;
        build(tree[root_node].left_child, left, mid);
        build(tree[root_node].right_child, mid + 1, right);
        // TODO careful
        tree[root_node].right_most = std::max(tree[tree[root_node].left_child].right_most,
                                              tree[tree[root_node].right_child].right_most);
    }
}

void create_node_copy(int& root_node) {
    ++last;
    tree[last] = tree[root_node];
    root_node = last;
}

int64_t mul(int64_t fst, int64_t scd, int64_t mod) {
    return (fst * scd) % mod;
}

int64_t sum(int64_t fst, int64_t scd, int64_t mod) {
    return (fst + scd) % mod;
}

// TODO : optimize, pass mid
void update_tree_node(int root_node, int left, int right) {
    int mid = (left + right) / 2;
    tree[root_node].value = sum(
            tree[tree[root_node].left_child].value,
            mul(
                    power_value[mid - left + 1],
                    tree[tree[root_node].right_child].value,
                    hash_value.second
            ),
            hash_value.second
    );
    tree[root_node].hash.first = sum(
            tree[tree[root_node].left_child].hash.first,
            mul(
                    power_first[mid - left + 1],
                    tree[tree[root_node].right_child].hash.first,
                    hash_first.second
            ),
            hash_first.second
    );
    tree[root_node].hash.second = sum(
            tree[tree[root_node].left_child].hash.second,
            mul(
                    power_second[mid - left + 1],
                    tree[tree[root_node].right_child].hash.second,
                    hash_second.second
            ),
            hash_second.second
    );
    tree[root_node].right_most = std::max(tree[tree[root_node].left_child].right_most,
                                          tree[tree[root_node].right_child].right_most);
}

bool has_1_at(int root_node, int left, int right, int pos) {
    if (left == right) {
        assert(left == pos);
        return tree[root_node].value;
    }
    int mid = (left + right) / 2;
    if (pos <= mid) {
        return has_1_at(tree[root_node].left_child, left, mid, pos);
    } else {
        return has_1_at(tree[root_node].right_child, mid + 1, right, pos);
    }
}

void add_1_at(int& root_node, int left, int right, int pos) {
    create_node_copy(root_node);
    if (left == right) {
        assert(left == pos);
        assert(tree[root_node].value == 0);
        tree[root_node].value = 1;
        tree[root_node].hash.first = 1;
        tree[root_node].hash.second = 1;
        tree[root_node].right_most = -1; // == pos == right
    } else {
        int mid = (left + right) / 2;
        if (pos <= mid) {
            add_1_at(tree[root_node].left_child, left, mid, pos);
        } else {
            add_1_at(tree[root_node].right_child, mid + 1, right, pos);
        }
        update_tree_node(root_node, left, right);
    }
}

// assert here for right most != pos
int find_closest_0_right(int root_node, int left, int right, int pos) {
    if (left == right) {
        assert(tree[root_node].value == 0);
        return left;
    }
    int mid = (left + right) / 2;
    // TODO code deduplicate
    if (pos <= mid && tree[tree[root_node].left_child].right_most > pos) {
        return find_closest_0_right(tree[root_node].left_child, left, mid, pos);
    }
    assert(tree[tree[root_node].right_child].right_most > pos);
    return find_closest_0_right(tree[root_node].right_child, mid + 1, right, pos);
}

// TODO: check
void fill_with_zero(int zero_node, int& root_node, 
                    int left, int right, int left_zero, int right_zero) {
    if (left_zero == left && right_zero == right) {
        ++last;
        tree[last] = tree[zero_node];
        root_node = last;
        // TODO update
    } else {
        create_node_copy(root_node);
        int mid = (left + right) / 2;
        if (right_zero <= mid) {
            fill_with_zero(tree[zero_node].left_child,
                           tree[root_node].left_child, left, mid, left_zero, right_zero);
        } else if (mid + 1 <= left_zero) {
            fill_with_zero(tree[zero_node].right_child,
                           tree[root_node].right_child, mid + 1, right, left_zero, right_zero);
        } else {
            fill_with_zero(tree[zero_node].left_child,
                           tree[root_node].left_child, left, mid, left_zero, mid);
            fill_with_zero(tree[zero_node].right_child,
                           tree[root_node].right_child, mid + 1, right, mid + 1, right_zero);
        }
        update_tree_node(root_node, left, right);
        // TODO update
    }
}

void add(int &root_node, int one_pos) {
    if (!has_1_at(root_node, 0, maxpower - 1, one_pos)) {
        add_1_at(root_node, 0, maxpower - 1, one_pos);
    } else {
        int closest = find_closest_0_right(root_node, 0, maxpower - 1, one_pos);
        fill_with_zero(1, root_node, 0, maxpower - 1, one_pos, closest - 1);
        add_1_at(root_node, 0, maxpower - 1, closest);
    }
}

void init() {
    root.resize(nodes + 1);
    parent.resize(nodes + 1);

    const int someconst = 1; // TODO
    tree.resize(someconst * (nodes + 4 * edges) * 30 + 30 * maxpower);

    power_value.resize(maxpower);
    power_first.resize(maxpower);
    power_second.resize(maxpower);

    power_value[0] = 1;
    for (int iter = 1; iter < maxpower; ++iter) {
        power_value[iter] = (power_value[iter - 1] * hash_value.first) % hash_value.second;
    }

    power_first[0] = 1;
    for (int iter = 1; iter < maxpower; ++iter) {
        power_first[iter] = (power_first[iter - 1] * hash_first.first) % hash_first.second;
    }

    power_second[0] = 1;
    for (int iter = 1; iter < maxpower; ++iter) {
        power_second[iter] = (power_second[iter - 1] * hash_second.first) % hash_second.second;
    }

    root[start] = 1;
    last = 0;
    build(root[start], 0, maxpower - 1);

    for (int iter = 1; iter <= nodes; ++iter) {
        if (iter != start) {
            root[iter] = 1;
            add(root[iter], maxpower - 1);
        }
    }
}

bool hash_eq(const std::pair<int64_t, int64_t>& fst, const std::pair<int64_t, int64_t>& scd) {
    return fst.first == scd.first && fst.second == scd.second;
}

bool node_hash_eq(int fst, int scd) {
    return hash_eq(tree[fst].hash, tree[scd].hash) && tree[fst].value == tree[scd].value;
}

int comparator(int fst, int scd, int left, int right) {
    if (left == right) {
        return tree[fst].value > tree[scd].value;
    } else {
        int mid = (left + right) / 2;
        if (node_hash_eq(tree[fst].right_child, tree[scd].right_child)) {
            return comparator(tree[fst].left_child, tree[scd].left_child, left, mid);
        } else {
            return comparator(tree[fst].right_child, tree[scd].right_child, mid + 1, right);
        }
    }
}

struct  std_set_comparator {
    bool operator () (const std::pair<int, int>& fst, const std::pair<int, int>& scd) {
        return comparator(fst.first, scd.first, 0, maxpower - 1);
    }
};

void print_answer(int node) {
    std::vector<int> answer;
    std::cout << tree[root[node]].value << "\n";
    answer.push_back(node);
    while (node != start) {
        node = parent[node];
        answer.push_back(node);
    }
    std::cout << answer.size() << "\n";
    std::reverse(answer.begin(), answer.end());
    for (int elem: answer) {
        std::cout << elem << " ";
    }
}

void run() {
    std::vector<int> visited(nodes + 1, false);
    std::priority_queue<std::pair<int, int>,
            std::vector<std::pair<int, int>>,
            std_set_comparator> que;
    que.push(std::make_pair(root[start], start));
    while (!que.empty()) {
        int node = que.top().second;
        que.pop();
        if (node == finish) {
            print_answer(node);
            return;
        }
        if (visited[node]) {
            continue;
        }
        visited[node] = true;
        for (int iter = 0; iter < graph[node].size(); ++iter) {
            int other_node = graph[node][iter].first;
            int new_root = root[node];
            add(new_root, graph[node][iter].second);
            if (comparator(root[other_node], new_root, 0, maxpower - 1)) {
                root[other_node] = new_root;
                parent[other_node] = node;
                que.push(std::make_pair(root[other_node], other_node));
            }
        }
    }
    std::cout << "-1" << "\n";
}

int main() {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    read_input();
    init();
    run();

    return 0;
}

