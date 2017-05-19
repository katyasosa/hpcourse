#include <iostream>
#include <fstream>
#include <algorithm>
#include <array>
#include <cmath>

#include <tbb/flow_graph.h>

using namespace std;
using namespace tbb::flow;

typedef tuple<int, int> coord;

struct pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

using image = vector<vector<pixel>>;

image imread(const std::string& path) {
    if (path.compare(path.size() - 4, 4, ".dat") != 0) {
        cerr << "Can read only prepared .dat files!" << endl;
        throw invalid_argument(path);
    }

    ifstream file(path, ios::binary | ios::in);

    std::uint32_t h, w, d;
    file.read(reinterpret_cast<char*>(&h), 4);
    file.read(reinterpret_cast<char*>(&w), 4);
    file.read(reinterpret_cast<char*>(&d), 4);

    auto data = vector<vector<pixel>>(h);
    for (auto& row: data) {
        row.resize(w);
    }

    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            auto pix = array<char, 3>();
            file.read(pix.data(), 3);
            data[i][j] = pixel { uint8_t(pix[0]),
                                 uint8_t(pix[1]),
                                 uint8_t(pix[2])};
        }
    }

    return data;
}

void imwrite(const image& source, const string& path) {
    int h = source.size();
    int w = source[0].size();
    int d = 3;
    ofstream file(path, ios::binary);
    file.write(reinterpret_cast<char*>(&h), 4);
    file.write(reinterpret_cast<char*>(&w), 4);
    file.write(reinterpret_cast<char*>(&d), 4);

    for (auto& row : source) {
        for (auto& pix: row) {
            file.write(reinterpret_cast<const char*>(&pix.r), 1);
            file.write(reinterpret_cast<const char*>(&pix.g), 1);
            file.write(reinterpret_cast<const char*>(&pix.b), 1);
        }
    }
    file.close();
}


int main() {
    image big_image = imread("/home/user/Documents/study/parallel/hpcourse/csc/2017/3.TBBFlowGraph/data/image.dat");
    int big_n_rows = big_image.size(), big_n_cols = big_image[0].size();

    // 1. Считываем маленькое изображение
    string default_path = "/home/user/Documents/study/parallel/hpcourse/csc/2017/3.TBBFlowGraph/data/cheer.dat";
    image small_image = imread(default_path);
    int small_n_rows = small_image.size(), small_n_cols = small_image[0].size();

    cout << "Big image size: " << big_n_rows << " x " << big_n_cols << endl
            << "Small image size: " << small_n_rows << " x " << small_n_cols << endl;

    graph g;

    // 2. Узел, генерирующий из большого изображения все возможные прямоугольники размера искомого изображения
    int cur_step = 0, steps_by_row = (big_n_cols - small_n_cols + 1),
            max_step = steps_by_row * (big_n_rows - small_n_rows + 1);
    source_node<coord> generate_subimages(g, [&]( coord &xy ) {
        if (cur_step < max_step - 1) {
            int step = ++cur_step;
            xy = coord(step / steps_by_row, step % steps_by_row);
            return true;
        } else {
            return false;
        }
    }, false);

    // 3. Буферный узел
    buffer_node<coord> subimages_buffer(g);

    // 4. Узел, подсчитывающий разницу между искомым изображением и кандидатом
    function_node<coord, tuple<int, coord>> diff_node(g, 1, [&](coord xy) {
        return tuple<int, coord>(123, xy); // TODO
    });

    // 5. Узел, содержащий результат - минимальную разницу и координаты верхнего левого угла.


    // 6. Узел, записывающий окрестность найденного изображения в файл.


    // Соединяем вершины ребрами
    make_edge(generate_subimages, subimages_buffer);
    make_edge(subimages_buffer, diff_node);


    // Запускаем
    generate_subimages.activate();

    g.wait_for_all();
}