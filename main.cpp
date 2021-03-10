#include "fourier.hpp"

#include "matplotlibcpp.h"

#include <iostream>
#include <vector>
#include <chrono>
#include <functional>

auto invoke_tm_chrono = [](auto&& func, auto&&... args) -> double {
            std::cout << "std::chrono(): ";
            auto start = std::chrono::system_clock::now();
            std::forward<decltype(func)>(func)(std::forward<decltype(args)>(args)...); // 完全転送
            auto end = std::chrono::system_clock::now();
            auto duration = end - start;
            double duration_micro_s = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
            std::cout << duration_micro_s << "[µs]" << std::endl;
            return duration_micro_s;
        };

namespace plt = matplotlibcpp;

int main(int, char**) 
{
    std::cout << "Hello, Fourier!\n";

    // ダミーデータ(sin)
    const int N = 32;
    double basic_freq = 2 * 3.141659 / N;
    std::vector<double> x(N);
    std::vector<double> sine(N);
    for (int i = 0; i < N; ++i)
    {
        x[i] = basic_freq * i;
        sine[i] = std::sin(x[i]) + std::sin(5 * x[i]) + std::sin(10 * x[i]); //+ std::sin(30 * x[i]); // ナイキスト周波数16に対して30なので、エイリアシング発生
    }

    plt::plot(x, sine, "b");
    plt::xlim(0, 7);
    plt::show();

    Fourier<fft::CooleyTurkey> fourier(N);
    auto rotors = fourier.rotors();
    // std::for_each(std::begin(rotors), std::end(rotors), [] (auto value) {
    //     std::cout << "rotor: " << value << std::endl;
    // });

    std::cout << "data_size: " << fourier.size() << std::endl;

    std::vector<int> k(N);
    for (int i = 0; i < k.size(); ++i)
    {
        // k.at(i) = i * basic_freq;
        k.at(i) = i;
    }
    std::printf("basic_freq=%f\n", basic_freq);

    // DFT
    double* data = &(*std::begin(sine));
    std::cout << "-----DFT-----" << std::endl;
    // std::bindを使うよりもラムダ式で直接オブジェクト変数をキャプチャしたほうが早いし、安全!
    std::function<bool(double*, size_t)> dft_bind = [&](auto data, auto n) { return fourier.dft(data, n); }; 
    invoke_tm_chrono(dft_bind, data, N);
    auto fourier_coef = fourier.fourier_coef();
    std::for_each(std::begin(fourier_coef), std::begin(fourier_coef) + 8, [] (auto value) {
        std::cout << "Amp: " << std::abs(value) << ", Angle: " << std::arg(value) << std::endl;
    });

    plt::xlim(0, N);
    plt::plot(k, fourier.amplifiers());
    plt::show();

    // FFT
    std::cout << "-----FFT-----" << std::endl;
    // std::bindを使うよりもラムダ式で直接オブジェクト変数をキャプチャしたほうが早いし、安全!
    std::function<bool(double*, size_t)> fft_bind = [&](auto data, auto n) { return fourier.fft(data, n); };
    invoke_tm_chrono(fft_bind, data, N);
    fourier_coef = fourier.fourier_coef();
    std::for_each(std::begin(fourier_coef), std::begin(fourier_coef) + 8, [] (auto value) {
        std::cout << "Amp: " << std::abs(value) << ", Angle: " << std::arg(value) << std::endl;
    });

    plt::xlim(0, N);
    plt::plot(k, fourier.amplifiers());
    plt::show();

    
}
