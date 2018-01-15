#include <random>
#include <cstdint>
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>

struct Player {
    Player();
    Player(int id, int64_t effectiveness);
    int id;
    int64_t effectiveness;
};

struct CompareByEffectiveness {
    bool operator() (const Player& left, const Player& right);
};

struct CompareByIndex {
    bool operator() (const Player& left, const Player& right);
};

template<typename InputIterator,
        typename OutputIterator,
        typename Comparator>
void Merge(InputIterator first_begin,
           InputIterator first_end,
           InputIterator second_begin,
           InputIterator second_end,
           OutputIterator output_begin,
           Comparator comparator);

template<typename Iterator,
        typename Comparator = std::less<typename std::iterator_traits<Iterator>::value_type>>
void MSort(Iterator begin,
           Iterator end,
           Comparator comparator = Comparator());

std::vector<Player> ReadInput(std::istream& cin = std::cin);

std::vector<Player> BuildMostEffectiveSolidTeam(std::vector<Player> players);

int64_t PlayersEffectivenessSum(const std::vector<Player>& players);

void PrintOutput(std::vector<Player> best_team, std::ostream& cout = std::cout);

int main() {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);

    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::vector<Player> players = ReadInput();

    std::vector<Player> best_team = BuildMostEffectiveSolidTeam(players);

    PrintOutput(best_team);

    return 0;
}

Player::Player() {}

Player::Player(int id, int64_t effectiveness):
        id(id),
        effectiveness(effectiveness)
{}

bool CompareByEffectiveness::operator()(const Player& left, const Player& right) {
    return left.effectiveness < right.effectiveness;
}

bool CompareByIndex::operator() (const Player& left, const Player& right) {
    return left.id < right.id;
}

template<typename InputIterator,
        typename OutputIterator,
        typename Comparator>
void Merge(InputIterator first_begin,
           InputIterator first_end,
           InputIterator second_begin,
           InputIterator second_end,
           OutputIterator output_begin,
           Comparator comparator)
{
    while (first_begin != first_end && second_begin != second_end) {
        if (comparator(*first_begin, *second_begin)) {
            *output_begin = *first_begin;
            ++first_begin;
        } else {
            *output_begin = *second_begin;
            ++second_begin;
        }
        ++output_begin;
    }
    std::copy(first_begin, first_end, output_begin);
    std::copy(second_begin, second_end, output_begin);
}

template<typename Iterator,
        typename Comparator>
void MSort(Iterator begin,
           Iterator end,
           Comparator comparator)
{
    if (std::distance(begin, end) < 2) {
        return;
    }

    int range_size = static_cast<int>(std::distance(begin, end));
    Iterator middle = begin + (range_size / 2);
    MSort(begin, middle, comparator);
    MSort(middle, end, comparator);

    using container_type = typename std::iterator_traits<Iterator>::value_type;
    std::vector<container_type> buffer(begin, middle);
    Merge(buffer.begin(), buffer.end(), middle, end, begin, comparator);
}

std::vector<Player> ReadInput(std::istream& cin) {
    int input_size;
    cin >> input_size;
    std::vector<Player> players;
    players.reserve(input_size);
    int64_t player_effectiveness;
    for (int player_id = 1; player_id <= input_size; ++player_id) {
        std::cin >> player_effectiveness;
        players.emplace_back(player_id, player_effectiveness);
    }
    return players;
}

std::vector<Player> BuildMostEffectiveSolidTeam(std::vector<Player> players) {
    if (players.size() <= 2) {
        return players;
    }

    MSort(players.begin(), players.end(), CompareByEffectiveness());

    struct TeamInfo {
        std::vector<Player>::iterator left;
        std::vector<Player>::iterator right;
        int64_t sum;
    };

    TeamInfo current_team;
    current_team.left = players.begin();
    current_team.right = players.begin() + 1;
    current_team.sum = players[0].effectiveness;

    TeamInfo best_team = current_team;

    for (; current_team.left != players.end(); ++current_team.left) {
        if (current_team.left != players.begin()) {
            current_team.sum -= (current_team.left - 1)->effectiveness;
        }
        while (current_team.right != players.end() &&
               std::distance(current_team.left, current_team.right) < 2)
        {
            current_team.sum += current_team.right->effectiveness;
            ++current_team.right;
        }
        while (current_team.right != players.end() &&
                (current_team.left->effectiveness + (current_team.left + 1)->effectiveness
                    >= current_team.right->effectiveness))
        {
            current_team.sum += current_team.right->effectiveness;
            ++current_team.right;
        }
        if (current_team.sum > best_team.sum) {
            best_team = current_team;
        }
    }

    return {best_team.left, best_team.right};
};

int64_t PlayersEffectivenessSum(const std::vector<Player>& players) {
    int64_t sum = 0;
    for (const Player& one_player: players) {
        sum += one_player.effectiveness;
    }
    return sum;
}

void PrintOutput(std::vector<Player> best_team, std::ostream& cout) {
    MSort(best_team.begin(), best_team.end(), CompareByIndex());
    int64_t effectiveness_sum = PlayersEffectivenessSum(best_team);
    cout << effectiveness_sum << "\n";
    for (const Player &one_player: best_team) {
        cout << one_player.id << " ";
    }
}
