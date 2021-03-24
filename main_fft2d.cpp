/**
 * @file main_fft2d.cpp
 * @author your name (you@domain.com)
 * @brief 二次元FFT
 * @version 0.1
 * @date 2021-03-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "fourier.hpp"

#include "matplotlibcpp.h"

#include "bmp_policy.hpp"

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

int main(int,char**)
{
    // 画像の読み込み
    using namespace Is::imgproc::format_policy;
    BmpFilePolicy bmpfile;
    int width = 0;
    int height = 0;
    int channels = 0;
    std::string filename = "/Users/inoueshinichi/Desktop/LENNA2.bmp";
    bmpfile.load(filename, width, height, channels, true);

    using byte = unsigned char;
    byte* bmp = new byte[width * height * channels];
    bmpfile.get_data(bmp);
    std::cout << "width: " << width << std::endl;
    std::cout << "height: " << height << std::endl;
    std::cout << "channels: " << channels << std::endl;

    // // BMPファイルの表示
    // plt::imshow(bmp, height, width, channels);
    // plt::show();

    // GrayScale
    byte* gray_img = new byte[width * height];
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            gray_img[i * width + j] = bmp[i * (channels * width) + channels * j + 0];
        }
    }

    // BMPファイルの表示
    plt::imshow(gray_img, height, width, 1);
    plt::show();

    // 2次元FFT
    Fourier2D<fft::CooleyTurkey> fourier2d(width, height);

    // bind
    auto bind_fft2d = [&](auto img, auto width, auto height) {
        fourier2d.fft2d(img, width, height);
    };

    //計測
    invoke_tm_chrono(bind_fft2d, gray_img, width, height);

    // 上下左右反転
    fourier2d.shift_fft2d();
    
    std::vector<double> amp = fourier2d.amplifiers_2d();
    std::vector<float> amp_f(amp.size());
    for (int i = 0; i < amp_f.size(); ++i) {
        amp_f.at(i) = (float)20 * std::log2(amp.at(i));
    }
    const float* amp_f_ptr = &(amp_f[0]);
    int amp_f_width = fourier2d.width();
    int amp_f_height = fourier2d.height();
    std::cout << "amp_f_width: " << amp_f_width << std::endl;
    std::cout << "amp_f_height: " << amp_f_height << std::endl;

    plt::imshow(amp_f_ptr, amp_f_width, amp_f_height, 1);
    plt::show();

    std::vector<double> angle = fourier2d.angles_2d();
    std::vector<float> angle_f(angle.size());
    for (int i = 0; i < angle_f.size(); ++i) {
        angle_f.at(i) = (float)angle.at(i);
    }
    const float* angle_f_ptr = &(angle_f[0]);
    plt::imshow(angle_f_ptr, amp_f_width, amp_f_height, 1);
    plt::show();

    delete[] gray_img;
    delete[] bmp;
}