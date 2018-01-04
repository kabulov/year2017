
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <experimental/optional>

class Hash {
public:
    Hash() = default;
    size_t GetIndex(int number);
    void SetInterval(int64_t interval);
    void GenerateHash(std::mt19937& generator);

private:
    static const int64_t prime_ = 2038074743;
    static const int64_t max_positive_value_ = 1000LL * 1000 * 1000;

    int64_t interval_;
    int64_t multiplier_;
    int64_t adder_;

    int64_t GetHash(int64_t number);
    int64_t Normalize(int64_t number);
};

class HashTableWithoutCollisions {
public:
    HashTableWithoutCollisions() = default;
    void Initialize(const std::vector<int>& numbers, std::mt19937& generator);
    bool Contains(int number);

private:
    Hash hash_;
    std::vector<std::experimental::optional<int>> buckets_;

    bool HasCollision(const std::vector<int>& numbers);
};

class FixedSet {
public:
    FixedSet() = default;
    void Initialize(const std::vector<int>& numbers);
    bool Contains(int number);

private:
    Hash hash_;
    std::vector<HashTableWithoutCollisions> buckets_;

    void BuildFirstLevelHashFunction(const std::vector<int>& numbers, std::mt19937& generator);
};

int64_t Square(int64_t value);

void ReadVector(std::vector<int>* data, std::istream& cin = std::cin);

void ReadInput(std::vector<int>* numbers,
               std::vector<int>* queries,
               std::istream& cin = std::cin);

const std::vector<char> ProcessQueries(const std::vector<int>& numbers,
                                       const std::vector<int>& queries);

void PrintOutput(const std::vector<char>& query_result,
                 std::ostream& cout = std::cout);

int main () {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);

    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::vector<int> numbers;
    std::vector<int> queries;

    ReadInput(&numbers, &queries);

    const std::vector<char> query_result = ProcessQueries(numbers, queries);

    PrintOutput(query_result);

    return 0;
}

void ReadVector(std::vector<int>* data, std::istream& cin) {
    int size;
    cin >> size;
    (*data).resize(size);
    for (int& element: (*data)) {
        cin >> element;
    }
}

void ReadInput(std::vector<int>* numbers,
               std::vector<int>* queries,
               std::istream& cin) {
    ReadVector(numbers);
    ReadVector(queries);
}

const std::vector<char> ProcessQueries(const std::vector<int>& numbers,
                                       const std::vector<int>& queries)
{
    std::vector<char> query_result;
    query_result.reserve(queries.size());

    FixedSet fixed_set;
    fixed_set.Initialize(numbers);

    for (int query: queries) {
        query_result.push_back(fixed_set.Contains(query));
    }

    return query_result;
}

void PrintOutput(const std::vector<char>& query_result, std::ostream& cout) {
    for (size_t position = 0; position < query_result.size(); ++position) {
        bool set_contains_key = static_cast<bool>(query_result[position]);
        std::cout << (set_contains_key ? "Yes" : "No") << "\n";
    }
}

bool FixedSet::Contains(int number) {
    size_t position = hash_.GetIndex(number);
    return buckets_[position].Contains(number);
}

void FixedSet::Initialize(const std::vector<int>& numbers) {
    hash_.SetInterval(numbers.size());
    buckets_.resize(numbers.size());

    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 generator = std::mt19937(seed);

    BuildFirstLevelHashFunction(numbers, generator);

    std::vector<std::vector<int>> buckets_contents(buckets_.size());
    for (int number: numbers) {
        buckets_contents[hash_.GetIndex(number)].push_back(number);
    }

    for (size_t bucket = 0; bucket < buckets_contents.size(); ++bucket) {
        if (!buckets_contents[bucket].empty()) {
            buckets_[bucket].Initialize(buckets_contents[bucket], generator);
        }
    }
}

void FixedSet::BuildFirstLevelHashFunction(const std::vector<int>& numbers,
                                           std::mt19937& generator)
{
    const int64_t keys_amount_magnification_factor = 4;
    const int64_t max_keys_amount = keys_amount_magnification_factor * buckets_.size();
    std::vector<int64_t> buckets_sizes(buckets_.size());
    int64_t keys_amount;
    do {
        hash_.GenerateHash(generator);
        std::fill(buckets_sizes.begin(), buckets_sizes.end(), 0);
        for (int number: numbers) {
            ++buckets_sizes[hash_.GetIndex(number)];
        }
        keys_amount = 0;
        for (int64_t bucket_size: buckets_sizes) {
            keys_amount += Square(bucket_size);
        }
    } while (keys_amount > max_keys_amount);
}

bool HashTableWithoutCollisions::Contains(int number) {
    if (!buckets_.empty()) {
        size_t position = hash_.GetIndex(number);
        return buckets_[position] && buckets_[position].value() == number;
    }
    return false;
}

void HashTableWithoutCollisions::Initialize(const std::vector<int> &numbers,
                                            std::mt19937& generator) {
    buckets_.resize(Square(numbers.size()), std::experimental::nullopt);
    hash_.SetInterval(buckets_.size());

    do {
        hash_.GenerateHash(generator);
    } while (HasCollision(numbers));

    for (int number: numbers) {
        buckets_[hash_.GetIndex(number)] = number;
    }
}

bool HashTableWithoutCollisions::HasCollision(const std::vector<int>& numbers) {
    std::vector<char> buckets(Square(numbers.size()), false);
    for (int number: numbers) {
        size_t position = hash_.GetIndex(number);
        if (buckets[position]) {
            return true;
        }
        buckets[position] = true;
    }
    return false;
}

void Hash::SetInterval(int64_t interval) {
    interval_ = interval;
}

void Hash::GenerateHash(std::mt19937& generator) {
    multiplier_ = std::uniform_int_distribution<int>(1, prime_ - 1)(generator);
    adder_ = std::uniform_int_distribution<int>(0, prime_ - 1)(generator);
}

size_t Hash::GetIndex(int number) {
    return static_cast<size_t>(GetHash(number) % interval_);
}

int64_t Hash::GetHash(int64_t number) {
    number = Normalize(number);
    return (multiplier_ * number + adder_) % prime_;
}

int64_t Hash::Normalize(int64_t number) {
    if (number >= 0) {
        return number;
    }
    return max_positive_value_ - number;
}

int64_t Square(int64_t value) {
    return value * value;
}

