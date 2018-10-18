#include <algorithm>
#include <utility>
#include <iostream>
#include <vector>

int main () {

    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);

    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int amount;
    std::cin >> amount;
    int64_t lower;
    std::cin >> lower;
    int64_t upper;
    std::cin >> upper;
    std::vector<std::pair<int64_t, int64_t>> data(amount);
    for (std::pair<int64_t, int64_t>& elem: data) {
        std::cin >> elem.first >> elem.second;
    }

    int answer_mask = 0;
    int64_t best_cost = 0;

    const int half = 16;
    std::vector<std::pair<int64_t, std::pair<int64_t, int>>> half_data;
    half_data.reserve(1 << (std::min(half, amount))); // !!!
    for (int mask = 0; mask < (1 << std::min(half, amount)); ++mask) {
        int64_t weight = 0;
        int64_t cost = 0;
        for (int pos = 0; pos < std::min(half, amount); ++pos) {
            if ((mask >> pos) & 1) {
                weight += data[pos].first;
                cost += data[pos].second;
            }
        }
        half_data.push_back(std::make_pair(weight, std::make_pair(cost, mask)));
        if (lower <= weight && weight <= upper) {
            if (cost > best_cost) {
                best_cost = cost;
                answer_mask = mask;
            }
        }
    }
    half_data[0].first = -5 * static_cast<int64_t>(1e16);

    if (amount <= half) {
        if (answer_mask == 0) {
            std::cout << 0;
        } else {
            int bitcount = 0;
            int maskcopy = answer_mask;
            while (maskcopy) {
                ++bitcount;
                maskcopy &= maskcopy - 1;
            }
            std::cout << bitcount << std::endl;
            for (int pos = 0; pos < std::min(half, amount); ++pos) {
                if ((answer_mask >> pos) & 1) {
                    std::cout << pos + 1 << " "; // !!! sorted ?
                }
            }
        }
    } else {
        std::sort(half_data.begin(), half_data.end());
        std::vector<std::pair<int64_t, std::pair<int64_t, int>>> half_data_cost(half_data.size());
        for (size_t iter = 0; iter < half_data.size(); ++iter) {
            half_data_cost[iter].first = half_data[iter].second.first;
            half_data_cost[iter].second.first = half_data[iter].first;
            half_data_cost[iter].second.second = half_data[iter].second.second;
        }

        int bucket_size = 1 << (half / 2);
        for (int pos = 0; pos < (1 << half); pos += bucket_size) {
            std::sort(half_data_cost.begin() + pos, half_data_cost.begin() + pos + bucket_size);
        }

        int second_answer_mask = 0;
        int64_t second_best_cost = 0;

        std::pair<int, int> third_answer_mask = std::make_pair(0, 0);
        int64_t third_best_cost = 0;

        for (int mask = 1; mask < (1 << (amount - half)); ++mask) {
            int64_t weight = 0;
            int64_t cost = 0;
            for (int pos = 0; pos < amount - half; ++pos) {
                if ((mask >> pos) & 1) {
                    weight += data[half + pos].first;
                    cost += data[half + pos].second;
                }
            }
            if (lower <= weight && weight <= upper) {
                if (cost > second_best_cost) {
                    second_answer_mask = mask;
                    second_best_cost = cost;
                }
            }

            // run all masks in second
            if (weight < upper) {
                for (int pos = 0; pos < (1 << half); pos += bucket_size) {
                    int64_t new_lower = lower - weight;
                    int64_t new_upper = upper - weight;
                    int start = pos;
                    int end = start + bucket_size - 1;
                    if (half_data[end].first < new_lower) {
                        continue;
                    }
                    if (half_data[start].first > new_upper) {
                        break;
                    }
                    if (new_lower <= half_data[start].first && half_data[end].first <= new_upper) {
                        int64_t another_cost = half_data_cost[end].first;
                        if (cost + another_cost > third_best_cost) {
                            third_answer_mask.first = half_data_cost[end].second.second;
                            third_answer_mask.second = mask;
                            third_best_cost = cost + another_cost;
                        }
                    } else {
                        for (int position = start; position <= end; ++position) {
                            if (new_lower <= half_data[position].first &&
                                    half_data[position].first <= new_upper) {
                                if (cost + half_data[position].second.first > third_best_cost) {
                                    third_answer_mask.first = half_data[position].second.second;
                                    third_answer_mask.second = mask;
                                    third_best_cost = cost + half_data[position].second.first;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (best_cost >= second_best_cost && best_cost >= third_best_cost) {
            int bitcount = 0;
            int maskcopy = answer_mask;
            while (maskcopy) {
                ++bitcount;
                maskcopy &= maskcopy - 1;
            }
            std::cout << bitcount << std::endl;
            for (int pos = 0; pos < half; ++pos) {
                if ((answer_mask >> pos) & 1) {
                    std::cout << pos + 1 << " ";
                }
            }
        } else {
            if (second_best_cost >= best_cost && second_best_cost >= third_best_cost) {
                int bitcount = 0;
                int maskcopy = second_answer_mask;
                while (maskcopy) {
                    ++bitcount;
                    maskcopy &= maskcopy - 1;
                }
                std::cout << bitcount << std::endl;
                for (int pos = 0; pos < half; ++pos) {
                    if ((second_answer_mask >> pos) & 1) {
                        std::cout << pos + 1 + 16 << " ";
                    }
                }
            } else {
                if (third_best_cost >= best_cost && third_best_cost >= second_best_cost) {
                    int bitcount = 0;
                    int maskcopy = third_answer_mask.first;
                    while (maskcopy) {
                        ++bitcount;
                        maskcopy &= maskcopy - 1;
                    }
                    maskcopy = third_answer_mask.second;
                    while (maskcopy) {
                        ++bitcount;
                        maskcopy &= maskcopy - 1;
                    }
                    std::cout << bitcount << std::endl;
                    for (int pos = 0; pos < half; ++pos) {
                        if ((third_answer_mask.first >> pos) & 1) {
                            std::cout << pos + 1 << " ";
                        }
                    }
                    for (int pos = 0; pos < half; ++pos) {
                        if ((third_answer_mask.second >> pos) & 1) {
                            std::cout << pos + 1 + 16 << " ";
                        }
                    }
                } else {
                    std::cout << "0";
                }
            }
        }
    }

    return 0;
}
