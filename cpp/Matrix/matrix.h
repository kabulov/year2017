#pragma once

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <vector>

#include <cassert>
#include <iostream>

typedef enum { kNone, kSum, kSub, kMul } Kind;

template <typename T, typename Derived>
struct Base {
    const Derived& GetRef() const {
        return static_cast<const Derived&>(*this);
    }
};

template <typename T, typename T1, typename T2>
class Glue;

template <typename T, typename T1, typename T2>
struct Mid {
    const Glue<T, T1, T2>& src_;
    Mid(const Glue<T, T1, T2>& src) : src_(src) {
    }
    T operator()(int i, int j);
};

template <typename T>
class Matrix : public Base<T, Matrix<T>> {
public:
    Matrix(const Matrix& other) : data_(other.Rows()) {
        for (size_t i = 0; i < other.Rows(); ++i) {
            for (size_t j = 0; j < other.Columns(); ++j) {
                data_[i].emplace_back(other.data_[i][j]);
            }
            // data_[i] = other.data_[i];
        }
    }

    bool IsLeaf() const {
        return true;
    }

    void CollectMatrices(std::vector<const Matrix<T>*>& matrices) const {
        if (matrices.empty()) {
            matrices.clear();  // suppress warnings
        }
    }

    const Matrix<T>* GetMatrixPtr() const {
        return &static_cast<const Matrix<T>&>(*this);
    }

    const Matrix& operator=(const Matrix& other) {
        if (&other == this) {
            return *this;
        }
        for (size_t i = 0; i < data_.size(); ++i) {
            data_[i].clear();
        }
        data_.clear();
        data_.resize(other.data_.size());
        for (size_t i = 0; i < data_.size(); ++i) {
            data_[i] = other.data_[i];
        }
        // data_ = other.data_;
        return *this;
    }

    template <typename T1, typename T2>
    Matrix(const Glue<T, T1, T2>& src) {
        Copy<T1, T2>(src);
    }

    template <typename T1, typename T2>
    const Matrix& operator=(const Glue<T, T1, T2>& src) {
        // if (&src == this) return *this;
        for (size_t i = 0; i < data_.size(); ++i) {
            data_[i].clear();
        }
        data_.clear();
        Copy<T1, T2>(src);
        return *this;
    }

    explicit Matrix(size_t rows, size_t cols) : data_(rows, std::vector<T>(cols, T{})) {
    }

    explicit Matrix(size_t n) : data_(n, std::vector<T>(n, T{})) {
    }

    explicit Matrix(const std::vector<std::vector<T>>& matrix) {
        for (const auto& row : matrix) {
            data_.emplace_back(row);
        }
    }

    size_t Rows() const {
        return data_.size();
    }

    size_t Columns() const {
        return data_.empty() ? 0u : data_.front().size();
    }

    Matrix(const std::initializer_list<std::initializer_list<T>>& data) {
        size_t n = data.size();
        data_.reserve(n);
        for (const auto& row : data) {
            data_.emplace_back(row);
        }
    }

    T& operator()(int i, int j) {
        return data_[i][j];
    }

    const T& operator()(int i, int j) const {
        return data_[i][j];
    }

private:
    std::vector<std::vector<T>> data_;

    template <typename T1, typename T2>
    void Copy(const Glue<T, T1, T2>& src) {
        Mid<T, T1, T2> m(src);
        size_t rows = src.Rows();
        size_t cols = src.Columns();
        data_.resize(rows, std::vector<T>(cols));
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                data_[i][j] = m(i, j);
            }
        }
    }
};

template <typename T, typename T1, typename T2>
class Glue : public Base<T, Glue<T, T1, T2>> {
public:
    static const int kEmpty = -1;

    Glue(const T1& first, const T2& second, Kind kind = kNone) : left_(first), right_(second) {
        kind_ = kind;
        switch (kind_) {
            case kSum:
            case kSub:
                if (first.Rows() != second.Rows() || first.Columns() != second.Columns()) {
                    throw std::runtime_error(
                        "matrix sizes are not compliant for addition/subtraction");
                }
                rows_ = first.Rows();
                cols_ = first.Columns();
                break;
            case kMul:
                if (first.Columns() != second.Rows()) {
                    throw std::runtime_error("matrix sizes are not compliant for multiplication");
                }
                rows_ = first.Rows();
                cols_ = second.Columns();
                break;
            case kNone:
                throw std::runtime_error("bad operator in glue constructor");
        }
    }

    bool IsLeaf() const {
        return false;
    }

    const Matrix<T>* GetMatrixPtr() const {
        if (!matrix_) {
            Precalc();
        }
        return matrix_.get();
    }

    void CollectMatrices(std::vector<const Matrix<T>*>& matrices) const {
        if (left_.IsLeaf()) {
            matrices.emplace_back(left_.GetMatrixPtr());
            matrices.emplace_back(right_.GetMatrixPtr());
            return;
        }
        left_.CollectMatrices(matrices);
        matrices.emplace_back(right_.GetMatrixPtr());
    }

    void SexyMul() const {
        std::vector<const Matrix<T>*> matrices;
        CollectMatrices(matrices);

        size_t size = matrices.size();
        std::vector<std::vector<int>> dp(size, std::vector<int>(size, 0));
        std::vector<std::vector<int>> ans(size, std::vector<int>(size, kEmpty));

        assert(size >= 0);
        assert(ans[0][size - 1u] == kEmpty);
        RunDp(matrices, dp, ans, 0u, size - 1u);

        matrix_ = RestoreAnswer(matrices, ans, 0u, size - 1u);
    }

    using KMatrix = std::vector<std::vector<int>>;

    void RunDp(const std::vector<const Matrix<T>*>& matrices, KMatrix& dp, KMatrix& ans, size_t lt,
               size_t rt) const {
        if (lt + 1 >= rt) {
            if (lt + 1u == rt) {
                dp[lt][rt] =
                    (*matrices[lt]).Rows() * (*matrices[lt]).Columns() * (*matrices[rt]).Columns();
            }
            ans[lt][rt] = lt;
            return;
        }
        for (size_t md = lt; md < rt; ++md) {
            if (ans[lt][md] == kEmpty) {
                RunDp(matrices, dp, ans, lt, md);
                assert(ans[lt][md] != kEmpty);
            }
            if (ans[md + 1][rt] == kEmpty) {
                RunDp(matrices, dp, ans, md + 1, rt);
                assert(ans[md + 1][rt] != kEmpty);
            }
            int upd =
                dp[lt][md] + dp[md + 1][rt] +
                (*matrices[lt]).Rows() * (*matrices[md]).Columns() * (*matrices[rt]).Columns();
            if (ans[lt][rt] == kEmpty || dp[lt][rt] >= upd) {
                dp[lt][rt] = upd;
                ans[lt][rt] = md;
            }
        }
    }

    std::unique_ptr<Matrix<T>> RestoreAnswer(const std::vector<const Matrix<T>*>& matrices,
                                             const KMatrix& ans, size_t lt, size_t rt) const {
        if (lt == rt) {
            return std::unique_ptr<Matrix<T>>(new Matrix<T>(*matrices[lt]));
        }
        std::unique_ptr<Matrix<T>> result = std::unique_ptr<Matrix<T>>(
            new Matrix<T>((*matrices[lt]).Rows(), (*matrices[rt]).Columns()));
        auto left_matrix = RestoreAnswer(matrices, ans, lt, ans[lt][rt]);
        auto right_matrix = RestoreAnswer(matrices, ans, ans[lt][rt] + 1, rt);
        size_t rows = left_matrix->Rows();
        size_t mid = left_matrix->Columns();
        size_t cols = right_matrix->Columns();
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                T sum{};
                for (size_t k = 0; k < mid; ++k) {
                    sum += (*left_matrix)(i, k) * (*right_matrix)(k, j);
                }
                (*result)(i, j) = sum;
            }
        }
        return result;
    }

    size_t Rows() const {
        return rows_;
    }

    size_t Columns() const {
        return cols_;
    }

    T operator()(int i, int j) {
        return GetAt(i, j);
    }

    T operator()(int i, int j) const {
        return GetAt(i, j);
    }

private:
    const T1& left_;
    const T2& right_;
    size_t rows_ = 0;
    size_t cols_ = 0;
    Kind kind_ = kNone;
    mutable std::unique_ptr<Matrix<T>> matrix_ = {nullptr};

    T GetAt(int i, int j) const {
        Precalc();
        return (*matrix_)(i, j);
    }

    void Precalc() const {
        switch (kind_) {
            case kSum:
                if (!matrix_) {
                    matrix_ =
                        std::unique_ptr<Matrix<T>>(new Matrix<T>(left_.Rows(), left_.Columns()));
                    for (size_t r = 0; r < left_.Rows(); ++r) {
                        for (size_t c = 0; c < left_.Columns(); ++c) {
                            (*matrix_)(r, c) = left_(r, c) + right_(r, c);
                        }
                    }
                }
                return;
            case kSub:
                if (!matrix_) {
                    matrix_ =
                        std::unique_ptr<Matrix<T>>(new Matrix<T>(left_.Rows(), left_.Columns()));
                    for (size_t r = 0; r < left_.Rows(); ++r) {
                        for (size_t c = 0; c < left_.Columns(); ++c) {
                            (*matrix_)(r, c) = left_(r, c) - right_(r, c);
                        }
                    }
                }
                return;
            case kMul:
                if (!matrix_) {
                    SexyMul();
                }
                return;
            case kNone:
                throw std::runtime_error("bad operator in Precalc");
            default:
                throw std::runtime_error("unexpected branch in Precalc");
        }
    }
};

template <typename T, typename T1, typename T2>
const int Glue<T, T1, T2>::kEmpty;

// so only now we define the body of this method, after glue is defined
template <typename T, typename T1, typename T2>
T Mid<T, T1, T2>::operator()(int i, int j) {
    return src_(i, j);
}

template <typename T, typename T1, typename T2>
const Glue<T, T1, T2> operator+(const Base<T, T1>& left, const Base<T, T2>& right) {
    return Glue<T, T1, T2>(left.GetRef(), right.GetRef(), kSum);
}

template <typename T, typename T1, typename T2>
const Glue<T, T1, T2> operator-(const Base<T, T1>& l, const Base<T, T2>& r) {
    return Glue<T, T1, T2>(l.GetRef(), r.GetRef(), kSub);
}

template <typename T, typename T1, typename T2>
const Glue<T, T1, T2> operator*(const Base<T, T1>& l, const Base<T, T2>& r) {
    return Glue<T, T1, T2>(l.GetRef(), r.GetRef(), kMul);
}

template <typename T, typename T1, typename T2>
bool operator==(const Base<T, T1>& l, const Base<T, T2>& r) {
    size_t rows = l.GetRef().Rows();
    size_t cols = l.GetRef().Columns();
    if (rows != r.GetRef().Rows() || cols != r.GetRef().Columns()) {
        throw std::runtime_error("matrix sizes are not compliant for matrix comparison");
    }
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            if (l.GetRef()(i, j) != r.GetRef()(i, j)) {
                return false;
            }
        }
    }
    return true;
}
