#include <iostream>
#include <vector>
#include <algorithm>

using Vec = std::vector<int64_t>;
using VecVec = std::vector<Vec>;

int size;
std::vector<VecVec> data;

void init(int size) {
    data.resize(size, VecVec(size, Vec(size, 0)));
}

void inc(int xreal, int yimag, int znan, int64_t delta) {
    for (int ix = xreal; ix < size; ix = (ix | (ix + 1))) {
        for (int jy = yimag; jy < size; jy = (jy | (jy + 1))) {
            for (int kz = znan; kz < size; kz = (kz | (kz + 1))) {
                data[ix][jy][kz] += delta;
            }
        }
    }
}

int64_t sum(int xreal, int yimag, int znan) {
    int64_t result = 0;
    for (int ix = xreal; ix >= 0; ix = (ix & (ix + 1)) - 1) {
        for (int jy = yimag; jy >= 0; jy = (jy & (jy + 1)) - 1) {
            for (int kz = znan; kz >= 0; kz = (kz & (kz + 1)) - 1) {
                result += data[ix][jy][kz];
            }
        }
    }
    return result;
}

int64_t offer(int xreal, int yimag, int znan) {
    if (xreal < 0 || yimag < 0 || znan < 0) {
        return 0;
    }
    return sum(xreal, yimag, znan);
}

int64_t getsum(int x_1x, int y_1y, int z_1z, int x_2xx, int y_2yy, int z_2zz) {
    int64_t result = sum(x_2xx, y_2yy, z_2zz);
    result -= offer(x_1x - 1, y_2yy, z_2zz);
    result -= offer(x_2xx, y_1y - 1, z_2zz);
    result -= offer(x_2xx, y_2yy, z_1z - 1);
    result += offer(x_1x - 1, y_1y - 1, z_2zz);
    result += offer(x_1x - 1, y_2yy, z_1z - 1);
    result += offer(x_2xx, y_1y - 1, z_1z - 1);
    result -= offer(x_1x - 1, y_1y - 1, z_1z - 1);
    return result;
}

int main () {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::cin >> size;
    init(size);

    int command;
    std::vector<int> ix(2);
    std::vector<int> jy(2);
    std::vector<int> kz(2);
    int64_t delta;
    int64_t answer;
    while (std::cin >> command) {
        if (command == 3) {
            break;
        }
        std::cin >> ix[0] >> jy[0] >> kz[0];
        if (command == 1) {
            std::cin >> delta;
            inc(ix[0], jy[0], kz[0], delta);
        } else {
            std::cin >> ix[1] >> jy[1] >> kz[1];
            answer = getsum(ix[0], jy[0], kz[0], ix[1], jy[1], kz[1]);
            std::cout << answer << "\n";
        }
    }

    return 0;
}
