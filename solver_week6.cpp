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
#include <random>

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

static uint32_t y = 2463534242;
uint32_t xor_shift() {    
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

std::vector<std::vector<int>> nearly_city(const std::vector<city>& cities, int k){ // 近い 15 の街の保存
    int n = cities.size();

    std::vector<std::vector<int>> response(n);

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
    double get_dist(const std::vector<double>& dist_table, int u, int v, int n) {
        return dist_table[u * n + v];
    }

    // 2-opt
    struct TwoOpt {
        int i, j;
        double delta;

        bool prepare(const TspState& state, const std::vector<double>& dist_table, int i_idx, int j_idx, int n) {
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

    // [3-opt近傍] 3本のエッジを切り、7通りの繋ぎ替えから最適なものを選ぶ
    struct ThreeOpt {
        int i, j, k;
        int pattern;
        double delta;

        bool prepare(const TspState& state, const std::vector<double>& dist_table, int i_idx, int j_idx, int k_idx, int n) {
            // インデックスを昇順 (i < j < k) にソートする
            int arr[3] = {i_idx, j_idx, k_idx};
            if (arr[0] > arr[1]) std::swap(arr[0], arr[1]);
            if (arr[1] > arr[2]) std::swap(arr[1], arr[2]);
            if (arr[0] > arr[1]) std::swap(arr[0], arr[1]);
            i = arr[0]; j = arr[1]; k = arr[2];

            // 頂点が共有される（エッジが隣接する）ケースや長さ0の区間を排除
            if (i == j || j == k) return false;
            if (i + 1 == j || j + 1 == k || (k + 1) % n == i) return false;

            int a = state.order[i],      b = state.order[i + 1];
            int c = state.order[j],      d = state.order[j + 1];
            int e = state.order[k],      f = state.order[(k + 1) % n];

            // 元の3エッジの距離
            double d_ab = get_dist(dist_table, a, b, n);
            double d_cd = get_dist(dist_table, c, d, n);
            double d_ef = get_dist(dist_table, e, f, n);
            double current_dist = d_ab + d_cd + d_ef;

            // 7通りの繋ぎ替えパターンの距離を計算
            double d1 = get_dist(dist_table, a, c, n) + get_dist(dist_table, b, d, n) + d_ef; // 2-opt
            double d2 = d_ab + get_dist(dist_table, c, e, n) + get_dist(dist_table, d, f, n); // 2-opt
            double d3 = get_dist(dist_table, a, c, n) + get_dist(dist_table, b, e, n) + get_dist(dist_table, d, f, n); // 3-opt
            double d4 = get_dist(dist_table, a, d, n) + get_dist(dist_table, e, b, n) + get_dist(dist_table, c, f, n); // 3-opt
            double d5 = get_dist(dist_table, a, d, n) + get_dist(dist_table, e, c, n) + get_dist(dist_table, b, f, n); // 3-opt
            double d6 = get_dist(dist_table, a, e, n) + get_dist(dist_table, d, b, n) + get_dist(dist_table, c, f, n); // 3-opt
            double d7 = get_dist(dist_table, a, e, n) + d_cd + get_dist(dist_table, b, f, n); // 2-opt (全体反転)

            // SAなので、7パターンのうち「最も delta が小さいもの」をこの遷移の代表とする
            double min_d = d1; pattern = 1;
            if (d2 < min_d) { min_d = d2; pattern = 2; }
            if (d3 < min_d) { min_d = d3; pattern = 3; }
            if (d4 < min_d) { min_d = d4; pattern = 4; }
            if (d5 < min_d) { min_d = d5; pattern = 5; }
            if (d6 < min_d) { min_d = d6; pattern = 6; }
            if (d7 < min_d) { min_d = d7; pattern = 7; }

            delta = min_d - current_dist;
            return true;
        }

        void apply(TspState& state) const {
            auto it1 = state.order.begin() + i + 1;
            auto it2 = state.order.begin() + j + 1;
            auto it3 = state.order.begin() + k + 1;

            // C++の標準関数 (reverse と rotate) を使えば、どんな複雑な3-optの繋ぎ替えも
            // 補助配列なしでインプレース（メモリ確保ゼロ）で超高速に実現できる
            if (pattern == 1)      { std::reverse(it1, it2); }
            else if (pattern == 2) { std::reverse(it2, it3); }
            else if (pattern == 3) { std::reverse(it1, it2); std::reverse(it2, it3); }
            else if (pattern == 4) { std::rotate(it1, it2, it3); }
            else if (pattern == 5) { std::reverse(it1, it2); std::rotate(it1, it2, it3); }
            else if (pattern == 6) { std::reverse(it2, it3); std::rotate(it1, it2, it3); }
            else if (pattern == 7) { std::reverse(it1, it3); }

            // 影響を受けた区間のpos(逆引き)を更新
            for (int x = i + 1; x <= k; ++x) {
                state.pos[state.order[x]] = x;
            }
            state.current_dist += delta;
        }
    };

    // i と j を入れ替え
    struct Swap {
        int i, j;
        double delta;

        bool prepare(const TspState& state, const std::vector<double>& dist_table, int i_idx, int j_idx) {
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

        bool prepare(const TspState& state, const std::vector<double>& dist_table, int i_idx, int j_idx) {
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

double calc_initial_temp(const TspState& state, const std::vector<double>& dist_table, const std::vector<std::vector<int>>& candidates, int n, 
     int ratio_2opt, int ratio_3opt, int ratio_swap, int ratio_insert, double target_prob = 0.6) {
    int sample_count = 0;
    double sum_delta = 0.0;
    
    Neighborhood::TwoOpt n_2opt;
    Neighborhood::ThreeOpt n_3opt;
    Neighborhood::Swap n_swap;
    Neighborhood::Insert n_insert;

    const double neighbor_prob = 0.80; // 本番スタート時の確率に合わせる

    for (int iter = 0; iter < 1000; ++iter) {
        // 1. 本番と全く同じルールで i, j, k を選ぶ
        int i = xor_shift() % n;
        int j;
        int u_current = state.order[i];
        if (rnd() < neighbor_prob && !candidates[u_current].empty()) {
            int idx = static_cast<int>(rnd() * rnd() * candidates[u_current].size());
            j = state.pos[candidates[u_current][idx]];
        } else {
            j = xor_shift() % n;
        }

        int k;
        int v_current = state.order[j];
        if (rnd() < neighbor_prob && !candidates[v_current].empty()) {
            int idx = static_cast<int>(rnd() * candidates[v_current].size());
            k = state.pos[candidates[v_current][idx]];
        } else {
            k = xor_shift() % n;
        }
        
        bool is_valid = false;
        int op = xor_shift() % (ratio_2opt + ratio_3opt + ratio_swap + ratio_insert);
        int bound_3opt = ratio_2opt + ratio_3opt;
        int bound_swap = bound_3opt + ratio_swap;
        double delta = 0;

        if (op < ratio_2opt) {
            is_valid = n_2opt.prepare(state, dist_table, i, j, n);
            delta = n_2opt.delta;
        } else if(op < bound_3opt) {
            is_valid = n_3opt.prepare(state, dist_table, i, j, k, n);
            delta = n_3opt.delta;
        } else if (op < bound_swap) {
            is_valid = n_swap.prepare(state, dist_table, i, j);
            delta = n_swap.delta;
        } else {
            is_valid = n_insert.prepare(state, dist_table, i, j);
            delta = n_insert.delta;
        }

        if (is_valid && delta > 1e-6) {
            sum_delta += delta;
            sample_count++;
        }
    }

    if (sample_count == 0) return 30.0; // 実際より小さめのフェイルセーフ値に調整
    
    double avg_delta = sum_delta / sample_count;
    return -avg_delta / std::log(target_prob);
}

std::vector<int> solve_tsp(const std::vector<city>& cities, std::vector<std::vector<int>>& candidates, int limit, int ratio_2opt, int ratio_3opt, int ratio_swap, int ratio_insert, double target_prob){
    auto init_order = two_opt(cities, greedy_order(cities)); // 初期解は greedy + 2-opt
    TspState state(init_order, cities);

    int n = cities.size();

    std::vector<double> dist_table(n*n); // 各ノードの距離を前計算
    for (int i=0; i<n; ++i) {
        for (int j = 0; j < n; ++j) {
            dist_table[i * n + j] = std::sqrt(calc_distance_squared(cities[i].position, cities[j].position));
        }
    }
    auto get_dist = [&](int i, int j) -> double {
        return dist_table[i*n + j];
    };

    Timer timer;
    const int TIME_LIMIT_MS = limit * 1000; // limit 秒行う

    std::vector<int> best_order = state.order;
    double best_dist = state.current_dist;

    const double start_temp = calc_initial_temp(state, dist_table, candidates, n, ratio_2opt, ratio_3opt, ratio_swap, ratio_insert, target_prob);
    const double end_temp = 0.01;


    // 初期化
    CoolingScheduler scheduler(start_temp, end_temp, TIME_LIMIT_MS);
    long long iteration_count = 0;

    // 各近傍操作インスタンス
    Neighborhood::TwoOpt n_2opt;
    Neighborhood::ThreeOpt n_3opt;
    Neighborhood::Swap n_swap;
    Neighborhood::Insert n_insert;
    double current_temp = start_temp;
    double neighbor_prob = 0.80;

    while(true) {
        if ((iteration_count & 4095) == 0) { // 時間管理
            if (timer.elapsed_ms() > TIME_LIMIT_MS) break;
            current_temp = scheduler.get_temperature(timer.elapsed_ms());
            double progress = static_cast<double>(timer.elapsed_ms()) / TIME_LIMIT_MS;
            neighbor_prob = 0.80 + 0.20 * std::min(1.0, progress);

            double true_dist = 0.0;
            for (int i=0; i<n; ++i) {
                true_dist += get_dist(state.order[i], state.order[(i + 1) % n]);
            }
            state.current_dist = true_dist;
        }
        

        // 遷移先は、近傍または完全にランダム
        int i = xor_shift() % n;
        int j;
        int u_current = state.order[i];
        if (rnd() < neighbor_prob && !candidates[u_current].empty()) {
            int idx = static_cast<int>(rnd() * rnd() * candidates[u_current].size());
            j = state.pos[candidates[u_current][idx]];
        } else {
            j = xor_shift() % n;
        }
        int k;
        int v_current = state.order[j];
        if (rnd() < neighbor_prob && !candidates[v_current].empty()) {
            int idx = static_cast<int>(rnd() * candidates[v_current].size());
            k = state.pos[candidates[v_current][idx]];
        } else {
            k = xor_shift() % n;
        }

        // // ランダムに操作を決定する
        // int ratio_2opt = 70;
        // int ratio_3opt = 5;
        // int ratio_swap = 15;
        // int ratio_insert = 10;
        int op = xor_shift() % (ratio_2opt + ratio_3opt + ratio_swap + ratio_insert);
        int bound_3opt = ratio_2opt + ratio_3opt;
        int bound_swap = bound_3opt + ratio_swap;

        double delta = 0;
        bool is_valid = false;

        if (op < ratio_2opt) {
            is_valid = n_2opt.prepare(state, dist_table, i, j, n);
            delta = n_2opt.delta;
        } else if(op < bound_3opt) {
            is_valid = n_3opt.prepare(state, dist_table, i, j, k, n);
            delta = n_3opt.delta;
        } else if (op < bound_swap) {
            is_valid = n_swap.prepare(state, dist_table, i, j);
            delta = n_swap.delta;
        } else {
            is_valid = n_insert.prepare(state, dist_table, i, j);
            delta = n_insert.delta;
        }

        if (!is_valid) continue;

        // apply するかどうか
        if (delta <= 0 || rnd() < std::exp(-delta / current_temp)) {
            if (op < ratio_2opt) n_2opt.apply(state);
            else if(op < bound_3opt) n_3opt.apply(state);
            else if (op < bound_swap) n_swap.apply(state);
            else n_insert.apply(state);

            if (state.current_dist < best_dist) { //ベストスコアの更新
                best_dist = state.current_dist;
                best_order = state.order;
            }
        }
        iteration_count++;
    }
    // std::cout << iteration_count << '\n';

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
    int r_2opt = 35, r_3opt = 26, r_swap = 2, r_insert = 18, k = 9, targe_prob = 0.128;
    std::random_device rd;
    y = rd();
    std::array<int, 2> minutes = {180, 60};
    for(int i=6; i<=7; ++i) {
        std::string input_file_name = std::format("./input_{}.csv", i);
        std::string output_file_name = std::format("./output_{}.csv", i);

        std::vector<city> cities = read_cities(input_file_name); // 全ての街の座標が順に入っている
        std::vector<std::vector<int>> candidates = nearly_city(cities, k); // 上位40個の近傍の街が入っている
        double best_score = 43000.0;
        std::vector<int> order_of_cities;
        for(int i=0; i<5; ++i) {
            if()
        }
        std::vector<int> order_of_cities = solve_tsp(cities, candidates, minutes[i-6], 35, 26, 2,18, 0.128); // TSP を解く
        output_answer(order_of_cities, output_file_name); // 出力する

        std::cout << i << std::endl;
    }
}

// int main(int argc, char* argv[]) {
//     // デフォルト値
//     int r_2opt = 70, r_3opt = 5, r_swap = 15, r_insert = 10, k = 15;
//     double target_prob = 300.0;
    
//     // コマンドライン引数があれば上書き
//     if (argc >= 7) {
//         r_2opt   = std::stoi(argv[1]);
//         r_3opt   = std::stoi(argv[2]);
//         r_swap   = std::stoi(argv[3]);
//         r_insert = std::stoi(argv[4]);
//         k = std::stoi(argv[5]);
//         target_prob = std::stod(argv[6]);
//     }

//     std::random_device rd;
//     y = rd();
    
//     // チューニング時は評価を高速化するため、ファイル1つ（1分）だけでテストするのがおすすめ
//     std::string input_file_name = "./input_7.csv"; 
//     std::vector<city> cities = read_cities(input_file_name);
//     std::vector<std::vector<int>> candidates = nearly_city(cities, k);
    
//     // TSPを解く (10秒間)
//     std::vector<int> order_of_cities = solve_tsp(cities, candidates, 30, r_2opt, r_3opt, r_swap, r_insert, target_prob);
    
//     // 最終的な「本当の総距離」を計算して、標準出力にそれ「だけ」を出す
//     double final_dist = calc_total_distance(cities, order_of_cities);
//     std::cerr <<final_dist << std::endl;
//     std::cout << final_dist << std::endl; 

//     return 0;
// }