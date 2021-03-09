#include "fourier.hpp"

#include <iostream>
#include <vector>

int main(int, char**) 
{
    std::cout << "Hello, Fourier!\n";

    // ダミーデータ(sin)
    const int N = 1000;
    double basic_freq = 2 * 3.141659 / N;
    std::vector<double> sine(N);
    for (int i = 0; i < 1000; ++i)
    {
        sine[i] = std::sin(basic_freq * i);
    }

    Fourier<fft::CooleyTurkey> fourier(N);
    auto rotors = fourier.rotors();
    std::for_each(std::begin(rotors), std::end(rotors), [] (auto value) {
        std::cout << "rotor: " << value << std::endl;
    });

    std::cout << "data_size: " << fourier.size() << std::endl;

    // DFT
    double* data = &(*std::begin(sine));
    std::cout << "-----DFT-----" << std::endl;
    fourier.dft(data, N);
    auto fourier_coef = fourier.fourier_coef();
    std::for_each(std::begin(fourier_coef), std::begin(fourier_coef) + 5, [] (auto value) {
        std::cout << "Amp: " << std::abs(value) << ", Angle: " << std::arg(value) << std::endl;
    });

    // FFT
    std::cout << "-----FFT-----" << std::endl;
    fourier.fft(data, N);
    fourier_coef = fourier.fourier_coef();
    std::for_each(std::begin(fourier_coef), std::begin(fourier_coef) + 5, [] (auto value) {
        std::cout << "Amp: " << std::abs(value) << ", Angle: " << std::arg(value) << std::endl;
    });
}
