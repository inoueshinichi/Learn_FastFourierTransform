#pragma once

#include <complex>
#include <vector>
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
            // 2のべき乗で表される数値を表現するために最低限必要なビット数を取得
            size_t size = indices.size();
            size_t size_by_bit = 8 * sizeof(size); // sizeを表現するbit数
            size_t n_level;
            size_t one_bit_count = 0; // 1じゃなければ、おかしい
            for (n_level = 0; n_level < size_by_bit; ++n_level)
            {
                if (size >> n_level == 1)
                    one_bit_count += 1;
            }

            if (one_bit_count > 1)
                throw std::runtime_error("Data size is invalid.");

            // 参照インデックスマップを計算
            for (size_t index = 0; index < size; ++index)
            {
                size_t bit = 0;
                for (size_t target = index, n = 0; n < n_level; bit <<= 1, target >>= 1, ++n)
                {
                    // targetの下位1bitを抽出して、前回抽出して1bit左シフトしたものとORをとる.
                    // これをn_level回行えば、ビットリバースが完成.
                    bit = (target & 1) | bit; 
                }
                // 格納
                indices[index] = bit;
            }

            return n_level;
        }


    public:
        using FourierVector = std::vector<std::complex<double>>;
        using RotorVector = std::vector<std::complex<double>>;

        CooleyTurkey() {};
        ~CooleyTurkey() {};

        static size_t calc_size(size_t size)
        {
            size_t exp_size = 1;
            while (size > exp_size) {
                exp_size *= 2;
            }
            return exp_size;
        }

        static FourierVector 
        fft(const std::vector<double>& data, const RotorVector& rotors)
        {
            /*周波数間引き型のFFT*/

            // 複素フーリエ係数の準備
            FourierVector fouriers(data.size()); // N倍されて出力される
            std::copy(std::begin(data), std::end(data), std::begin(fouriers)); // 虚部なしの複素数
            

            // ビットリバースを行ったインデックスマップを作成
            std::vector<size_t> indice_map(data.size());
            int n_level = indice_map_with_bit_reverse(indice_map);
            
            /**
             * @brief バタフライ演算
             */
            int half_size = data.size();
            int butterfly_num = 1;
            int butterfly_offset, j1, j2, idx_w;
            for (int i = 1; i <= n_level; ++i) // 統治分割のレベル
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
                        fouriers[j1] = fouriers[j1] + fouriers[j2]; // 複素数での演算
                        fouriers[j2] = rotors[idx_w] * (fouriers[j1] - fouriers[j2]); // 複素数での演算
                        idx_w += butterfly_num; // 1, 2, 4, 8, ...の倍数で回転子の添字の加算量が増える.
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


            // バタフライダイアグラムの出力配列の並びを替える(周波数間引き型)
            for (size_t i = 0; i < indice_map.size(); ++i)
            {
                std::complex<double> tmp = fouriers[i];
                fouriers[i] = fouriers[indice_map[i]];
                fouriers[indice_map[i]] = tmp;
            }

            // 複素フーリエ係数はN倍化されたままなので、1/Nする
            size_t size = fouriers.size();
            std::complex<double> norm(1.0/size, 0.0);
            std::for_each(std::begin(fouriers), std::end(fouriers), 
            [&norm](auto& value) {
                value = value / norm; // 実部、虚部をsizeで割る.
            });

            return fouriers;
        }
    };
}