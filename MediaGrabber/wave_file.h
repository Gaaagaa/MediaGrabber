/**
 * @file   wave_file.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 *
 * 文件名称：wave_file.h
 * 创建日期：2019年07月23日
 * 文件标识：
 * 文件摘要：实现 WAVE 文件相关操作的工具类。
 *
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年07月23日
 * 版本摘要：
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __WAVE_FILE_H__
#define __WAVE_FILE_H__

#include "xtypes.h"

////////////////////////////////////////////////////////////////////////////////

/**
 * @struct x_wave_chunk_header_t
 * @brief  WAV 文件格式的文件头。
 */
typedef struct __wave_chunk_header__
{
   x_char_t     xct_chunk_id[4];     ///< 内容为"RIFF"
   x_uint32_t   xut_chunk_size;      ///< 存储文件的字节数（不包含 xct_chunk_id 和 xut_chunk_size 这8个字节）
   x_char_t     xct_format[4];       ///< 内容为"WAVE"
} x_wave_chunk_header_t;

/**
 * @struct x_wave_chunk_format_t
 * @brief  WAV 文件格式的基本描述信息。
 */
typedef struct __wave_chunk_format__
{
    x_char_t    xct_sub_chunk_1id[4]; ///< 内容为"fmt"
    x_uint32_t  xut_sub_chunk_1size;  ///< 存储该子块的字节数（不含前面的 xct_sub_chunk_1id 和 xut_sub_chunk_1size 这8个字节）
    x_uint16_t  xut_audio_format;     ///< 存储音频文件的编码格式，例如若为PCM则其存储值为1，若为其他非PCM格式的则有一定的压缩。
    x_uint16_t  xut_num_channels;     ///< 通道数，单通道(Mono)值为1，双通道(Stereo)值为2，等等
    x_uint32_t  xut_sample_rate;      ///< 采样率，如 8k, 44.1k 等
    x_uint32_t  xut_byte_rate;        ///< 位速（每秒存储的bit数），其值 = xut_sample_rate * xut_num_channels * xut_bits_per_sample / 8
    x_uint16_t  xut_block_align;      ///< 块对齐大小，其值 = xut_num_channels * xut_bits_per_sample / 8
    x_uint16_t  xut_bits_per_sample;  ///< 每个采样点的bit数，一般为 8, 16, 32 等。
} x_wave_chunk_format_t;

/**
 * @struct x_wave_chunk_data_t
 * @brief  WAV 文件格式的数据块描述信息。
 */
typedef struct __wave_chunk_data__
{
    x_char_t    xct_sub_chunk_2id[4]; ///< 内容为"data"
    x_uint32_t  xut_sub_chunk_2size;  ///< 内容为接下来的正式数据部分的字节数，其值 = [NumSamples] * xut_num_channels * xut_bits_per_sample / 8
} x_wave_chunk_data_t;

#define WAVE_HFD_SIZE (sizeof(x_wave_chunk_header_t) + \
                       sizeof(x_wave_chunk_format_t) + \
                       sizeof(x_wave_chunk_data_t))

////////////////////////////////////////////////////////////////////////////////
// x_wave_file_writer_t

/**
 * @brief 用于 WAVE 文件（PCM）数据写入操作的工具类。
 */
class x_wave_file_writer_t
{
    // common data types
protected:
    typedef x_wave_chunk_format_t x_format_t;

    // constructor/destructor
public:
    explicit x_wave_file_writer_t(void);
    ~x_wave_file_writer_t(void);

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 打开文件。
     *
     * @param [in ] xct_filename        : 数据写入操作的目标文件路径名。
     * @param [in ] xut_num_channels    : 通道数。
     * @param [in ] xut_sample_rate     : 采样率。
     * @param [in ] xut_bits_per_sample : 每个采样点的位数。
     *
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t open(x_cstring_t xct_filename,
                   x_uint16_t xut_num_channels,
                   x_uint32_t xut_sample_rate,
                   x_uint16_t xut_bits_per_sample);

    /**********************************************************/
    /**
     * @brief 关闭文件。
     */
    x_void_t close(void);

    /**********************************************************/
    /**
     * @brief 判断是否文件已经打开。
     */
    inline x_bool_t is_open(void) const
    {
        return (X_NULL != m_xht_file);
    }

    /**********************************************************/
    /**
     * @brief 写入 PCM 数据。
     *
     * @param [in ] xct_dptr : 要写入的 PCM 数据缓存。
     * @param [in ] xut_dlen : 要写入的 PCM 数据长度。
     *
     * @return x_int32_t
     *         - 成功，返回 写入字节数（>= 0）；
     *         - 失败，返回 -1。
     */
    x_int32_t write(x_uchar_t * xct_dptr, x_uint32_t xut_dlen);

    // data members
protected:
    x_handle_t  m_xht_file;   ///< 文件句柄
    x_format_t  m_xfmt_info;  ///< 文件格式信息
};

////////////////////////////////////////////////////////////////////////////////

#endif // __WAVE_FILE_H__
