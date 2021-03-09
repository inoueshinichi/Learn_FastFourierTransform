#pragma once

#include "fft_policy.hpp"

#include <complex>
#include <vector>
#include <algorithm>
// #include <numbers>


template <class FftPolicy>
class Fourier 
{
    /**
     * @brief 回転子
     * 複素数: a+j*b
     * W_k,n = exp{-j*2*pi*k*n/N} = cos{2*pi*k*n/N} + j*sin{2*pi*k*n/N}
     * @note kは基本角周波数のk倍
     * @note nはサンプリング添字
     * @note Nはサンプリング数
     */
    using Rotor = std::complex<double>;
    std::vector<Rotor> rotors_; // 2次元[N][N]

    /**
     * @brief 複素フーリエ係数
     * @note If dft, X_k = Σ_n{W_k,n * x(n)} (n=0,...N-1)
     */
    using FourierCoef = std::complex<double>;
    std::vector<FourierCoef> fouriers_;

    /**
     * @brief データ領域
     * @note アルゴリズムによっては、元データサイズと異なる.(e.g 2のべき乗にする)
     */
    std::vector<double> data_;

    /**
     * @brief データサイズ
     * @note アルゴリズムによっては、元データサイズと異なる.(e.g 2のべき乗にする)
     */
    size_t size_;

    // 回転子W_k,nを作成
    void calc_rotors()
    {
        double w_cos = 0.0;
        double w_sin = 0.0;
        double base_freq = 3.141592653589793; //std::numbers::pi / size_;
        rotors_.resize(size_ * size_);
        for (size_t k = 0; k < size_; ++k)
        {
            for (size_t n = 0; n < size_; ++n)
            {
                w_cos = std::cos(base_freq * k * n);
                w_sin = std::sin(base_freq * k * n);
                rotors_[k * size_ + n] = std::complex(w_cos, w_sin); 
            }
        }
    }

public:
    Fourier(size_t size)
    {
        // 1. フーリエ変換に必要なデータサイズを計算
        size_ = FftPolicy::calc_size(size);

        // 2. 回転子W_k,nを作成
        calc_rotors();
    }

    virtual ~Fourier() {};
    Fourier(const Fourier&) = default;
    Fourier& operator=(const Fourier&) = default;
    Fourier(Fourier&&) = default;
    Fourier& operator=(Fourier&&) = default;

    bool dft(double* data, size_t size)
    {
        if (size > size_)
            return false;

        data_.resize(size_);
        std::fill(std::begin(data_), std::end(data_), (double)0); // 0ゼロ埋め
        std::copy(std::begin(data), std::end(data), std::begin(data_));
        fouriers_.resize(size_);

        // 愚直に行列を2重for文で計算: O(N^2)
        for (size_t k = 0; k < size_; ++k)
        {
            for (size_t n = 0; n < size_; ++n)
            {
                fouriers_[k] += rotors_[k * size_ + n] * data_.at(n);
            }
        }
        return true;
    }

    template <class T>
    bool fft(T* data, size_t size)
    {
        if (size > size_)
            return false;

        data_.resize(size_);
        std::fill(std::begin(data_), std::end(data_), (double)0); // 0ゼロ埋め
        std::copy(std::begin(data), std::end(data), std::begin(data_));

        // ポリシーが受け持つ独自アルゴリズムに任せる
        fouriers_ = FftPolicy::fft(data_, rotors_);
        return true;
    }

    std::vector<double> get_power_spectrums()
    {
        /**
         * @brief パワースペクトル
         * ||z|| = |z| * |z|
         */
        auto power_spectrums = get_amplifiers();
        std::for_each(std::begin(power_spectrums), std::end(power_spectrums),
        [](auto& value) {
            value *= value; // 2乗
        });

        return power_spectrums;
    }

    std::vector<double> get_amplifiers()
    {
        /**
         * @brief 振幅
         * z = sqrt(a*a+b*b)
         * z = std::abs(std::complex<double>)
         */
        std::vector<double> amplifiers(fouriers_.size());
        std::transform(std::begin(fouriers_), 
                       std::end(fouriers_),
                       std::begin(amplifiers), 
                       [&size_](const auto& value) -> double {
                           return std::abs(value) / size_; // 振幅はN倍化されているので、戻す
        });

        return amplifiers;
    }

    std::vector<double> get_angles()
    {
        /**
         * @brief 位相(偏角)
         * theta = tan^-1(b/a)
         * theta = atan2(a,b)
         */
        std::vector<double> angles(fouriers_.size());
        std::transform(std::begin(fouriers_), 
                       std::end(fouriers_),
                       std::begin(angles), 
                       [](const auto& value) -> double {
                           return std::arg(value); // 偏角
        });

        return angles;
    }
};