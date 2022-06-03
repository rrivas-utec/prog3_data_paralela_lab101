#include <iostream>
#include <random>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <thread>
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
    auto vec = generar_vector("../datos_20.txt");
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
    int rango = size(vec) / n_hilo;
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

void ejemplo_3() {
    auto vec = generar_vector("../datos_20.txt");
    int s1 = 0;
    int n_hilos = 4;
    int rango = size(vec) / n_hilos;

    vector<thread> v_hilos(n_hilos);
    vector<int>  v_sub_total(n_hilos);

    auto iter = begin(vec);
    auto riter = begin(v_sub_total);
    int i = 0;
    thread t1(summarize<vector<int>::iterator>, iter, rango, ref(s1));
    int s2 = 0;
    advance(iter, rango);
    thread t2(summarize<decltype(begin(vec))>, iter, i*rango), rango, ref(s2));
    int s3 = 0;
    ++i;
    thread t3([&vec, rango, i, &s3]{summarize(next(iter, rango, ref(s3));});
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


int main() {
//    generar_archivo("../datos_20.txt", 20);
    ejemplo_1();
    ejemplo_2();
    return 0;
}
