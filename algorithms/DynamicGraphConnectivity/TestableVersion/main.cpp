#include <iostream>
#include <utility>
#include <random>
#include <chrono>
#include <cassert>
#include <vector>

#include "treap.h"
#include "solution.h"

// OK: check sizes (1 indexation in treap)
// OK: check sum calculation (set adjacent as well as sum)
// OK: check parents
// OK: check pointer invalidation
// OK: check expected depth of tree
// OK: check array cut and merge

// TODO: turn to edges from chars
// TODO: test reroot
// TODO: test link
// TODO: test cut

int simple_test() {
    tree_node *root = nullptr;
    Generator generator;
    std::string str;
    insert(root, 1, 'a', generator.Toss());
    insert(root, 2, 'b', generator.Toss());
    insert(root, 3, 'c', generator.Toss());
    insert(root, 4, 'd', generator.Toss());
    walk(root, str);
    // std::cout << str;
    assert(str == "abcd");
    erase(root, 4);
    erase(root, 3);
    erase(root, 2);
    erase(root, 1);
    walk(root, str);
    // std::cout << str;
    assert(str == "abcd");
    clear(root);
}

int test_insert_erase() {
    std::string str;
    tree_node *root = nullptr;
    Generator generator;
    //srand(time(nullptr));
    for (int i = 0; i < 10000; ++i) {
        assert(get_size(root) == str.size());
        int pos = generator.Toss() % (str.size() + 1LL);
        if (generator.Toss() & 1) {
            char ch = 'a' + generator.Toss() % 26LL;
            str.insert(pos, 1, ch);
            insert(root, pos + 1, ch, generator.Toss());
        } else {
            if (!str.empty()) {
                str.erase(pos, 1);
                erase(root, pos + 1);
            }
        }
        std::string buf;
        walk(root, buf);
        if (buf != str) {
            std::cout << "fail: treap vs string : " << root << " vs " << buf << std::endl;
            return 1;
        }
        assert(get_size(root) == str.size());
        //std::cout << i << " " << str << "\n";
    }
    clear(root);
    assert(get_size(root) == 0);
    std::cout << "OK" << std::endl;
    return 0;
}

void test1() {
    std::cout << "test1 begin\n";
    for (int i = 0; i < 10; ++i) {
        std::cout << i << " ";
        test_insert_erase();
    }
    std::cout << "test1 finish\n";
}

int test_sum_interval() {
    std::vector<int> data;
    Generator generator;
    ptr root = nullptr;
    for (int i = 0; i < 1000; ++i) {
        if (generator.Toss() & 1) {
            int pos = generator.Toss() % (data.size() + 1LL);
            int value = generator.Toss(); // !!! change param if you want
            data.insert(data.begin() + pos, value);
            insert(root, pos + 1, value, generator.Toss());
        } else {
            if (!data.empty()) {
                int pos = generator.Toss() % (1LL*data.size());
                data.erase(data.begin() + pos);
                erase(root, pos + 1);
            }
        }
        int sum = 0;
        for (int elem : data){
            sum += elem;
        }
        int other = get_sum(root);
        if (sum != other) {
            std::cout << "fail: data vs treap " << sum << " " << other << std::endl;
            for (int elem: data) {
                std::cout << elem << " ";
            }
            std::cout << std::endl;
            assert(sum == other);
            return 1;
        }
        if (!data.empty()) {
            for (int iter = 0; iter < 100; ++iter) {
                int left = generator.Toss() % data.size();
                int right = left + generator.Toss() % (data.size() - left);
                assert(left <= right);
                int sum = 0;
                for (int d = left; d <= right; ++d) {
                    sum += data[d];
                }
                int other = sum_interval(root, left + 1, right + 1);
                if (sum != other) {
                    std::cout << "fail: data vs treap " << sum << " " << other << std::endl;
                    std::cout << "interval : " << left << " " << right << std::endl;
                    for (int elem: data) {
                        std::cout << elem << " ";
                    }
                    std::cout << std::endl;
                    assert(sum == other);
                    return 1;
                }
            }
        }
    }
    clear(root);
    assert(get_size(root) == 0);
    std::cout << "OK" << std::endl;
    return 0;
}

void test2() {
    std::cout << "test2 begin\n";
    for (int i = 0; i < 10; ++i) {
        std::cout << i << " ";
        test_sum_interval();
    }
    std::cout << "test2 finish\n";
}

int test_parent(int factor) {
    Generator generator;
    for (int i = 0; i < 100; ++i) {
        int size = generator.Toss() % 1000 + 1;
        ptr root = generate_random_treap(size * factor);
        check_parent(root);
        clear(root);
        assert(get_size(root) == 0);
    }
    std::cout << "OK" << std::endl;
}

void test3() {
    std::cout << "test3 begin\n";
    for (int i = 0; i < 10; ++i) {
        std::cout << i << " ";
        test_parent(i + 1);
    }
    std::cout << "test3 finish\n";
}

int test_pointer_invalidation(int factor) {
    ptr root = nullptr;
    Generator generator;
    std::vector<ptr> iterators;
    std::vector<char> data;
    for (int iter = 0; iter < 1000 * factor; ++iter) {
        if (generator.Toss() & 1) {
            int pos = (generator.Toss() % (get_size(root) + 1)) + 1;
            char ch = 'a' + generator.Toss() % 26;
            ptr iterator = insert(root, pos, ch, generator.Toss());
            iterators.insert(iterators.begin() + pos - 1, iterator);
            data.insert(data.begin() + pos - 1, ch);
        } else {
            if (!iterators.empty()) {
                int pos = (generator.Toss() % get_size(root)) + 1;
                erase(root, pos);
                iterators.erase(iterators.begin() + pos - 1);
                data.erase(data.begin() + pos - 1);
            }
        }
        for (int pos = 0; pos < iterators.size(); ++pos) {
            assert(get_pos(iterators[pos]) == pos + 1);
            assert(data[pos] == iterators[pos]->value);
            assert(data[pos] == get_char_at(root, pos + 1));
            assert(iterators[pos] == get_iterator_at(root, pos + 1));
        }
    }
    clear(root);
    assert(get_size(root) == 0);
    std::cout << "OK" << std::endl;
    return 0;
}

void test4() {
    std::cout << "test4 begin\n";
    for (int i = 0; i < 10; ++i) {
        std::cout << i << " ";
        test_pointer_invalidation(i);
    }
    std::cout << "test4 finish\n";
}

int test_depth(int factor) {
    for (int iter = 1; iter < 100 * factor; ++iter) {
        ptr root = generate_random_treap(iter + 1);
        assert(get_depth(root) <= 5.0 * log(iter + 1.0));
        clear(root);
        assert(get_size(root) == 0);
    }
    for (int iter = 0; iter < 100; ++iter) {
        ptr root = generate_random_treap(1000.0 * factor);
        assert(get_depth(root) <= 5.0 * log(1000.0 * factor));
        clear(root);
        assert(get_size(root) == 0);
    }
    std::cout << "OK" << std::endl;
    return 1;
}

void test5() {
    std::cout << "test5 begin\n";
    for (int i = 0; i < 10; ++i) {
        std::cout << i << " ";
        test_depth(i + 1);
    }
    std::cout << "test5 finish\n";
}

int test_cut_merge(int size) {
    Generator generator;
    std::vector<char> data(size);
    for (int i = 0; i < size; ++i) {
        data[i] = 'a' + generator.Toss() % 26;
    }
    ptr root = nullptr;
    for (int i = 0; i < size; ++i) {
        insert(root, i + 1, data[i], generator.Toss());
    }
    for (int iter = 0; iter < 100; ++iter) {
        int pos = generator.Toss() % size;
        std::reverse(data.begin(), data.end());
        std::reverse(data.begin(), data.begin() + data.size() - 1 - pos);
        std::reverse(data.begin() + data.size() - 1 - pos, data.end());
        ptrpair splitted = split(root, pos + 1);
        root = merge(splitted.second, splitted.first);
        std::string str;
        walk(root, str);
        for (int i = 0; i < size; ++i) {
            assert(str[i] == data[i]);
        }
    }
    std::cout << "OK" << std::endl;
    return 0;
}

void test6() {
    std::cout << "test6 begin\n";
    for (int i = 0; i < 10; ++i) {
        std::cout << i << " ";
        test_cut_merge(i + 1);
    }
    for (int i = 0; i < 10; ++i) {
        std::cout << i + 10 << " ";
        test_cut_merge((i + 1) * 100);
    }
    for (int i = 0; i < 10; ++i) {
        std::cout << i + 20 << " ";
        test_cut_merge((i + 1) * 10000);
    }
    std::cout << "test6 finish\n";
}

std::vector<std::vector<int>> graph;
std::vector<int> used;

void dfs(int node) {
    used[node] = true;
    for (int i = 0; i < graph[i].size(); ++i) {
        if (graph[node][i] && !used[i]) {
            dfs(i);
        }
    }
}

int test10() {
    int size = 100;
    graph.resize(size, std::vector<int>(size, 0));
    used.resize(size, 0);

    Generator generator;
    DynamicGraph mygraph(size);

    std::vector<std::vector<int>> input;

    int tests = 10000;
    for (int t = 0; t < tests; ++t) {
        int op = generator.Toss() % 3;
        int i, j;
        if (op == 0) {
            std::vector<std::pair<int, int>> some;
            for (i = 0; i < size; ++i) {
                for (j = 0; j < size; ++j) {
                    if (i == j) {
                        continue;
                    }
                    if (!graph[i][j]) {
                        some.push_back({i, j});
                    }
                }
            }
            if (some.empty()) {
                continue;
            }
            int pos = generator.Toss() % some.size();
            i = some[pos].first;
            j = some[pos].second;
            graph[i][j] = graph[j][i] = 1;
        } else if (op == 1) {
            std::vector<std::pair<int, int>> some;
            for (i = 0; i < size; ++i) {
                for (j = 0; j < size; ++j) {
                    if (i == j) {
                        continue;
                    }
                    if (graph[i][j]) {
                        some.push_back({i, j});
                    }
                }
            }
            if (some.empty()) {
                continue;
            }
            int pos = generator.Toss() % some.size();
            i = some[pos].first;
            j = some[pos].second;
            graph[i][j] = graph[j][i] = 0;
        } else {
            do {
                i = generator.Toss() % size;
                j = generator.Toss() % size;
            } while (i == j);
        }
        input.push_back({op, i, j});
    }
    freopen("badinput.txt", "w", stdout);
    std::cout << size << " " << input.size() << std::endl;
    for (int iter = 0; iter < input.size(); ++iter) {
        int op, i, j;
        op = input[iter][0];
        i = input[iter][1] + 1;
        j = input[iter][2] + 1;
        if (op == 0) {
            std::cout << "+" << " " << i << " " << j;
        } else if (op == 1) {
            std::cout << "-" << " " << i << " " << j;
        } else {
            std::cout << "?";
        }
        std::cout << "\n";
    }
    fflush(stdout);
    for (int t = 0; t < size; ++t) {
        std::fill(graph[t].begin(), graph[t].end(), 0);
    }
    for (int t = 0; t < input.size(); ++t) {
        int op, i, j;
        op = input[t][0];
        i = input[t][1];
        j = input[t][2];
        if (op == 0) {
            graph[i][j] = graph[j][i] = 1;
            mygraph.AddEdge(i, j);
        } else if (op == 1) {
            graph[i][j] = graph[j][i] = 0;
            mygraph.RemoveEdge(i, j);
        } else {
            std::fill(used.begin(), used.end(), 0);
            int cnt = 0;
            for (int iter = 0; iter < size; ++iter) {
                if (!used[iter]) {
                    ++cnt;
                    dfs(iter);
                }
            }
            int mycnt = mygraph.GetComponentsNumber();
            if (cnt != mycnt) {
                return 1;
            }
        }
    }

    return 0;
}

void testAll() {
    simple_test();
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test10();
}


int main() {

    if (0) {
        testAll();
    } else if (0){
        test6();
    } else if (0) {
        solve();
    } else {
        int ans = test10();
        if (ans == 1) {
            std::cout << "ERROR" << std::endl;
            return 1;
        }
    }

    return 0;
}

