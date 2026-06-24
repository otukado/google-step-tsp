#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>
#include <array>
#include <algorithm>
#include <format>
#include <iostream>
#include <cstdint>
#include <chrono>
#include <cmath>

constexpr double INF = std::numeric_limits<double>::infinity();

// それぞれの街の点を表す struct
struct point_2D {
    double x = 0;
    double y = 0;
};

struct city {
    point_2D position;
    int id;
};

uint32_t xor_shift() {
    static uint32_t y = 2463534242;
    y = y ^ (y << 13);
    y = y ^ (y >> 17);
    return y = y ^ (y << 5);
}
inline double rnd() {
    return (double)xor_shift() / 4294967295.0;
}

class Timer {
    std::chrono::high_resolution_clock::time_point start_time;
public:
    Timer() { start_time = std::chrono::high_resolution_clock::now(); }
    int elapsed_ms() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::duration<int, std::milli>>(now - start_time).count();
    }
};

// input file の名前を受け取り、街の配列を返す関数
std::vector<city> read_cities(const std::string input_file_name) {
    std::vector<city> cities;
    std::ifstream input(input_file_name);
    std::string line;

    int idx = 0; // 街の番号
    while(getline(input, line)) { // input 内の各行について
        if(line == "x,y") continue;

        std::stringstream ss(line);
        std::string raw_position; // line からカンマで区切った生の座標
        point_2D input_position; //この行で input される街の座標が入る
        for(int i=0; i<2; ++i) {
            getline(ss, raw_position, ',');
            double position = std::stod(raw_position); // カンマで区切った文字列を string から double にする
            if(i == 0) input_position.x = position; // カンマで区切った一つ目の position は x 座標
            else input_position.y = position; // 二つ目なら y 座標
        }
        cities.push_back({input_position, idx});
        ++idx;
    }

    return cities;
}

// 2乗を返す
double square(double x) {
    return x * x;
}

double calc_distance_squared(const point_2D& a, const point_2D& b) {
    return square(a.x - b.x) + square(a.y - b.y);
}

std::vector<std::vector<int>> nearly_city(const std::vector<city>& cities){ // 近い 15 の街の保存
    int n = cities.size();

    std::vector<std::vector<int>> response(n);
    int k = 15;

    for(int i=0; i<n; ++i) {
        std::vector<std::pair<double, int>> distances; //距離を保存する
        distances.reserve(n);
        for(int j=0; j<n; ++j) {
            if(i == j) continue;
            distances.push_back({calc_distance_squared(cities[i].position, cities[j].position), j});
        }
        std::partial_sort(distances.begin(), distances.begin() + k, distances.end()); // 前 15 個だけ sort

        response[i].reserve(k);
        for(int j=0; j<k; ++j) {
            response[i].push_back(distances[j].second);
        }
    }

    return response;
};

std::vector<int> greedy_order(const std::vector<city>& cities) {
    int number_of_cities = static_cast<int>(cities.size()); // 街の数
    auto visited = std::vector(number_of_cities, false); // 各街をすでに訪れたかどうか
    std::vector<int> answer;

    int start_city = xor_shift() % number_of_cities;
    point_2D start = cities[start_city].position;
    visited[start_city] = true;
    answer.push_back(start_city);

    point_2D current_position = start; // 現在の座標。最初なので、スタートする街

    while(true) {
        double min_distance = INF; //今までの候補の中で最も近い場所との距離
        int nearest_city;

        for(int i=1; i<number_of_cities; ++i) {
            if(visited[i]) continue;
            double distance = calc_distance_squared(current_position, cities[i].position); //現在いる都市との距離の二乗

            if(min_distance <= distance) continue; //最小距離より、distance が小さければ
            nearest_city = i; // 最も近い都市として記録
            min_distance = distance;
        }

        answer.push_back(nearest_city); // 次に移動するから push_back
        visited[nearest_city] = true; // 次に移動するから visited を true にする
        current_position = cities[nearest_city].position; // nearest_city が次の移動先
        if((int) answer.size() == number_of_cities) return answer;
    }
}

std::vector<int> two_opt(const std::vector<city>& cities, std::vector<int> order) {
    int cities_num = cities.size();
    bool improved = true;

    while(improved) {
        improved = false;
        for(int i=0; i<cities_num; ++i) { // 二重ループで辺の組を数える
            for(int j=i+2; j<cities_num; ++j) { // 辺の本数は、 {order のサイズ} - 1, だったんだけど、
                if(i == (j+1) % cities_num) continue;

                const point_2D& point_1 = cities[order[i]].position;
                const point_2D& point_2 = cities[order[(i+1) % cities_num]].position;
                const point_2D& point_3 = cities[order[j]].position;
                const point_2D& point_4 = cities[order[(j+1) % cities_num]].position;
                
                double current_distance = calc_distance_squared(point_1, point_2) + calc_distance_squared(point_3, point_4);
                double new_distance = calc_distance_squared(point_1, point_3) + calc_distance_squared(point_2, point_4);

                if (current_distance - new_distance > 1e-8) {
                    std::reverse(std::next(order.begin(), i+1), std::next(order.begin(), j+1));
                    improved = true;
                }
            }
        }
    }
    return order;
};


inline double calc_total_distance(const std::vector<city>& cities, const std::vector<int>& order) {
    double total = 0.0;
    int n = order.size();
    for (int i = 0; i < n; ++i) {
        total += std::sqrt(calc_distance_squared(cities[order[i]].position, cities[order[(i + 1) % n]].position));
    }
    return total;
}

class TspState {
public:
    std::vector<int> order; // 現在の巡回ルート
    std::vector<int> pos; // 各都市が現在 order のどこにあるか
    double current_dist; // 現在の距離
    int n; // 都市数

    TspState(const std::vector<int>& initial_order, const std::vector<city>& cities) {
        order = initial_order;
        n = order.size();
        pos.resize(n);
        for (int i = 0; i < n; ++i) pos[order[i]] = i;
        
        // 初期距離の計算
        current_dist = 0.0;
        for (int i=0; i<n; ++i) {
            current_dist += std::sqrt(calc_distance_squared(cities[order[i]].position, cities[order[(i + 1) % n]].position));
        }
    }
};

class CoolingScheduler {
    double start_temp;
    double end_temp;
    int time_limit_ms;
public:
    CoolingScheduler(double start, double end, int limit_ms) 
        : start_temp(start), end_temp(end), time_limit_ms(limit_ms) {}

    // いい感じの温度管理(コピペ)
    inline double get_temperature(int elapsed_ms) const {
        double progress = static_cast<double>(elapsed_ms) / time_limit_ms;
        progress = std::min(1.0, progress);
        return start_temp * std::pow(end_temp / start_temp, progress);
    }
};

namespace Neighborhood {
    
    // 2点間の距離を取得する関数
    double get_dist(const std::vector<float>& dist_table, int u, int v, int n) {
        return dist_table[u * n + v];
    }

    // 2-opt
    struct TwoOpt {
        int i, j;
        double delta;

        bool prepare(const TspState& state, const std::vector<float>& dist_table, int i_idx, int j_idx, int n) {
            i = i_idx; j = j_idx;
            if (i == j) return false;
            if (i > j) std::swap(i, j);
            if (i == (j + 1) % state.n || (i + 1) % state.n == j) return false;

            int u = state.order[i];
            int u_next = state.order[(i + 1) % state.n];
            int v = state.order[j];
            int v_next = state.order[(j + 1) % state.n];

            delta = get_dist(dist_table, u, v, n) + get_dist(dist_table, u_next, v_next, n) - get_dist(dist_table, u, u_next, n) - get_dist(dist_table, v, v_next, n);
            return true;
        }

        void apply(TspState& state) const {
            std::reverse(state.order.begin() + i + 1, state.order.begin() + j + 1);
            for (int k=i+1; k<=j; ++k) pos_update(state, k);
            state.current_dist += delta;
        }
        void pos_update(TspState& state, int k) const {
            state.pos[state.order[k]] = k;
        }
    };

    // i と j を入れ替え
    struct Swap {
        int i, j;
        double delta;

        bool prepare(const TspState& state, const std::vector<float>& dist_table, int i_idx, int j_idx) {
            i = i_idx; j = j_idx;
            if (i == j) return false;

            int n = state.n;
            int u = state.order[i];
            int u_prev = state.order[(i - 1 + n) % n];
            int u_next = state.order[(i + 1) % n];
            int v = state.order[j];
            int v_prev = state.order[(j - 1 + n) % n];
            int v_next = state.order[(j + 1) % n];
            if (j == (i + 1) % n) {
                delta = get_dist(dist_table, u_prev, v, n) + get_dist(dist_table, u, v_next, n) 
                      - get_dist(dist_table, u_prev, u, n) - get_dist(dist_table, v, v_next, n);
            } else if (i == (j + 1) % n) {
                delta = get_dist(dist_table, v_prev, u, n) + get_dist(dist_table, v, u_next, n) 
                      - get_dist(dist_table, v_prev, v, n) - get_dist(dist_table, u, u_next, n);
            } else {
                delta = get_dist(dist_table, u_prev, v, n) + get_dist(dist_table, v, u_next, n) + get_dist(dist_table, v_prev, u, n) + get_dist(dist_table, u, v_next, n) 
                      - (get_dist(dist_table, u_prev, u, n) + get_dist(dist_table, u, u_next, n) + get_dist(dist_table, v_prev, v, n) + get_dist(dist_table, v, v_next, n));
            }
            return true;
        }

        void apply(TspState& state) const {
            std::swap(state.order[i], state.order[j]);
            state.pos[state.order[i]] = i;
            state.pos[state.order[j]] = j;
            state.current_dist += delta;
        }
    };

    // 街i を j の直後に insert
    struct Insert {
        int i, j;
        double delta;

        bool prepare(const TspState& state, const std::vector<float>& dist_table, int i_idx, int j_idx) {
            i = i_idx; j = j_idx;
            if (i == j || j == (i - 1 + state.n) % state.n) return false;

            int n = state.n;
            int u = state.order[i];
            int u_prev = state.order[(i - 1 + n) % n];
            int u_next = state.order[(i + 1) % n];
            int v = state.order[j];
            int v_next = state.order[(j + 1) % n];

            delta = get_dist(dist_table, u_prev, u_next, n) + get_dist(dist_table, v, u, n) + get_dist(dist_table, u, v_next, n) 
                  - (get_dist(dist_table, u_prev, u, n) + get_dist(dist_table, u, u_next, n) + get_dist(dist_table, v, v_next, n));
            return true;
        }

        void apply(TspState& state) const { // 実際に反映するとき
            if (i < j) {
                std::rotate(state.order.begin() + i, state.order.begin() + i + 1, state.order.begin() + j + 1);
                for (int k=i; k<=j; ++k) state.pos[state.order[k]] = k;
            } else {
                std::rotate(state.order.begin() + j + 1, state.order.begin() + i, state.order.begin() + i + 1);
                for (int k=j+1; k<=i; ++k) state.pos[state.order[k]] = k;
            }
            state.current_dist += delta;
        }
    };
}

std::vector<int> solve_tsp(const std::vector<city>& cities, std::vector<std::vector<int>>& candidates, int limit){
    auto init_order = two_opt(cities, greedy_order(cities)); // 初期解は greedy + 2-opt
    TspState state(init_order, cities);

    int n = cities.size();

    std::vector<float> dist_table(n*n); // 各ノードの距離を前計算
    for (int i=0; i<n; ++i) {
        for (int j = 0; j < n; ++j) {
            dist_table[i * n + j] = std::sqrt(calc_distance_squared(cities[i].position, cities[j].position));
        }
    }
    auto get_dist = [&](int i, int j) -> double {
        return dist_table[i*n + j];
    };

    Timer timer;
    const int TIME_LIMIT_MS = limit * 60 * 1000; // limit 分行う

    std::vector<int> best_order = state.order;
    double best_dist = state.current_dist;

    // 初期温度の計算
    double sum_delta = 0.0;
    int sample_count = 0;
    for (int k = 0; k < 2000; ++k) {
        int i = xor_shift() % n, j = xor_shift() % n;
        Neighborhood::TwoOpt op;
        if (op.prepare(state, dist_table, i, j, n) && op.delta > 0) {
            sum_delta += op.delta; sample_count++;
        }
    }
    double start_temp = -(sum_delta / (sample_count > 0 ? sample_count : 1)) / std::log(0.3);
    
    // 初期化
    CoolingScheduler scheduler(start_temp, 0.01, TIME_LIMIT_MS);
    long long iteration_count = 0;

    // 各近傍操作インスタンス
    Neighborhood::TwoOpt n_2opt;
    Neighborhood::Swap n_swap;
    Neighborhood::Insert n_insert;
    double current_temp = start_temp;
    while(true) {
        if ((iteration_count & 4095) == 0) { // 時間管理
            if (timer.elapsed_ms() > TIME_LIMIT_MS) break;
            current_temp = scheduler.get_temperature(timer.elapsed_ms());
        }

        // 遷移先は、近傍または完全にランダム
        int i = xor_shift() % n;
        int j = 0;
        int u_current = state.order[i];
        if (rnd() < 0.9 && !candidates[u_current].empty()) {
            j = state.pos[candidates[u_current][xor_shift() % candidates[u_current].size()]];
        } else {
            j = xor_shift() % n;
        }

        // ランダムに操作を決定する
        int op = xor_shift() % 10;
        double delta = 0;
        bool is_valid = false;

        if (op < 7) {
            is_valid = n_2opt.prepare(state, dist_table, i, j, n);
            delta = n_2opt.delta;
        } else if (op < 9) {
            is_valid = n_swap.prepare(state, dist_table, i, j);
            delta = n_swap.delta;
        } else {
            is_valid = n_insert.prepare(state, dist_table, i, j);
            delta = n_insert.delta;
        }

        if (!is_valid) continue;

        // apply するかどうか
        if (delta <= 0 || rnd() < std::exp(-delta / current_temp)) {
            if (op < 7) n_2opt.apply(state);
            else if (op < 9) n_swap.apply(state);
            else n_insert.apply(state);

            if (state.current_dist < best_dist) { //ベストスコアの更新
                best_dist = state.current_dist;
                best_order = state.order;
            }
        }
        iteration_count++;
    }
    std::cout << iteration_count << '\n';

    return two_opt(cities, best_order);
}

void output_answer(const std::vector<int>& answer, const std::string& output_file_name) {
    std::ofstream output(output_file_name);
    output << "index" << '\n';
    for(const auto& city_index: answer) { //  各街の index を一行ずつ出力
        output << city_index << '\n';
    }
}

int main() {
    std::array<int, 2> minutes = {5, 10};
    for(int i=6; i<=7; ++i) {
        std::string input_file_name = std::format("./input_{}.csv", i);
        std::string output_file_name = std::format("./output_{}.csv", i);

        std::vector<city> cities = read_cities(input_file_name); // 全ての街の座標が順に入っている
        std::vector<std::vector<int>> candidates = nearly_city(cities); // 上位15個の近傍の街が入っている
        std::vector<int> order_of_cities = solve_tsp(cities, candidates, minutes[i-6]); // TSP を解く
        output_answer(order_of_cities, output_file_name); // 出力する

        std::cout << i << std::endl;
    }
}