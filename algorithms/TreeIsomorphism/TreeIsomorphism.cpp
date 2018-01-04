#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <random>
#include <stdexcept>

// types
int amount;
typedef std::vector<int> vi;
typedef std::vector<vi> vvi;
typedef std::vector<std::pair<int, int> > vp;
typedef std::vector<vp> vvp;


// input graph and parents
std::vector<vvi> tree(2);
// graph centers, <= 2 nodes
vvi center(2);


void read_input() {
  std::cin >> amount;
  for (vvi& one_tree : tree) {
    one_tree.resize(amount);
  }
  vi nodes(2);
  for (int tree_index = 0; tree_index < 2; ++tree_index) {
    for (int edge_index = 0; edge_index + 1 < amount; ++edge_index) {
      for (int node_index = 0; node_index < 2; ++node_index) {
        std::cin >> nodes[node_index];
        --nodes[node_index];
      }
      tree[tree_index][nodes[0]].push_back(nodes[1]);
      tree[tree_index][nodes[1]].push_back(nodes[0]);
    }
    // kind of a memory optimization ???
    for (int node = 0; node < amount; ++node) {
      size_t size = tree[tree_index][node].size();
      tree[tree_index][node].resize(size);
      // vi(tree[tree_index][node]).swap(tree[tree_index][node]);
    }
  }
}



class TreeCenterNodeFinder {
public:
  TreeCenterNodeFinder(int amount,
                       const vvi& tree,
                       int first_node,
                       int second_node);

  vi FindCenterNodes();

private:
  void dfs(int node, int parent);
  void saveCenterNodeNumbers();

  const vvi& tree_;

  int first_node_;
  int second_node_;

  vi stack_;
  int stack_size_;

  int first_center_;
  int second_center_;
};


TreeCenterNodeFinder::TreeCenterNodeFinder(int amount,
                                           const vvi &tree,
                                           int first_node,
                                           int second_node)
  : 
    tree_(tree),
    first_node_(first_node),
    second_node_(second_node),
    stack_(amount),
    stack_size_(0),
    first_center_(-1),
    second_center_(-1) {}


vi TreeCenterNodeFinder::FindCenterNodes() {
  dfs(first_node_, -1);
  if (first_center_ == -1) {
    throw std::runtime_error("No center found, TreeCenterNodeFinder::FindCenterNodes");
  }
  if (second_center_ == -1) {
    return {first_center_};
  } else {
    return {first_center_, second_center_};
  }
}


void TreeCenterNodeFinder::saveCenterNodeNumbers() {
  int center_index = stack_size_ / 2;
  first_center_ = stack_[center_index];
  if (stack_size_ % 2 == 0) {
    second_center_ = stack_[center_index - 1];
  }
}


void TreeCenterNodeFinder::dfs(int node, int parent) {
  // dont forget to pop stack when quitting
  stack_[stack_size_] = node;
  ++stack_size_;
  if (node == second_node_) {
    saveCenterNodeNumbers();
  } else {
    for (int child_node : tree_[node]) {
      if (child_node == parent) {
        continue;
      }
      dfs(child_node, node);
    }
  }
  --stack_size_;
}


class TreeCenterFinder {
public:
  TreeCenterFinder(int amount, const vvi& tree);
  vi FindCenter();
  
private:
  void dfs(int node, int parent);
  int farmost();

  
  const vvi& tree_;
  vi distance_;
};


TreeCenterFinder::TreeCenterFinder(int amount, const vvi& tree)
  : 
    tree_(tree),
    distance_(amount) {}


vi TreeCenterFinder::FindCenter() {
  std::fill(distance_.begin(), distance_.end(), 0);
  dfs(0, -1);
  int first_diameter_node = farmost();
  std::fill(distance_.begin(), distance_.end(), 0);
  dfs(first_diameter_node, -1);
  int second_diameter_node = farmost();
  TreeCenterNodeFinder tree_center_node_finder(amount,
                                               tree_,
                                               first_diameter_node,
                                               second_diameter_node);
  vi result = tree_center_node_finder.FindCenterNodes();
  return result;
}


void TreeCenterFinder::dfs(int node, int parent) {
  if (parent != -1) {
    distance_[node] = distance_[parent] + 1;
  }
  for (int child_node : tree_[node]) {
    if (child_node == parent) {
      continue;
    }
    dfs(child_node, node);
  }
}


int TreeCenterFinder::farmost() {
  int max_distance = 0;
  int max_distance_node = 0;
  for (int index = 0; index < amount; ++index) {
    if (distance_[index] > max_distance) {
      max_distance = distance_[index];
      max_distance_node = index;
    }
  }
  return max_distance_node;
}


void find_centers() {
  for (int tree_index = 0; tree_index < 2; ++tree_index) {
    TreeCenterFinder tree_center_finder(amount, tree[tree_index]);
    center[tree_index] = tree_center_finder.FindCenter();
  }
}


bool isCornerCase () {
  return amount <= 2;
}


void processCornerCase() {
  std::cout << 1 << std::endl;
  if (amount == 2) {
    std::cout << 2 << std::endl;
  }
}


class TreeIsomorphismFinder {
public:
  TreeIsomorphismFinder(const vvi& first_tree, int first_tree_root,
                        const vvi& second_tree, int second_tree_root,
                        int amount);
  bool isIsomorphic();
  vi getIsomorphism();

private:
  void buildIsomorphism(int first_tree_node, int second_tree_node);
  void find_parent(int which, int node, int parent);
  void set_label_for_leafs(int which);
  bool is_leaf(int which, int node);
  int depth(int which, int node, int parent);
  void update_working_set(int which, int depth, int working_set_index);
  int getParent(int which, int node);
  int getLabel(int which, int node);
  void update_children_label(int which, int node, int parent, int label);
  bool is_used(int which, int parent);
  void update_next_working_set(int next, int parent, int which);
  void update_current_parent(int which, int parent);
  void make_used(int which, int parent);
  void sort_parents(int which);
  void set_labels(int working_set_next);
  bool is_child_lists_equal(int node, int which,
                            int node_prev, int child_prev);
  const vp& get_child_list(int node, int which);
  int compare(const vp& first, const vp& second);
  void sort(int left, int right);
  const vp& get_child_list(int index);
  void find_leafs(int node, int parent, int which, int depth);

    // input
  const vvi& first_tree_;
  int first_tree_root_;
  const vvi& second_tree_;
  int second_tree_root_;

  // output
  vi isomorphism_;

  // implementation
  vi first_parent_;
  vi second_parent_;
  vi first_label_;
  vi second_label_;
  vvp first_children_label_;
  vvp second_children_label_;

  vi first_used_;
  vi second_used_;
  int working_set_current_;
  int working_set_next_;
  std::vector<vp> working_set_;
  std::vector<int> working_set_size_;

  vi first_current_parents_;
  vi second_current_parents_;
  int first_current_parents_size_;
  int second_current_parents_size_;

  vvi first_leafs_;
  vvi second_leafs_;
};


TreeIsomorphismFinder::TreeIsomorphismFinder(const vvi& first_tree, int first_tree_root,
                                             const vvi& second_tree, int second_tree_root,
                                             int amount)
  : first_tree_(first_tree),
    first_tree_root_(first_tree_root),
    second_tree_(second_tree),
    second_tree_root_(second_tree_root),
    
    isomorphism_(amount, -1),
    first_parent_(amount, -1),
    second_parent_(amount, -1),
    first_label_(amount, -1),
    second_label_(amount, -1),
    first_children_label_(amount),
    second_children_label_(amount),
    first_used_(amount, 0),
    second_used_(amount, 0),
    working_set_current_(0),
    working_set_next_(0),
    working_set_(2, vp(amount + amount)),
    working_set_size_(2, 0),
    first_current_parents_(amount),
    second_current_parents_(amount),
    first_current_parents_size_(0),
    second_current_parents_size_(0),
    first_leafs_(amount),
    second_leafs_(amount) {}


void TreeIsomorphismFinder::find_parent(int which, int node, int parent) {
  if (which == 0) {
    first_parent_[node] = parent;
    for (int child : first_tree_[node]) {
      if (child != parent) {
        find_parent(which, child, node);
      }
    }
  } else {
    second_parent_[node] = parent;
    for (int child : second_tree_[node]) {
      if (child != parent) {
        find_parent(which, child, node);
      }
    }
  }
}


bool TreeIsomorphismFinder::is_leaf(int which, int node) {
  if (which == 0) {
    if (node != first_tree_root_ && first_tree_[node].size() == 1) {
      return true;
    }
  } else {
    if (node != second_tree_root_ && second_tree_[node].size() == 1) {
      return true;
    }
  }
  return false;
}


void TreeIsomorphismFinder::set_label_for_leafs(int which) {
  for (int index = 0; index < amount; ++index) {
    if (which == 0) {
      if (is_leaf(which, index) == true) {
        first_label_[index] = 0;
      }
    } else {
      if (is_leaf(which, index) == true) {
        second_label_[index] = 0;
      }
    }
  }
}


int TreeIsomorphismFinder::depth(int which, int node, int parent) {
  int max_depth = 0;
  if (which == 0) {
    for (int child : first_tree_[node]) {
      if (child != parent) {
        int node_depth = depth(which, child, node);
        max_depth = std::max(max_depth, node_depth);
      }
    }
  } else {
    for (int child : second_tree_[node]) {
      if (child != parent) {
        int node_depth = depth(which, child, node);
        max_depth = std::max(max_depth, node_depth);
      }
    }
  }
  return max_depth + 1;
}


void TreeIsomorphismFinder::update_working_set(int which, int depth, int working_set_index) {
  if (which == 0) {
    for (int leaf : first_leafs_[depth]) {
      int& position = working_set_size_[working_set_index];
      working_set_[working_set_index][position] = std::pair<int, int>(leaf, which);
      ++position;
    }
  } else {
    for (int leaf : second_leafs_[depth]) {
      int& position = working_set_size_[working_set_index];
      working_set_[working_set_index][position] = std::pair<int, int>(leaf, which);
      ++position;
    }
  }
}


int TreeIsomorphismFinder::getParent(int which, int node) {
  if (which == 0) {
    return first_parent_[node];
  } else {
    return second_parent_[node];
  }
}


int TreeIsomorphismFinder::getLabel(int which, int node) {
  if (which == 0) {
    return first_label_[node];
  } else {
    return second_label_[node];
  }
}


void TreeIsomorphismFinder::update_children_label(int which,
                                                  int node,
                                                  int parent,
                                                  int label) {
  if (which == 0) {
    first_children_label_[parent].push_back(std::pair<int, int>(label, node));
  } else {
    second_children_label_[parent].push_back(std::pair<int, int>(label, node));
  }
}


bool TreeIsomorphismFinder::is_used(int which, int parent) {
  if (which == 0) {
    return first_used_[parent];
  } else {
    return second_used_[parent];
  }
}


void TreeIsomorphismFinder::make_used(int which, int parent) {
  if (which == 0) {
    first_used_[parent] = true;
  } else {
    second_used_[parent] = true;
  }
}


void TreeIsomorphismFinder::update_next_working_set(int next, int parent, int which) {
  int& position = working_set_size_[next];
  working_set_[next][position] = std::pair<int, int>(parent, which);
  ++position;
}


void TreeIsomorphismFinder::update_current_parent(int which, int parent) {
  if (which == 0) {
    int& position = first_current_parents_size_;
    first_current_parents_[position] = parent;
    ++position;
  } else {
    int& position = second_current_parents_size_;
    second_current_parents_[position] = parent;
    ++position;
  }
}


void TreeIsomorphismFinder::sort_parents(int which) {
  if (which == 0) {
    for (int index = 0; index < first_current_parents_size_; ++index) {
      int parent = first_current_parents_[index];
      std::sort(first_children_label_[parent].begin(),
                first_children_label_[parent].end());
    }
  } else {
    for (int index = 0; index < second_current_parents_size_; ++index) {
      int parent = second_current_parents_[index];
      std::sort(second_children_label_[parent].begin(),
                second_children_label_[parent].end());
    }
  }
}


void TreeIsomorphismFinder::sort(int left, int right) {
  if (left >= right) {
    return;
  }
  int lt = left;
  int rt = right;
  std::default_random_engine dre;
  std::uniform_int_distribution<int> unid(0, rt - lt);
  int random_position = lt + unid(dre);
  int mid_node = working_set_[working_set_next_][random_position].first;
  int mid_which = working_set_[working_set_next_][random_position].second;
  const vp& mid = get_child_list(mid_node, mid_which);
  while (lt <= rt) {
    while (compare(get_child_list(lt), mid) == -1) {
      ++lt;
    }
    while (compare(mid, get_child_list(rt)) == -1) {
      --rt;
    }
    if (lt <= rt) {
      std::swap(working_set_[working_set_next_][lt],
                working_set_[working_set_next_][rt]);
      ++lt;
      --rt;
    }
  }
  sort(left, rt);
  sort(lt, right);
}


int TreeIsomorphismFinder::compare(const vp& first, const vp& second) {
  size_t size = std::min(first.size(), second.size());
  for (size_t index = 0; index < size; ++index) {
    if (first[index].first < second[index].first) {
      return -1;
    }
    if (first[index].first > second[index].first) {
      return 1;
    }
  }
  if (first.size() < second.size()) {
    return -1;
  }
  if (first.size() > second.size()) {
    return 1;
  }
  return 0;
}


const vp& TreeIsomorphismFinder::get_child_list(int index) {
  int node = working_set_[working_set_next_][index].first;
  int which = working_set_[working_set_next_][index].second;
  return get_child_list(node, which);
}


const vp& TreeIsomorphismFinder::get_child_list(int node, int which) {
  if (which == 0) {
    return first_children_label_[node];
  } else {
    return second_children_label_[node];
  }
}


bool TreeIsomorphismFinder::is_child_lists_equal(int node, int which,
                                                 int node_prev, int which_prev) {
  const vp& first = get_child_list(node, which);
  const vp& second = get_child_list(node_prev, which_prev);
  if (first.size() != second.size()) {
    return false;
  }
  for (size_t index = 0; index < first.size(); ++index) {
    if (first[index].first != second[index].first) {
      return false;
    }
  }
  return true;
}


void TreeIsomorphismFinder::set_labels(int working_set_next) {
  int node = working_set_[working_set_next][0].first;
  int which = working_set_[working_set_next][0].second;
  if (which == 0) {
    first_label_[node] = 0;
  } else {
    second_label_[node] = 0;
  }
  int counter = 0;
  for (int index = 1; index < working_set_size_[working_set_next]; ++index) {
    node = working_set_[working_set_next][index].first;
    which = working_set_[working_set_next][index].second;
    int node_prev = working_set_[working_set_next][index - 1].first;
    int which_prev = working_set_[working_set_next][index - 1].second;
    if (is_child_lists_equal(node, which, node_prev, which_prev) == false) {
      ++counter;
    }
    if (which == 0) {
      first_label_[node] = counter;
    } else {
      second_label_[node] = counter;
    }
  }
}


void TreeIsomorphismFinder::find_leafs(int node, int parent, int which, int depth) {
  ++depth;
  if (is_leaf(which, node) == true) {
    if (which == 0) {
      first_leafs_[depth].push_back(node);
    } else {
      second_leafs_[depth].push_back(node);
    }
    return;
  }
  if (which == 0) {
    for (int child : first_tree_[node]) {
      if (child == parent) {
        continue;
      }
      find_leafs(child, node, which, depth);
    }
  } else {
    for (int child : second_tree_[node]) {
      if (child == parent) {
        continue;
      }
      find_leafs(child, node, which, depth);
    }
  }
}


bool TreeIsomorphismFinder::isIsomorphic() {
  int first_depth = depth(0, first_tree_root_, -1);
  int second_depth = depth(1, second_tree_root_, -1);
  if (first_depth != second_depth) {
    return false;
  }

  find_leafs(first_tree_root_, -1, 0, 0);
  find_leafs(second_tree_root_, -1, 1, 0);
  find_parent(0, first_tree_root_, -1);
  find_parent(1, second_tree_root_, -1);
  set_label_for_leafs(0);
  set_label_for_leafs(1);
  update_working_set(0, first_depth, working_set_current_);
  update_working_set(1, second_depth, working_set_current_);

  for (int step = 0; step + 1 < first_depth; ++step) {
    int current_size = working_set_size_[working_set_current_];
    int working_set_next = working_set_current_ ^ 1;
    working_set_next_ = working_set_next;
    working_set_size_[working_set_next] = 0;
    first_current_parents_size_ = 0;
    second_current_parents_size_ = 0;

    for (int index = 0; index < current_size; ++index) {
      int which = working_set_[working_set_current_][index].second;
      int node = working_set_[working_set_current_][index].first;
      int label = getLabel(which, node);
      int parent = getParent(which, node);

      // update first_used, second_used
      // update first_current_parent, second_current_parent
      // update working_set_[working_set_next]
      // update first_children_label_, second_children_label_

      update_children_label(which, node, parent, label);
      if (is_used(which, parent) == false) {
        update_next_working_set(working_set_next, parent, which);
        update_current_parent(which, parent);
        make_used(which, parent);
      }
    }

    sort_parents(0);
    sort_parents(1);

    // now sort next_working_set
    // need a comparator
    // by i and j take node, which, node_other, which_other
    // use first_children_label_, second_children_label_
    update_working_set(0, first_depth - step - 1, working_set_next);
    update_working_set(1, second_depth - step - 1, working_set_next);
    sort(0, working_set_size_[working_set_next] - 1);

    set_labels(working_set_next);

    working_set_current_ = working_set_next;
  }

  return first_label_[first_tree_root_] == second_label_[second_tree_root_];
}


vi TreeIsomorphismFinder::getIsomorphism() {
  buildIsomorphism(first_tree_root_, second_tree_root_);
  return isomorphism_;
}


void TreeIsomorphismFinder::buildIsomorphism(int first_tree_node, int second_tree_node) {
  isomorphism_[first_tree_node] = second_tree_node;
  if (first_children_label_[first_tree_node].size() !=
      second_children_label_[second_tree_node].size()) {
    throw std::runtime_error("sizes are not equal :: buildIsomorphism");
  }
  for (size_t index = 0;
       index < first_children_label_[first_tree_node].size();
       ++index) {
    int first_child = first_children_label_[first_tree_node][index].second;
    int second_child = second_children_label_[second_tree_node][index].second;
    buildIsomorphism(first_child, second_child);
  }
}


void no_solution() {
  std::cout << -1;
}


void print_isomorphism(const vi& isomorphism) {
  for (int node_index : isomorphism) {
    std::cout << node_index + 1 << "\n";
  }
}


void find_and_print_isomorphism() {

  if (center[0].size() != center[1].size()) {
    no_solution();
    return;
  }

  bool is_isomorphic = false;
  vi isomorphism;

  // now sizes are equal
  for (size_t center_index = 0; center_index < center[0].size(); ++center_index) {
    TreeIsomorphismFinder tree_isomorphism_finder(tree[0], center[0][0],
                                                  tree[1], center[1][center_index],
                                                  amount);
    is_isomorphic = tree_isomorphism_finder.isIsomorphic();
    if (is_isomorphic == true) {
      isomorphism = tree_isomorphism_finder.getIsomorphism();
      break;
    }
  }

  if (is_isomorphic == true) {
    print_isomorphism(isomorphism);
  } else {
    no_solution();
  }
}


int main () {

  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  read_input();

  // trivial solution if less than 2 nodes
  if (isCornerCase() == true) {
    processCornerCase();
    return 0;
  }

  find_centers();

  find_and_print_isomorphism();

  return 0;
}
