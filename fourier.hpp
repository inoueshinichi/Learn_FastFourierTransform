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
     * W_k = exp{-j*2*pi*k/N} = cos{2*pi*k/N} + j*sin{2*pi*k/N}
     * @note kは基本角周波数のk倍(kの個数はNに一致する)
     * @note Nはサンプリング数
     */
    using Rotor = std::complex<double>;
    std::vector<Rotor> rotors_; // 1次元

    /**
     * @brief 複素フーリエ係数
     * @note If dft, X_k = Σ_n{W_k,n * x(n)} (n=0,...N-1)
     * @note N倍されているので、振幅・位相として使用するときは1/N倍すること.
     */
    using FourierCoef = std::complex<double>;
    std::vector<FourierCoef> fouriers_;

    /**
     * @brief データ領域
     * @note 元データにゼロ埋めパディングを施したもの
     */
    std::vector<double> data_;

    /**
     * @brief データサイズ
     * @note アルゴリズムによっては、元データサイズと異なる.(e.g 2のべき乗にする)
     */
    size_t size_;

public:
    Fourier(size_t size)
    {
        // 1. フーリエ変換に必要なデータサイズを計算
        size_ = FftPolicy::calc_size(size);

        // 2. 回転子W_k,nを作成
        rotors_ = FftPolicy::calc_rotors(size_);
    }

    virtual ~Fourier() {};
    Fourier(const Fourier&) = default;
    Fourier& operator=(const Fourier&) = default;
    Fourier(Fourier&&) = default;
    Fourier& operator=(Fourier&&) = default;

    std::vector<std::complex<double>> rotors() const
    {
        return rotors_;
    }

    size_t size() const
    {
        return size_;
    }

    std::vector<double> zero_padding_data() const
    {
        return data_;
    }

    std::vector<FourierCoef> fourier_coef() const
    {
        return fouriers_;
    }

    bool dft(double* data, size_t size)
    {
        /*むちゃくちゃ遅いので、検証用に使うこと*/
        if (size > size_)
            return false;

        data_.resize(size_);
        std::fill(std::begin(data_), std::end(data_), (double)0); // 0ゼロ埋め
        for (size_t i = 0; i < size_; ++i) { data_[i] = data[i]; }
        fouriers_.resize(size_);

        // 回転子行列W_k,nを作成
        std::vector<Rotor> mtx_rotors(size_ * size_);
        double w_cos = 0.0;
        double w_sin = 0.0;
        double base_freq = 2 * 3.141592653589793 / size_; //std::numbers::pi / size_;
        for (size_t k = 0; k < size_; ++k)
        {
            for (size_t n = 0; n < size_; ++n)
            {
                w_cos = std::cos(base_freq * k * n);
                w_sin = std::sin(base_freq * k * n);
                mtx_rotors[k * size_ + n] = std::complex(w_cos, w_sin); 
            }
        }

        // 愚直に行列を2重for文で計算: O(N^2)
        for (size_t k = 0; k < size_; ++k)
        {
            for (size_t n = 0; n < size_; ++n)
            {
                // N倍されて出力される
                fouriers_[k] += mtx_rotors[k * size_ + n] * data_.at(n);
            }
        }

        // 1/N化する
        std::for_each(std::begin(fouriers_), std::end(fouriers_), [&](auto& value) {
            value = value * std::complex<double>(1.0/size_, 0.0);
        });
                
        return true;
    }

    bool idft(double* data, size_t size)
    {
        /*未実装*/
        return false;
    }

    template <class T>
    bool fft(const T* data, size_t size)
    {
        if (size > size_)
            return false;

        // ゼロ埋めデータの作成
        data_.resize(size_);
        std::fill(std::begin(data_), std::end(data_), (double)0); // ゼロ埋め
        for (size_t i = 0; i < size_; ++i) { data_[i] = data[i]; } // 端数はゼロ埋めされてる

        // 複素フーリエ係数の準備
        fouriers_.resize(data_.size()); // N倍されて出力される
        for (size_t i = 0; i < fouriers_.size(); ++i)
        {
            fouriers_[i] = std::complex<double>(data_[i], 0.0); // 虚部なしの複素数
            // std::printf("fouriers[%zu]: R:%f, I%f\n", i, fouriers[i].real(), fouriers[i].imag());
        }

        // ポリシーが受け持つ独自アルゴリズムに任せる
        FftPolicy::fft(fouriers_, rotors_);

        return true;
    }

    template <class T>
    bool ifft(T* data, size_t size)
    {
        return false;
    }

    std::vector<double> amplifiers()
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
                       [&](const auto& value) -> double {
                           return std::abs(value);
        });

        return amplifiers;
    }

    std::vector<double> angles()
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

    std::vector<double> power_spectrums()
    {
        /**
         * @brief パワースペクトル
         * ||z|| = |z| * |z|
         */
        auto power_spectrums = amplifiers();
        std::for_each(std::begin(power_spectrums), std::end(power_spectrums),
        [](auto& value) {
            value *= value; // 2乗
        });

        return power_spectrums;
    }

};



struct CutCirclePrams
{
    int radius;
    int cx;
    int cy;
};

struct CutRectParams
{
    int x;
    int y;
    int width;
    int height;
};


template <class FftPolicy>
class Fourier2D
{
    /**
     * @brief 回転子
     * 複素数: a+j*b
     * W_k = exp{-j*2*pi*k/N} = cos{2*pi*k/N} + j*sin{2*pi*k/N}
     * @note kは基本角周波数のk倍(kの個数はNに一致する)
     * @note Nはサンプリング数
     */
    using Rotor = std::complex<double>;
    std::vector<Rotor> rotors_width_;
    std::vector<Rotor> rotors_height_;

    /**
     * @brief 複素フーリエ係数
     * @note If dft, X_k = Σ_n{W_k,n * x(n)} (n=0,...N-1)
     * @note N倍されているので、振幅・位相として使用するときは1/N倍すること.
     */
    using FourierCoef = std::complex<double>;
    std::vector<FourierCoef> fouriers_; // 2次元[N][M]

    /**
     * @brief データ領域 2次元[N][M]
     * @note 元データにゼロ埋めパディングを施したもの
     */
    std::vector<double> data_; // 2次元[N][M]

    /**
     * @brief フーリエ変換で使用する画像の縦横サイズ
     * @note ゼロ埋め込みのサイズ
     */
    size_t width_;
    size_t height_;

public:
    Fourier2D(size_t width, size_t height)
    {
        // 1. ２次元フーリエ変換に必要な縦横サイズを計算
        auto[width_, height_] = FftPolicy::calc_2d_size(width, height);
        this->width_ = width_;
        this->height_ = height_;
        // std::printf("width_: %lu, height_: %lu\n", width_, height_);

        // 2. 2次元フーリエ係数に必要なメモリサイズを確保する
        fouriers_.resize(width_ * height_);

        // 3. 回転子の計算
        rotors_width_ = FftPolicy::calc_rotors(width_);
        rotors_height_ = FftPolicy::calc_rotors(height_);
    }

    virtual ~Fourier2D() {};
    Fourier2D(const Fourier2D&) = default;
    Fourier2D& operator=(const Fourier2D&) = default;
    Fourier2D(Fourier2D&&) = default;
    Fourier2D& operator=(Fourier2D&&) = default;

    size_t width() const 
    {
        return width_;
    }

    size_t height() const
    {
        return height_;
    }

    std::vector<Rotor> rotors_width() const
    {
        return rotors_width_;
    }

    std::vector<Rotor> rotors_height() const
    {
        return rotors_height_;
    }

    std::vector<double> zero_padding_data_2d() const
    {
        return data_;
    }

    std::vector<FourierCoef> fourier_coef_2d() const
    {
        return fouriers_;
    }

    template <class T>
    bool fft2d(const T* data, size_t width, size_t height)
    {
        if (width > width_ || height > height_)
            return false;
        std::printf("width_: %lu, height_: %lu\n", width_, height_);
        
        // ゼロ埋めデータ作成
        data_.resize(width_ * height_);
        std::fill(std::begin(data_), std::end(data_), (double)0);
        size_t i = 0;
        const T* j = data;
        auto k = std::begin(data_);
        for (;
             i < height; 
             ++i, j += width, k += width_)
        {
            // 元画像を埋め込む(範囲外はゼロ埋めされてる)
            std::copy(j, j + width, k);
        }

        // 複素フーリエ係数の準備
        fouriers_.resize(data_.size()); // N倍されて出力される
        for (size_t i = 0; i < fouriers_.size(); ++i)
        {
            fouriers_[i] = std::complex<double>(data_[i], 0.0); // 虚部なしの複素数
            // std::printf("fouriers[%zu]: R:%f, I%f\n", i, fouriers[i].real(), fouriers[i].imag());
        }

        // ポリシーが受け持つ独自アルゴリズムに任せる
        FftPolicy::fft2d(fouriers_, 
                         rotors_width_, 
                         rotors_height_);
        return true;
    }

    template <class T>
    bool ifft2d(T* data, size_t width, size_t height)
    {
        return false;
    }

    bool shift_fft2d(bool is_backward = false)
    {
        /*画像の上下少なくとも一方が奇数の場合でも対応可能な上下左右交換処理*/
        int half_width = (int)(std::ceil(width_ / 2));
        int half_height = (int)(std::ceil(height_ / 2));
        int half_x = (int)(width_ / 2);
        int half_y = (int)(height_ / 2);

        if (is_backward)
        {
            half_width = width_ - half_width;
            half_height = height_ - half_height;
            half_x = width_ - half_x;
            half_y = height_ - half_y;
        }

        if (half_width != half_x)
            std::printf("画像の幅が奇数: %lu\n", width_);
        
        if (half_height != half_y)
            std::printf("画像の高さが奇数: %lu\n", height_);

        int outside_half_width = width_ - half_width;
        int outside_half_height = height_ - half_height;
        
        if (fouriers_.empty())
            return false;
        
        // 第一象限, 第二象限, 第三象限, 第四象限
        std::vector<FourierCoef> left_top(half_width * half_height);
        std::vector<FourierCoef> right_top(outside_half_width * half_height);
        std::vector<FourierCoef> left_bottom(half_width * outside_half_height);
        std::vector<FourierCoef> right_bottom(outside_half_width * outside_half_height);

        /*抽出*/
        // 左上
        for (int y = 0, fy = 0; y < half_height; ++y, ++fy)
            for (int x = 0, fx = 0; x < half_width; ++x, ++fx)
                left_top[y * half_width + x] = fouriers_[fy * width_ + fx];
        // 右上
        for (int y = 0, fy = 0; y < half_height; ++y, ++fy)
            for (int x = 0, fx = half_width; x < outside_half_width; ++x, ++fx)
                right_top[y * outside_half_width + x] = fouriers_[fy * width_ + fx];
        // 左下
        for (int y = 0, fy = half_height; y < outside_half_height; ++y, ++fy)
            for (int x = 0, fx = 0; x < half_width; ++x, ++fx)
                left_bottom[y * half_width + x] = fouriers_[fy * width_ + fx];
        // 右下
        for (int y = 0, fy = half_height; y < outside_half_height; ++y, ++fy)
            for (int x = 0, fx = half_width; x < outside_half_width; ++x, ++fx)
                right_bottom[y * outside_half_width + x] = fouriers_[fy * width_ + fx];
        
        /*埋め込み*/
        // 左上を右下に埋める
        for (int y = 0, fy = half_y; y < half_height; ++y, ++fy)
            for (int x = 0, fx = half_x; x < half_width; ++x, ++fx)
                fouriers_[fy * width_ + fx] = left_top[y * half_width + x];
        // 右下を左上に埋める
        for (int y = 0, fy = 0; y < outside_half_height; ++y, ++fy)
            for (int x = 0, fx = 0; x < outside_half_width; ++x, ++fx)
                fouriers_[fy * width_ + fx] = right_bottom[y * outside_half_width + x];
        // 右上を左下に埋める
        for (int y = 0, fy = half_y; y < half_height; ++y, ++fy)
            for (int x = 0, fx = 0; x < outside_half_width; ++x, ++fx)
                fouriers_[fy * width_ + fx] = right_top[y * outside_half_width + x];
        // 左下を右上に埋める
        for (int y = 0, fy = 0; y < outside_half_height; ++y, ++fy)
            for (int x = 0, fx = half_x; x < half_width; ++x, ++fx)
                fouriers_[fy * width_ + fx] = left_bottom[y * half_width + x];

        return true;
    }

    std::vector<double> amplifiers_2d()
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
                       [&](const auto& value) -> double {
                           return std::abs(value);
        });

        return amplifiers;
    }

    std::vector<double> angles_2d()
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

    std::vector<double> power_spectrums_2d()
    {
        /**
         * @brief パワースペクトル
         * ||z|| = |z| * |z|
         */
        auto power_spectrums = amplifiers_2d();
        std::for_each(std::begin(power_spectrums), std::end(power_spectrums),
        [](auto& value) {
            value *= value; // 2乗
        });

        return power_spectrums;
    }


    void cut_circle_fft(std::vector<CutCirclePrams> params, bool is_inner_cut = true)
    {
        /*円形領域のカットによるフィルタ*/
        int radius_2 = 0;
        int cx = 0;
        int cy = 0;
        double rr = 0.0;
        if (is_inner_cut)
        {
            // for (int y = 0; y < height_; ++y)
            // {
            //     for (int x = 0; x < width_; ++x)
            //     {
            //         bool is_cut = false;
            //         for (auto iter = std::begin(params); iter != std:end(params); ++iter)
            //         {
            //             radius_2 = (*iter).radius * (*iter).radius;
            //             cx = (*iter).cx;
            //             cy = (*iter).cy;
            //             rr = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            //         }
            //     }
            // }
        }
        else
        {

        }
    }
};


