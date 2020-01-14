/**
 * @file   wave_file.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 *
 * 文件名称：wave_file.cpp
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

#include "wave_file.h"

#include <stdio.h>
#include <memory.h>
#include <assert.h>

////////////////////////////////////////////////////////////////////////////////
// x_wave_file_writer_t

//====================================================================

//
// x_wave_file_writer_t : constructor/destructor
//

x_wave_file_writer_t::x_wave_file_writer_t(void)
    : m_xht_file(X_NULL)
{
    memset(&m_xfmt_info, 0, sizeof(x_format_t));
}

x_wave_file_writer_t::~x_wave_file_writer_t(void)
{
    if (is_open())
    {
        close();
    }
}

//====================================================================

//
// public interfaces
//

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
x_int32_t x_wave_file_writer_t::open(x_cstring_t xct_filename,
                                     x_uint16_t xut_num_channels,
                                     x_uint32_t xut_sample_rate,
                                     x_uint16_t xut_bits_per_sample)
{
    x_int32_t xit_error = -1;

    do
    {
        if (is_open())
        {
            close();
        }

        FILE * xfile = fopen(xct_filename, "wb+");
        if (X_NULL == xfile)
        {
            xit_error = -1;
            break;
        }

        //======================================

        x_uchar_t xct_buffer[WAVE_HFD_SIZE] = { 0 };

        x_int32_t xit_offset = 0;
        x_wave_chunk_header_t * xheader = (x_wave_chunk_header_t *)(xct_buffer + xit_offset);
        xheader->xct_chunk_id[0] = 'R';
        xheader->xct_chunk_id[1] = 'I';
        xheader->xct_chunk_id[2] = 'F';
        xheader->xct_chunk_id[3] = 'F';
        xheader->xut_chunk_size  =  0 ;
        xheader->xct_format[0]   = 'W';
        xheader->xct_format[1]   = 'A';
        xheader->xct_format[2]   = 'V';
        xheader->xct_format[3]   = 'E';

        xit_offset += sizeof(x_wave_chunk_header_t);
        x_wave_chunk_format_t * xformat = (x_wave_chunk_format_t *)(xct_buffer + xit_offset);
        xformat->xct_sub_chunk_1id[0] = 'f';
        xformat->xct_sub_chunk_1id[1] = 'm';
        xformat->xct_sub_chunk_1id[2] = 't';
        xformat->xct_sub_chunk_1id[3] = ' ';
        xformat->xut_sub_chunk_1size  = sizeof(x_wave_chunk_format_t) - 8;
        xformat->xut_audio_format     = 1;
        xformat->xut_num_channels     = xut_num_channels;
        xformat->xut_sample_rate      = xut_sample_rate;
        xformat->xut_byte_rate        = xut_sample_rate * xut_num_channels * xut_bits_per_sample / 8;
        xformat->xut_block_align      = xut_num_channels * xut_bits_per_sample / 8;
        xformat->xut_bits_per_sample  = xut_bits_per_sample;

        xit_offset += sizeof(x_wave_chunk_format_t);
        x_wave_chunk_data_t * xdata = (x_wave_chunk_data_t *)(xct_buffer + xit_offset);
        xdata->xct_sub_chunk_2id[0] = 'd';
        xdata->xct_sub_chunk_2id[1] = 'a';
        xdata->xct_sub_chunk_2id[2] = 't';
        xdata->xct_sub_chunk_2id[3] = 'a';
        xdata->xut_sub_chunk_2size  =  0 ;

        //======================================

        x_size_t xst_wlen = fwrite(xct_buffer, WAVE_HFD_SIZE, 1, xfile);
        assert(WAVE_HFD_SIZE != xst_wlen);

        //======================================

        m_xht_file = (x_handle_t)xfile;

        memcpy(&m_xfmt_info,
               xct_buffer + sizeof(x_wave_chunk_header_t),
               sizeof(x_wave_chunk_format_t));

        //======================================
    } while (0);

    return xit_error;
}

/**********************************************************/
/**
 * @brief 关闭文件。
 */
x_void_t x_wave_file_writer_t::close(void)
{
    if (X_NULL != m_xht_file)
    {
        FILE * xfile = (FILE *)m_xht_file;
        fflush(xfile);
#if 1
        fseek(xfile, 0, SEEK_END);
        x_uint32_t xut_size = (x_uint32_t)ftell(xfile);

        if (xut_size > WAVE_HFD_SIZE)
        {
            // 修正 x_wave_chunk_header_t.xut_chunk_size 值
            // 即：存储文件的字节数
            x_uint32_t xut_file_size = xut_size - 8;
            fseek(xfile, 4, SEEK_SET);
            fwrite(&xut_file_size, sizeof(x_uint32_t), 1, xfile);

            // 修正 x_wave_chunk_data_t.xut_sub_chunk_2size 值
            // 即：正式数据部分的字节数
            x_uint32_t xut_data_size = xut_size - WAVE_HFD_SIZE + 4;
            fseek(xfile, WAVE_HFD_SIZE - sizeof(x_uint32_t), SEEK_SET);
            fwrite(&xut_data_size, sizeof(x_uint32_t), 1, xfile);

            fseek(xfile, 0, SEEK_END);
            fflush(xfile);
        }
#endif
        fclose(xfile);
        m_xht_file = X_NULL;
    }

    memset(&m_xfmt_info, 0, sizeof(x_format_t));
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
x_int32_t x_wave_file_writer_t::write(x_uchar_t * xct_dptr,
                                      x_uint32_t xut_dlen)
{
    if (!is_open())
    {
        return -1;
    }

    if ((X_NULL == xct_dptr) || (xut_dlen <= 0))
    {
        return 0;
    }

    x_int32_t xit_wlen = (x_int32_t)fwrite(xct_dptr,
                                           sizeof(x_uchar_t),
                                           xut_dlen,
                                           (FILE *)m_xht_file);

    return xit_wlen;
}
