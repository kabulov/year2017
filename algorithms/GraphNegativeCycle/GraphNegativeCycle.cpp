#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <utility>

const int inf = static_cast<int>(1e9);
using graphtype = std::vector<std::vector<std::pair<int, int>>>;
graphtype graph, trangraph;
std::vector<int> used;
std::vector<int> component;
std::vector<int> isneg;
std::vector<int> topo;
std::vector<int> dist;

struct edge {
    int from, to, cost;
};

std::vector<edge> origedges, usefuledges;

void dfs_topo(int node) {
    used[node] = true;
    for (size_t iter = 0; iter < graph[node].size(); ++iter) {
        int to = graph[node][iter].first;
        if (!used[to]) {
            dfs_topo(to);
        }
    }
    topo.push_back(node);
}

void dfs_strong(int node, int number) {
    used[node] = true;
    component[node] = number;
    for (size_t iter = 0; iter < trangraph[node].size(); ++iter) {
        int to = trangraph[node][iter].first;
        if (!used[to]) {
            dfs_strong(to, number);
        }
    }
}

int main () {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int nodes, edges;
    std::cin >> nodes >> edges;

    graph.resize(nodes);
    trangraph.resize(nodes);

    origedges.resize(edges);

    for (int iter = 0; iter < edges; ++iter) {
        int from, to, cost;
        std::cin >> from >> to >> cost;
        --from;
        --to;
        origedges[iter].from = from;
        origedges[iter].to = to;
        origedges[iter].cost = cost;
        graph[from].push_back(std::make_pair(to, cost));
        trangraph[to].push_back(std::make_pair(from, cost));
    }

    used.resize(nodes, 0);
    for (int iter = 0; iter < nodes; ++iter) {
        if (!used[iter]) {
            dfs_topo(iter);
        }
    }

    std::reverse(topo.begin(), topo.end());
    std::fill(used.begin(), used.end(), 0);
    component.resize(nodes);
    int component_number = 0;
    for (int iter = 0; iter < nodes; ++iter) {
        int root = topo[iter];
        if (!used[root]) {
            dfs_strong(root, component_number);
            ++component_number;
        }
    }

    usefuledges.reserve(edges);
    for (int iter = 0; iter < edges; ++iter) {
        int from = origedges[iter].from;
        int to = origedges[iter].to;
        if (component[from] != component[to]) {
            continue;
        }
        int cost = origedges[iter].cost;
        edge new_edge;
        new_edge.from = from;
        new_edge.to = to;
        new_edge.cost = cost;
        usefuledges.push_back(new_edge);
    }

    dist.resize(nodes, 0);
    for (int iter = 0; iter + 1 < nodes; ++iter) {
        for (size_t pos = 0; pos < usefuledges.size(); ++pos) {
            int from = usefuledges[pos].from;
            int to = usefuledges[pos].to;
            int cost = usefuledges[pos].cost;
            if (dist[to] > dist[from] + cost) {
                dist[to] = std::max(dist[from] + cost, -inf);
            }
        }
    }

    isneg.resize(component_number, 0);
    for (size_t pos = 0; pos < usefuledges.size(); ++pos) {
        int from = usefuledges[pos].from;
        int to = usefuledges[pos].to;
        int cost = usefuledges[pos].cost;
        if (dist[to] > dist[from] + cost) {
            int cmp = component[to];
            isneg[cmp] = true;
        }
    }

    int minimal = -1;
    for (int iter = 0; iter < nodes; ++iter) {
        int cmp = component[iter];
        if (isneg[cmp]) {
            minimal = iter;
            break;
        }
    }

    if (minimal == -1) {
        std::cout << "NO";
    } else {
        std::cout << "YES\n" << minimal + 1;
    }

    return 0;
}
