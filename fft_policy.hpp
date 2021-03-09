#pragma once

#include <complex>
#include <vector>
#include <algorithm>
// #include <numbers>
#include <numeric>
#include <stdexcept>

class CooleyTurkeyFft
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
            for (size_t target = index, n = 0; n < n_level; bit <<= 1, target >>= 1)
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
    CooleyTurkeyFft() {};
    ~CooleyTurkeyFft() {};

    static size_t calc_size(size_t size)
    {
        size_t exp_size = 1;
        while (size > exp_size) {
            exp_size *= 2;
        }
        return exp_size;
    }

    static std::vector<std::complex<double>> 
    fft(const std::vector<double>& data, const std::vector<std::complex<double>>& rotors)
    {
        // ビットリバースを行ったインデックスマップを作成
        std::vector<size_t> indice_map_array(data.size());
        int n_level = indice_map_with_bit_reverse(indice_map_array);
    
        /**
         * @brief バタフライダイアグラム
         * pt1. i(<=n_level)の数が増えるごとにバタフライ演算回数が2倍になる.
         * 
         * 
         */
        int half_size = data.size();
        int butterfly_num = 1;
        for (int i = 1; i <= n_level; ++i)
        {
            half_size /= 2;
            for (int j = 0; j < butterfly_num; ++j)
            {
                for (int k = 0; k < half_size; ++k)
                {

                }
            }
        }
    }
};