#include <iostream>
#include <random>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <thread>
#include <cmath>
#include <future>
using namespace std;

int rand_int(int start, int stop) {
    random_device rd; // vaso
    uniform_int_distribution<int> dis(start, stop); // dado
    return dis(rd);
}

void generar_archivo(string file_name, int sz) {
    ofstream file(file_name);
    if (file.fail()) {
        cout << "Error al crear\n";
    }
    for (int i = 0; i < sz; ++i)
        file << rand_int(1, 10) << " ";
}

vector<int> generar_vector(string file_name) {
    ifstream file(file_name);
    if (file.fail()) {
        cout << "Error al leerlo\n";
    }
    vector<int> result;
    copy(istream_iterator<int>(file), istream_iterator<int>(), back_inserter(result));
    return result;
}

void ejemplo_1() {
    auto vec = generar_vector("../datos_2X.txt");
    cout << size(vec) << endl;
    copy(begin(vec), end(vec), ostream_iterator<int>(cout, " "));
    cout << endl;
    cout << accumulate(begin(vec), end(vec), 0) << endl;
}

void summarize(int first, int stop, vector<int>& vec, int& sub_total) {
    sub_total = accumulate(
            next(begin(vec), first),
            next(begin(vec), stop), 0);
}

template <typename Iterator>
void summarize(Iterator start, int range, int& sub_total) {
    sub_total = accumulate(start, next(start, range), 0);
}

void ejemplo_2() {
    auto vec = generar_vector("../datos_20.txt");
    int s1 = 0;
    int n_hilo = 4;
    int sz = size(vec);
    int rango =  ceil((sz * 1.0)/ n_hilo);
    int i = 0;
    thread t1(summarize<vector<int>::iterator>, next(begin(vec), i*rango), rango, ref(s1));
    int s2 = 0;
    ++i;
    thread t2(summarize<decltype(begin(vec))>, next(begin(vec), i*rango), rango, ref(s2));
    int s3 = 0;
    ++i;
    thread t3([&vec, rango, i, &s3]{summarize(next(begin(vec), i*rango), rango, ref(s3));});
    int s4 = 0;
    ++i;
    thread t4([&vec, rango, i, &s4]{summarize(next(begin(vec), i*rango), rango, ref(s4));});

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    auto total = s1 + s2 + s3 + s4;
    cout << total << endl;
}

template <typename Iterator>
void summarize_2(Iterator start, int range, Iterator result) {
    *result = accumulate(start, next(start, range), 0);
}

void ejemplo_3() {
    // Generar un vector a partir de un archivo
    auto vec = generar_vector("../datos_20.txt");
    int s1 = 0;

    // Calcula hilos y rango
    int n_hilos = 4;
    int sz = size(vec);
    int range =  ceil((sz * 1.0)/ n_hilos);

    // Genera los contenedores de hilos y de subtotales
    vector<thread> v_hilos(n_hilos);
    vector<int> v_sub_total(n_hilos);

    // Generar 2 iteradores
    auto iter = begin(vec);             // Iterador de los datos
    auto riter = begin(v_sub_total);    // Iterador de los subtotal

    thread t1(summarize_2<decltype(iter)>, iter, range, riter);
    ++riter;
    advance(iter, range);

    thread t2(summarize_2<decltype(iter)>, iter, range, riter);
    ++riter;
    advance(iter, range);

    thread t3(summarize_2<decltype(iter)>, iter, range, riter);
    ++riter;
    advance(iter, range);

    thread t4(summarize_2<decltype(iter)>, iter, range, riter);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    auto total = accumulate(begin(v_sub_total), end(v_sub_total), 0);
    cout << total << endl;
}

void ejemplo_4() {
    // Generar un vector a partir de un archivo
    auto vec = generar_vector("../datos_2X.txt");

    // Calcula hilos y rango
    int n_hilos = 4;
    int sz = size(vec);
    size_t range =  ceil((sz * 1.0)/ n_hilos);

    // Genera los contenedores de hilos y de subtotales
    vector<thread> v_hilos(n_hilos);
    vector<int> v_sub_total(n_hilos);

    // Generar 2 iteradores
    auto iter = begin(vec);             // Iterador de los datos
    auto riter = begin(v_sub_total);    // Iterador de los subtotal

//    for (auto& h: v_hilos) {
//        if (distance(iter, end(vec)) < range)
//            range = distance(iter, end(vec));
//
//        h = thread(summarize_2<decltype(iter)>, iter, range, riter);
//        ++riter;
//        advance(iter, range);
//    }
    for_each(begin(v_hilos), prev(end(v_hilos)), [&, range](auto& hilo) {
       hilo = thread(summarize_2<decltype(iter)>, iter, range, riter);
        ++riter;
        advance(iter, range);
    });
    auto it_hilo = prev(end(v_hilos));
    range = distance(iter, end(vec));
    *it_hilo = thread(summarize_2<decltype(iter)>, iter, range, riter);

    for (auto& h: v_hilos)
        h.join();

    auto total = accumulate(begin(v_sub_total), end(v_sub_total), 0);
    cout << total << endl;
}

const int expected_range = 25;

int get_number_of_threads(int sz, int rng) {
    int max_threads = (sz + rng - 1) / rng;
    int k_threads = static_cast<int>(thread::hardware_concurrency());
    return min(k_threads != 0? k_threads: 2, max_threads);
}

template <typename Iterator>
auto accumulate_par(Iterator start, Iterator stop, typename Iterator::value_type initial) {

    // Calcula hilos y rango
    int sz = distance(start, stop);
    int n_hilos = get_number_of_threads(sz, expected_range);
    size_t range =  ceil((sz * 1.0)/ n_hilos);

    // Genera los contenedores de hilos y de subtotales
    vector<thread> v_hilos(n_hilos);
    vector<int> v_sub_total(n_hilos);

    // Generar 2 iteradores
    auto iter = start;             // Iterador de los datos
    auto riter = begin(v_sub_total);    // Iterador de los subtotal

    for_each(begin(v_hilos), prev(end(v_hilos)), [&, range](auto& hilo) {
        hilo = thread(summarize_2<decltype(iter)>, iter, range, riter);
        ++riter;
        advance(iter, range);
    });
    auto it_hilo = prev(end(v_hilos));
    range = distance(iter, stop);
    *it_hilo = thread(summarize_2<decltype(iter)>, iter, range, riter);

    for (auto& h: v_hilos)
        h.join();

    return accumulate(begin(v_sub_total), end(v_sub_total), initial);
}

void ejemplo_5() {
    // Generar un vector a partir de un archivo
    auto vec = generar_vector("../datos_2X.txt");
    cout << accumulate_par(begin(vec), end(vec), 0) << endl;
}

template <typename Iterator, typename T = typename Iterator::value_type>
auto accumulate_par_async(Iterator start, Iterator stop, T initial) {
    // Calcula hilos y rango
    int sz = distance(start, stop);
    int n_hilos = get_number_of_threads(sz, expected_range);
    size_t range =  ceil((sz * 1.0)/ n_hilos);

    // Genera los contenedores de hilos y de subtotales
    vector<future<T>> v_futures(n_hilos);

    // Generar 2 iteradores
    auto iter = start;             // Iterador de los datos

    for_each(begin(v_futures), prev(end(v_futures)), [&, range](auto& fut) {
        fut = async(std::accumulate<Iterator, T>, iter, next(iter, range), initial);
        advance(iter, range);
    });

    auto it_future = prev(end(v_futures));
    range = distance(iter, stop);
    *it_future = async(accumulate<Iterator, T>, iter, next(iter, range), initial);

    return accumulate(begin(v_futures), end(v_futures), initial,
    [](auto subtotal, auto& fut) { return subtotal + fut.get(); });
}

void ejemplo_6() {
    // Generar un vector a partir de un archivo
    auto vec = generar_vector("../datos_2X.txt");
    cout << accumulate_par_async(begin(vec), end(vec), 0) << endl;
}

template <typename Iterator>
auto accumulator_rec_async_(Iterator start, Iterator stop, int range) {
    // Condición Base
    auto size = distance(start, stop);
    if (size < range) return accumulate(start, stop, 0);
    // Condicion Recursiva
    auto middle = next(start, size / 2);
    auto a = async(accumulator_rec_async_<Iterator>, start, middle, range);
    auto b = async(accumulator_rec_async_<Iterator>, middle, stop, range);
    return a.get() + b.get();
}

template <typename Iterator, typename T = typename Iterator::value_type>
T accumulator_recursive_async(Iterator start, Iterator stop, T initial) {
    auto size = distance(start, stop);
    int n_hilos = get_number_of_threads(size, expected_range);
    auto rango = size / n_hilos;
    return initial + accumulator_rec_async_(start, stop, rango) ;
}

void ejemplo_7() {
    // Generar un vector a partir de un archivo
    auto vec = generar_vector("../datos_2X.txt");
    cout << accumulator_recursive_async(begin(vec), end(vec), 0) << endl;
}

int main() {
    generar_archivo("../datos_2X.txt", 25);
    ejemplo_1();
    ejemplo_2();
    ejemplo_3();
    ejemplo_4();
    ejemplo_5();
    ejemplo_6();
    ejemplo_7();
    return 0;
}
