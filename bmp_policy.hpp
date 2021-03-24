#ifndef IS_IMGPROC_BMP_POLICY_HPP
#define IS_IMGPROC_BMP_POLICY_HPP

#include <cstdint>
#include <string>

namespace Is
{
    namespace imgproc
    {
        namespace format_policy
        {
            using std::string;
            using byte = unsigned char;

            // Windows Bitmapファイルのフォーマットについて
            // https://www.mm2d.net/main/prog/c/image_io-05.html
            // https://github.com/ohmae/image-io
            class BmpFilePolicy
            {
                /*BMPファイルヘッダ (14byte)*/
                #pragma pack(2) // 構造体のアライメントを2byte境界にすることで、余計な詰め物がない構造体とする. ※重要
                typedef struct BmpFileHeader {
                    uint16_t bf_type;        // ファイルタイプ. 必ず"BM" B=0x42、M=0x4D
                    uint32_t bf_size;        // ファイルサイズ
                    uint16_t bf_reserved1;   // 予約1
                    uint16_t bf_reserved2;   // 予約2
                    uint32_t bf_offset_bits; // ファイル先頭から画像情報までのオフセット
                }BmpFileHeader;
                #pragma pack() // 構造体のアライメントをデフォルトの4byte境界に戻す. ※重要

                /*BMP情報ヘッダ*/
                typedef struct BmpInfoHeader {
                    uint32_t bi_size;             // この構造体のサイズ(40byte)
                    int32_t  bi_width;            // 画像の幅. 負の値は不正な値
                    int32_t  bi_height;           // 画像の高さ. 値が負の場合はトップダウン画像となる. ただし、この方式は互換性の観点から非推奨とされる.
                    uint16_t bi_planes;           // 画像の枚数. 常に1
                    uint16_t bi_bitcount;         // 色深度. 0, 1, 4, 8, 16, 24, 32
                    uint32_t bi_compression;      // 圧縮形式. 0, 1, 2, 3, 4, 5
                    uint32_t bi_size_image;       // 画像領域のサイズ
                    int32_t  bi_x_pels_per_meter; // 画像の横方向解像度情報. 解像度を扱わない場合は0でも問題ない
                    int32_t  bi_y_pels_per_meter; // 画像の縦方向解像度情報. 解像度を扱わない場合は0でも問題ない
                    uint32_t bi_color_used;       // カラーパレットのうち、実際に使用している色の個数
                    uint32_t bi_clor_important;   // カラーパレットのうち、重要な色の数
                }BmpInfoHeader;

                /*BMPカラーパレット*/
                typedef struct RgbQuad {
                   byte rgb_blue;
                   byte rgb_green;
                   byte rgb_red;
                   byte rgb_reserved;
                }RgbQuad;

                /*BMP情報*/
                typedef struct BmiInfo {
                    BmpInfoHeader bmi_header;
                    RgbQuad bmi_colors[1];
                }BmiInfo;

                BmpFileHeader bmp_file_header_;
                BmiInfo*      bmi_info_;
                byte*         bmp_;
                int32_t       width_;
                int32_t       height_;
                size_t        mem_width_;
                int32_t       channels_;
                size_t        data_size_;

                void clear();
                void setup(int32_t width, int32_t height, int32_t channels);
            public:
                BmpFilePolicy();
                ~BmpFilePolicy();
                BmpFilePolicy(const BmpFilePolicy&) = delete;
                BmpFilePolicy& operator=(const BmpFilePolicy&) = delete;
                BmpFilePolicy(BmpFilePolicy&&) = default;
                BmpFilePolicy& operator=(BmpFilePolicy&&) = default;

                void set_data(byte* data, int insert_color = -1);
                void get_data(byte* data, int extract_color = -1);
                void save(const string& filename, byte* data, int32_t width, int32_t height, int32_t channels, bool is_dump);
                void load(const string& filename, int32_t& width, int32_t& height, int32_t& channels, bool is_dump);
                void dump() const;
            };
        } // namespace format_policy
    } // namespace imgproc
}
#endif