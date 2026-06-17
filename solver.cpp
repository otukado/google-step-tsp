#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>
#include <array>
#include <algorithm>
#include <format>
#include <iostream>

constexpr double INF = std::numeric_limits<double>::infinity();

// それぞれの街の点を表す struct
struct point_2D {
    double x = 0;
    double y = 0;
};

struct city {
    point_2D position;
};


// input file の名前を受け取り、街の配列を返す関数
std::vector<city> read_cities(const std::string input_file_name) {
    std::vector<city> cities;
    std::ifstream input(input_file_name);
    std::string line;
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
        cities.push_back({input_position});
    }

    return cities;
}

// 2乗を返す
double square(double x) {
    return x * x;
}

std::vector<int> greedy_order(const std::vector<city>& cities) {
    int number_of_cities = static_cast<int>(cities.size()); // 街の数
    auto visited = std::vector(number_of_cities, false); // 各街をすでに訪れたかどうか
    std::vector<int> answer;

    // 一周する問題だから、どの点からスタートしても問題ないので、街 0 からスタートしてよい
    point_2D start = cities[0].position;
    visited[0] = true;
    answer.push_back(0);

    point_2D current_position = start; // 現在の座標。最初なので、スタートする街

    while(true) {
        double min_distance = INF; //今までの候補の中で最も近い場所との距離
        int nearest_city;

        for(int i=1; i<number_of_cities; ++i) {
            if(visited[i]) continue;
            auto [x_candidate, y_candidate] = cities[i].position;
            double distance = square(x_candidate - current_position.x) + square(y_candidate - current_position.y); //現在いる都市との距離の二乗

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


// bool is_over(const point_2D& a, const point_2D& b, const point_2D& c) {
//     return (a.y - b.y) * (c.x - a.x) < (c.y - a.y) * (a.x - b.x);
// }


// bool is_cross(const std::array<point_2D, 2>& first_line, const std::array<point_2D, 2>& second_line) {
//     if(first_line[0].x == first_line[1].x) return false;
//     if(first_line[1].x == second_line[0].x && first_line[1].y == second_line[0].y) return false; // 同じ点があれば、その時は確実にクロスではない
//     if(first_line[1].x == second_line[1].x && first_line[1].y == second_line[1].y) return false;
//     if(first_line[0].x == second_line[0].x && first_line[0].y == second_line[0].y) return false;
//     if(first_line[0].x == second_line[1].x && first_line[0].y == second_line[1].y) return false;
//     auto [x_1, y_1] = first_line[0]; // 点1
//     auto [x_2, y_2] = first_line[1]; // 点2
//     auto [x_3, y_3] = second_line[0]; // 点3
//     auto [x_4, y_4] = second_line[1]; // 点4

//     bool point3_is_above_first_line = is_over(first_line[0], first_line[1], second_line[0]); //点3 が first_line の上にあるかどうか
//     bool point4_is_above_first_line = is_over(first_line[0], first_line[1], second_line[1]); //点4 が first_line の上にあるかどうか
//     bool is_divided_by_first_line = point3_is_above_first_line != point4_is_above_first_line; // 点3 と点4 が first_line を挟んで反対側にあるか否か

//     bool point1_is_above_second_line = is_over(second_line[0], second_line[1], first_line[0]); //点1 が second_line の上にあるかどうか
//     bool point2_is_above_second_line = is_over(second_line[0], second_line[1], first_line[1]); //点2 が second_line の上にあるかどうか
//     bool is_divided_by_second_line = point1_is_above_second_line != point2_is_above_second_line; // 点1 と点2 が second_line を挟んで反対側にあるか否か

//     return is_divided_by_first_line && is_divided_by_second_line;
// };

double calc_distance_squared(const point_2D& a, const point_2D& b) {
    return square(a.x - b.x) + square(a.y - b.y);
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

std::vector<int> solve_tsp(const std::vector<city>& cities){
    auto greedy_answer = greedy_order(cities); //初めに貪欲法
    auto two_opt_answer = two_opt(cities, greedy_answer); // そのあとに 2-opt

    return two_opt_answer;
}

void output_answer(const std::vector<int>& answer, const std::string& output_file_name) {
    std::ofstream output(output_file_name);
    output << "index" << '\n';
    for(const auto& city_index: answer) { //  各街の index を一行ずつ出力
        output << city_index << '\n';
    }
}

int main() {
    int i = 5;
    for(int i=0; i<=6; ++i) {
        std::string input_file_name = std::format("./input_{}.csv", i);
        std::string output_file_name = std::format("./output_{}.csv", i);

        std::vector<city> cities = read_cities(input_file_name); // 全ての街の座標が順に入っている
        std::vector<int> order_of_cities = solve_tsp(cities); // TSP を解く
        output_answer(order_of_cities, output_file_name); // 出力する

        std::cout << i << std::endl;
    }
}