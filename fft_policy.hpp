#pragma once

#include <complex>
#include <vector>
#include <tuple>
#include <algorithm>
// #include <numbers>
#include <numeric>
#include <stdexcept>
#include <iostream>


namespace fft
{
    class CooleyTurkey
    {
        /**
         * @brief ビットリバースによる参照インデックスマップを取得
         * 
         * @param size 
         * @param indices ビットリバースで作成したインデックスマップの配列(入出力同じ)
         * @return int べき乗レベル
         */
        static int indice_map_with_bit_reverse(std::vector<size_t>& indices)
        {
            /*2のべき乗で表される数値を表現するために最低限必要なビット数を取得*/
            size_t size = indices.size();
            size_t size_by_bit = 8 * sizeof(size); // sizeを表現するbit数
            size_t n_level;
            for (n_level = 0; n_level < size_by_bit; ++n_level)
            {
                if (size >> n_level == 1)
                    break;
            }
            // std::printf("n_level: %zu\n", n_level);

            // 参照インデックスマップを計算
            for (size_t index = 0; index < size; ++index)
            {
                size_t bit = 0;
                for (size_t target = index, n = 0; ; bit <<= 1, target >>= 1)
                { // N=8のとき8を表現できるビット数は, n_level = 3, シフト量は, 3 - 1 = 2となる.

                    // targetの下位1bitを抽出して、前回抽出して1bit左シフトしたものとORをとる.
                    // これをn_level回行えば、ビットリバースが完成.
                    bit = (target & 1) | bit; 
                    if (++n == n_level) 
                        break;
                }
                // 格納
                indices[index] = bit;
            }
            // std::printf("check...\n");

            return n_level;
        }


    public:
        using FourierVector = std::vector<std::complex<double>>;
        using RotorVector = std::vector<std::complex<double>>;

        CooleyTurkey() {};
        ~CooleyTurkey() {};

        static RotorVector calc_rotors(size_t size)
        {
            /*回転子W_kを作成*/
            double w_cos = 0.0;
            double w_sin = 0.0;
            double base_freq = 2 * 3.141592653589793 / size; 
            //std::numbers::pi / size;
            RotorVector rotors(size);
            for (size_t k = 0; k < size; ++k)
            {
                w_cos = std::cos(base_freq * k);
                w_sin = std::sin(base_freq * k);
                rotors[k] = std::complex(w_cos, w_sin);
            }
            return rotors;
        }

        static size_t calc_size(size_t size)
        {
            size_t exp_size = 1;
            while (size > exp_size) {
                exp_size *= 2;
            }
            return exp_size;
        }

        static std::tuple<size_t, size_t>
        calc_2d_size(size_t width, size_t height)
        {
            size_t length = width > height ? width : height;
            size_t exp_size = calc_size(length);
            return std::make_tuple(exp_size, exp_size);
        }

        static void
        fft(FourierVector& fouriers, const RotorVector& rotors)
        {
            /*周波数間引き型のFFT*/
            // https://qiita.com/tommyecguitar/items/c7f1049b308411dbd6d3

            // ビットリバースを行ったインデックスマップを作成
            std::vector<size_t> indice_map(fouriers.size());
            int n_level = indice_map_with_bit_reverse(indice_map);
            // std::printf("n_level: %d\n", n_level);

            // n_levelは以下の計算でも求めることができる.
            // int ex = (int)(std::log((double)fouriers.size()) / log(2.0));
            // std::printf("ex: %d\n", ex);
            
            /**
             * @brief バタフライ演算
             */
            int half_size = fouriers.size();
            int butterfly_num = 1;
            int butterfly_offset, j1, j2, idx_w;
            for (int i = 0; i < n_level; ++i) // 統治分割のレベル
            {
                half_size /= 2; // half_sizeは, N/2, N/4, ...
                butterfly_offset = 0; // 統治分割されたバタフライダイアグラムの先頭インデックス
                for (int j = 0; j < butterfly_num; ++j)
                {
                    idx_w = 0;
                    for (int k = 0; k < half_size; ++k)
                    {
                        j1 = butterfly_offset + k;
                        j2 = j1 + half_size;
                        auto f1 = fouriers[j1];
                        auto f2 = fouriers[j2];
                        fouriers[j1] = f1 + f2; // 複素数での演算
                        fouriers[j2] = rotors[idx_w] * (f1 - f2); // 複素数での演算
                        idx_w += butterfly_num; // 1, 2, 4, 8, ...の倍数で回転子の添字の加算量が増える.
                        // std::printf("i=%d, j=%d, k=%d\n", i, j, k);
                    }
                    // butterfly_offset
                    // i==1 : N(使わない) 
                    // i==2 : N/2
                    // i==3 : N/4
                    // i==4 : N/8
                    // ...
                    butterfly_offset += 2 * half_size;
                }
                butterfly_num *= 2;
            }

            /**
             * @brief 分割統治法によるバタフライ演算
             * データ0~N-1を0~(N/2)-1, N/2~N-1に分割し、
             * 1) y(k) = x(k) + x(k+N/2) {k=0...(N/2)-1}
             * 2) y(k) = 
             * 未実装(統治分割法を理解してからかな？)
             */

            // std::printf("FFT: fourier coef\n");
            // for (auto& f : fouriers)
            // {
            //     std::cout << "Amp: " << std::abs(f) << ", Angle: " << std::arg(f) << std::endl;
            // }

            // for (size_t i = 0; i < indice_map.size(); ++i)
            // {
            //     std::printf("indice_map[%zu]=%zu\n", i, indice_map[i]);
            // }

            // バタフライダイアグラムの出力配列の並びを替える(周波数間引き型)
            FourierVector sorted_fouriers(fouriers.size());
            for (size_t i = 0; i < sorted_fouriers.size(); ++i)
            {   
                sorted_fouriers[i] = fouriers[indice_map[i]];
            }

            // std::printf("sorted FFT: fourier coef\n");
            // for (auto& f : fouriers)
            // {
            //     std::cout << "Amp: " << std::abs(f) << ", Angle: " << std::arg(f) << std::endl;
            // }


            // 複素フーリエ係数はN倍化されたままなので、1/Nする
            size_t size = sorted_fouriers.size();
            std::complex<double> norm(1.0/size, 0.0);
            std::for_each(std::begin(sorted_fouriers), std::end(sorted_fouriers), 
            [&norm](auto& value) {
                value = value * norm; // 実部、虚部をsizeで割る.
            });

            // 元の引数に演算結果を返す
            fouriers = sorted_fouriers;
        }

        static void
        fft2d(FourierVector& fouriers,
              const RotorVector& rotors_width,
              const RotorVector& rotors_height)
        {
            /**
             * @brief ToDo
             * 1. 画像の行ごとにフーリエ変換
             * 2. 複素フーリエ係数行列を転値
             * 3. 画像の行(列)ごとにフーリエ変換
             * 4. 2と同じことを行う
             * 5. 完了
             */
            size_t width = rotors_width.size();
            size_t height = rotors_height.size();

            std::cout << "FftPolicy::fft2d" << std::endl;

            // 1. 画像の行ごとにフーリエ変換
            size_t i = 0;
            auto j = std::begin(fouriers);
            for (;
                 i < height;
                 ++i, j += width)
            {
                FourierVector fourier_row(j, j + width);
                fft(fourier_row, rotors_width);
                std::copy(std::begin(fourier_row), std::end(fourier_row), j);
            }

            // 2. 複素フーリエ係数行列を転値
            FourierVector f_transpose(width * height);
            for (size_t j = 0; j < height; ++j)
            {
                for (size_t i = 0; i < width; ++i)
                {
                    f_transpose.at(i * height + j) = fouriers.at(j * width + i);
                }
            }

            // 3. 画像の行(列)ごとにフーリエ変換
            i = 0;
            j = std::begin(f_transpose);
            for (;
                 i < width;
                 ++i, j += height)
            {
                FourierVector fourier_row(j, j + height);
                fft(fourier_row, rotors_height);
                std::copy(std::begin(fourier_row), std::end(fourier_row), j);
            }

            // 4. 2と同じことを行う
            for (size_t j = 0; j < height; ++j)
            {
                for (size_t i = 0; i < width; ++i)
                {
                    fouriers.at(j * width + i) = f_transpose.at(i * height + j);
                }
            }
        }
    };
}