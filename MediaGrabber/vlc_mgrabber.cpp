/**
 * @file    vlc_mgrabber.cpp
 * <pre>
 * Copyright (c) 2017, Gaaagaa All rights reserved.
 * 
 * 文件名称：vlc_mgrabber.cpp
 * 创建日期：2017年09月20日
 * 文件标识：
 * 文件摘要：使用 libvlc 对音视频文件解码后提取 视频图像、音频数据 的工具类。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2017年09月20日
 * 版本摘要：
 * 
 * 历史版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "vlc_mgrabber.h"

#ifdef _MSC_VER
typedef std::intptr_t  ssize_t;
#endif // _MSC_VER

#include <vlc/vlc.h>

#include <memory.h>
#include <assert.h>
#include <string>

////////////////////////////////////////////////////////////////////////////////

#ifndef VLC_FOURCC
#ifdef WORDS_BIGENDIAN
#define VLC_FOURCC(a, b, c, d) \
        (((x_uint32_t)d) | (((x_uint32_t)c) << 8) \
           | (((x_uint32_t)b) << 16) | (((x_uint32_t)a) << 24))
#else // !WORDS_BIGENDIAN
#define VLC_FOURCC(a, b, c, d) \
        (((x_uint32_t)a) | (((x_uint32_t)b) << 8) \
           | (((x_uint32_t)c) << 16) | (((x_uint32_t)d) << 24))
#endif // WORDS_BIGENDIAN
#endif // VLC_FOURCC

/**********************************************************/
/**
 * @brief 根据当音频数据格式，获取其对应的采样点的 bit 数；
 *        一般为 8, 16, 32 等。
 *
 * @param [in ] xut_fourcc : 音频数据格式。
 *
 * @return x_uint32_t
 *         - 返回 0，表示未知；
 *         - 返回 非 0 值，表示格式可识别。
 */
static x_uint32_t audio_fourcc_bits_per_sample(x_uint32_t xut_fourcc)
{
    x_uint32_t xut_bits = 0;

    switch (xut_fourcc)
    {
    case VLC_FOURCC('s','8',' ',' ') : xut_bits =  8; break;
    case VLC_FOURCC('u','8',' ',' ') : xut_bits =  8; break;
    case VLC_FOURCC('s','1','6','l') : xut_bits = 16; break;
    case VLC_FOURCC('s','1','l','p') : xut_bits = 16; break;
    case VLC_FOURCC('s','1','6','b') : xut_bits = 16; break;
    case VLC_FOURCC('u','1','6','l') : xut_bits = 16; break;
    case VLC_FOURCC('u','1','6','b') : xut_bits = 16; break;
    case VLC_FOURCC('s','2','0','b') : xut_bits = 20; break;
    case VLC_FOURCC('s','2','4','l') : xut_bits = 24; break;
    case VLC_FOURCC('s','2','4','b') : xut_bits = 24; break;
    case VLC_FOURCC('u','2','4','l') : xut_bits = 24; break;
    case VLC_FOURCC('u','2','4','b') : xut_bits = 24; break;
    case VLC_FOURCC('s','2','4','4') : xut_bits = 24; break;
    case VLC_FOURCC('S','2','4','4') : xut_bits = 24; break;
    case VLC_FOURCC('s','3','2','l') : xut_bits = 32; break;
    case VLC_FOURCC('s','3','2','b') : xut_bits = 32; break;
    case VLC_FOURCC('u','3','2','l') : xut_bits = 32; break;
    case VLC_FOURCC('u','3','2','b') : xut_bits = 32; break;
    case VLC_FOURCC('f','3','2','l') : xut_bits = 32; break;
    case VLC_FOURCC('f','3','2','b') : xut_bits = 32; break;
    case VLC_FOURCC('f','6','4','l') : xut_bits = 64; break;
    case VLC_FOURCC('f','6','4','b') : xut_bits = 64; break;

    case VLC_FOURCC('S','1','6','N') : xut_bits = 16; break;
    case VLC_FOURCC('U','1','6','N') : xut_bits = 16; break;
    case VLC_FOURCC('S','2','4','N') : xut_bits = 24; break;
    case VLC_FOURCC('U','2','4','N') : xut_bits = 24; break;
    case VLC_FOURCC('S','3','2','N') : xut_bits = 32; break;
    case VLC_FOURCC('U','3','2','N') : xut_bits = 32; break;
    case VLC_FOURCC('F','L','3','2') : xut_bits = 32; break;
    case VLC_FOURCC('F','L','6','4') : xut_bits = 64; break;

    case VLC_FOURCC('S','1','6','I') : xut_bits = 16; break;
    case VLC_FOURCC('U','1','6','I') : xut_bits = 16; break;
    case VLC_FOURCC('S','2','4','I') : xut_bits = 24; break;
    case VLC_FOURCC('U','2','4','I') : xut_bits = 24; break;
    case VLC_FOURCC('S','3','2','I') : xut_bits = 32; break;
    case VLC_FOURCC('U','3','2','I') : xut_bits = 32; break;

    default:
        break;
    }

    return xut_bits;
}

/**********************************************************/
/**
 * @brief media player handle.
 */
static inline libvlc_media_player_t * MPLAYER(x_handle_t xht_mplayer)
{
    return ((libvlc_media_player_t *)xht_mplayer);
}

////////////////////////////////////////////////////////////////////////////////
// x_vlc_media_grabber_t

//====================================================================

// 
// x_vlc_media_grabber_t : common invoking
// 

static_assert(vlc_mgrabber_t::EVENT_CALLBACK_END_REACHED      == (x_int32_t)libvlc_MediaPlayerEndReached     , "MediaPlayerEndReached     ");
static_assert(vlc_mgrabber_t::EVENT_CALLBACK_TIME_CHANGED     == (x_int32_t)libvlc_MediaPlayerTimeChanged    , "MediaPlayerTimeChanged    ");
static_assert(vlc_mgrabber_t::EVENT_CALLBACK_POSITION_CHANGED == (x_int32_t)libvlc_MediaPlayerPositionChanged, "MediaPlayerPositionChanged");
static_assert(vlc_mgrabber_t::EVENT_CALLBACK_LENGTH_CHANGED   == (x_int32_t)libvlc_MediaPlayerLengthChanged  , "MediaPlayerLengthChanged  ");

// the global libvlc instance.
x_handle_t vlc_mgrabber_t::ms_xht_vlc_instance = X_NULL;

/**********************************************************/
/**
 * @brief Startup the libvlc.
 */
x_int32_t vlc_mgrabber_t::startup(x_int32_t xit_argc, const x_char_t * const * xct_argv)
{
    x_int32_t xit_err = -1;

    do 
    {
        //======================================

        if (X_NULL != ms_xht_vlc_instance)
        {
            xit_err = 0;
            break;
        }

        //======================================

        ms_xht_vlc_instance = create_instance(xit_argc, xct_argv);
        if (X_NULL == ms_xht_vlc_instance)
        {
            xit_err = libvlc_Error;
            break;
        }

        //======================================

        xit_err = 0;
    } while (0);

    if (0 != xit_err)
    {
        cleanup();
    }

    return xit_err;
}

/**********************************************************/
/**
 * @brief Cleanup the libvlc.
 */
x_void_t vlc_mgrabber_t::cleanup(void)
{
    if (X_NULL != ms_xht_vlc_instance)
    {
        release_instance(ms_xht_vlc_instance);
        ms_xht_vlc_instance = X_NULL;
    }
}

/**********************************************************/
/**
 * @brief Create and initialize a libvlc instance.
 */
x_handle_t vlc_mgrabber_t::create_instance(x_int32_t xit_argc,
                                           const x_char_t * const * xct_argv)
{
    return (x_handle_t)libvlc_new(xit_argc, xct_argv);
}

/**********************************************************/
/**
 * @brief Create and initialize a libvlc instance.
 */
x_void_t vlc_mgrabber_t::release_instance(x_handle_t xht_instance)
{
    if (X_NULL != xht_instance)
    {
        libvlc_release((libvlc_instance_t *)xht_instance);
        xht_instance = X_NULL;
    }
}

//====================================================================

// 
// x_vlc_media_grabber_t : constructor/destructor
// 

vlc_mgrabber_t::vlc_mgrabber_t(void)
    : m_xht_vlc_instance(X_NULL)
    , m_xht_vlc_mplayer(X_NULL)
    , m_xfunc_video_cbk(X_NULL)
    , m_xfunc_audio_cbk(X_NULL)
    , m_xfunc_event_cbk(X_NULL)
    , m_xpvt_xfunc_ctxt(X_NULL)
    , m_xbt_video_parse(X_FALSE)
    , m_xit_video_nbits(32)
    , m_xit_video_pitch(0)
    , m_xit_video_width(0)
    , m_xit_video_height(0)
    , m_xut_audio_fourcc(0)
    , m_xut_audio_bpsamp(0)
    , m_xut_audio_rate(0)
    , m_xut_audio_channels(0)
{
    memset(&m_xdt_video_data, 0, sizeof(x_vcbk_data_t));
}

vlc_mgrabber_t::~vlc_mgrabber_t(void)
{
    close();
}

//====================================================================

// 
// x_vlc_media_grabber_t : public interfaces
// 

/**********************************************************/
/**
 * @brief 打开工作流程（操作前请先调用 set_callback() 设置好回调参数）。
 *
 * @param [in ] xht_instance    : 关联的 libvlc 实例句柄（若为 X_NULL，则取全局的实例句柄）。
 * @param [in ] xszt_media_file : 音视频文件路径名。
 * @param [in ] xszt_options    : 附加的参数选项（使用 " :" （空格+':'）进行分隔的字符串集，为 X_NULL 时，则忽略）。
 *
 * @return x_int32_t
 *         - 成功，返回 0；
 *         - 失败，返回 错误码。
 */
x_int32_t vlc_mgrabber_t::open(x_handle_t xht_instance,
                               x_cstring_t xszt_media_file,
                               x_cstring_t xszt_options)
{
    x_int32_t xit_err = -1;

    libvlc_media_t         * vlc_media_ptr  = X_NULL;
    libvlc_media_player_t  * vlc_player_ptr = X_NULL;

    do
    {
        //======================================

        if (X_NULL == xszt_media_file)
        {
            break;
        }

        if (X_NULL == xht_instance)
        {
            if (X_NULL == ms_xht_vlc_instance)
                break;
            xht_instance = ms_xht_vlc_instance;
        }

        close();

        //======================================

        vlc_media_ptr = libvlc_media_new_location(
                                    (libvlc_instance_t *)xht_instance,
                                    xszt_media_file);
        if (X_NULL == vlc_media_ptr)
        {
            break;
        }

        //======================================
        // 设置播放器对象的工作配置选项

        if (X_NULL != xszt_options)
        {
            std::string xstr_options = xszt_options;
            x_size_t xst_bpos = std::string::npos;

            for (x_size_t xst_iter = xstr_options.find(" :", 0);
                 xst_iter != std::string::npos; )
            {
                xst_bpos = xst_iter + 2;
                xst_iter = xstr_options.find(" :", xst_bpos);

                std::string xstr_split =
                                xstr_options.substr(xst_bpos,
                                (xst_iter != std::string::npos) ?
                                (xst_iter - xst_bpos) :
                                xst_iter);

                if (xstr_split.size() > 0)
                {
                    libvlc_media_add_option(vlc_media_ptr, xstr_split.c_str());
                }
            }
        }

        //======================================
        // 构建播放器对象

        vlc_player_ptr = libvlc_media_player_new_from_media(vlc_media_ptr);
        if (X_NULL == vlc_player_ptr)
        {
            break;
        }

        //======================================
        // 设置相关的回调操作接口

        // 设置视频回调
        if (X_NULL != m_xfunc_video_cbk)
        {
            libvlc_video_set_callbacks(
                    vlc_player_ptr,
                    (libvlc_video_lock_cb   )&vlc_mgrabber_t::video_lock_cbk,
                    (libvlc_video_unlock_cb )&vlc_mgrabber_t::video_unlock_cbk,
                    (libvlc_video_display_cb)&vlc_mgrabber_t::video_display_cbk,
                    (x_pvoid_t)this);

            libvlc_video_set_format_callbacks(
                    vlc_player_ptr,
                    (libvlc_video_format_cb)&vlc_mgrabber_t::video_format_cbk,
                    X_NULL);
        }

        // 设置音频回调
        if (X_NULL != m_xfunc_audio_cbk)
        {
            libvlc_audio_set_callbacks(
                    vlc_player_ptr,
                    (libvlc_audio_play_cb  )&vlc_mgrabber_t::audio_play_cbk,
                    (libvlc_audio_pause_cb )&vlc_mgrabber_t::audio_pause_cbk,
                    (libvlc_audio_resume_cb)&vlc_mgrabber_t::audio_resume_cbk,
                    (libvlc_audio_flush_cb )&vlc_mgrabber_t::audio_flush_cbk,
                    (libvlc_audio_drain_cb )&vlc_mgrabber_t::audio_drain_cbk,
                    (x_pvoid_t)this);

            libvlc_audio_set_volume_callback(
                    vlc_player_ptr,
                    (libvlc_audio_set_volume_cb)&vlc_mgrabber_t::audio_set_volume_cbk);

            libvlc_audio_set_format_callbacks(
                    vlc_player_ptr,
                    (libvlc_audio_setup_cb)&vlc_mgrabber_t::audio_setup_cbk,
                    (libvlc_audio_cleanup_cb)&vlc_mgrabber_t::audio_cleanup_cbk);
        }

        // 设置监听事件
        if (X_NULL != m_xfunc_event_cbk)
        {
            libvlc_event_manager_t * event_mgr_ptr =
                    libvlc_media_player_event_manager(vlc_player_ptr);

            libvlc_event_attach(event_mgr_ptr,
                                libvlc_MediaPlayerEndReached,
                                (libvlc_callback_t)&vlc_mgrabber_t::handle_event_cbk,
                                this);
            libvlc_event_attach(event_mgr_ptr,
                                libvlc_MediaPlayerTimeChanged,
                                (libvlc_callback_t)&vlc_mgrabber_t::handle_event_cbk,
                                this);
            libvlc_event_attach(event_mgr_ptr,
                                libvlc_MediaPlayerPositionChanged,
                                (libvlc_callback_t)&vlc_mgrabber_t::handle_event_cbk,
                                this);
            libvlc_event_attach(event_mgr_ptr,
                                libvlc_MediaPlayerLengthChanged,
                                (libvlc_callback_t)&vlc_mgrabber_t::handle_event_cbk,
                                this);
        }

        //======================================

        xit_err = libvlc_media_player_play(vlc_player_ptr);
        if (0 != xit_err)
        {
            break;
        }

        m_xht_vlc_instance = xht_instance;
        m_xht_vlc_mplayer  = (x_handle_t)vlc_player_ptr;
        vlc_player_ptr     = X_NULL;

        //======================================

        xit_err = 0;
    } while (0);

    if (X_NULL != vlc_media_ptr)
    {
        libvlc_media_release(vlc_media_ptr);
        vlc_media_ptr = X_NULL;
    }

    if (X_NULL != vlc_player_ptr)
    {
        libvlc_media_player_stop(vlc_player_ptr);
        libvlc_media_player_release(vlc_player_ptr);
        vlc_player_ptr = X_NULL;
    }

    if (0 != xit_err)
    {
        close();
    }

    return xit_err;
}

/**********************************************************/
/**
 * @brief 关闭工作流程。
 */
x_void_t vlc_mgrabber_t::close(void)
{
    //======================================

    m_xht_vlc_instance = X_NULL;

    if (X_NULL != m_xht_vlc_mplayer)
    {
        libvlc_media_player_stop(MPLAYER(m_xht_vlc_mplayer));
        libvlc_media_player_release(MPLAYER(m_xht_vlc_mplayer));
        m_xht_vlc_mplayer = X_NULL;
    }

    //======================================

    m_xbt_video_parse  = X_FALSE;
    m_xit_video_nbits  = 32;
    m_xit_video_pitch  = 0;
    m_xit_video_width  = 0;
    m_xit_video_height = 0;
    memset(&m_xdt_video_data, 0, sizeof(x_vcbk_data_t));

    //======================================

    m_xut_audio_fourcc   = 0;
    m_xut_audio_bpsamp   = 0;
    m_xut_audio_rate     = 0;
    m_xut_audio_channels = 0;

    //======================================
}

/**********************************************************/
/**
 * @brief 工作流程是否已经打开。
 */
x_bool_t vlc_mgrabber_t::is_open(void) const
{
    return ((X_NULL != m_xht_vlc_mplayer) ? X_TRUE : X_FALSE);
}

//====================================================================
// 播放器控制相关接口

/**
 * is_playing
 *
 * \return 1 if the media player is playing, 0 otherwise
 *
 * \libvlc_return_bool
 */
x_bool_t vlc_mgrabber_t::is_playing()
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        return ((1 == libvlc_media_player_is_playing(MPLAYER(m_xht_vlc_mplayer))) ?
                    X_TRUE : X_FALSE);
    }

    return X_FALSE;
}

/**
 * Get the current movie length (in ms).
 *
 * \return the movie length (in ms), or -1 if there is no media.
 */
x_llong_t vlc_mgrabber_t::get_length()
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        return libvlc_media_player_get_length(MPLAYER(m_xht_vlc_mplayer));
    }

    return -1L;
}

/**
 * Get the current movie time (in ms).
 *
 * \return the movie time (in ms), or -1 if there is no media.
 */
x_llong_t vlc_mgrabber_t::get_time()
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        return libvlc_media_player_get_time(MPLAYER(m_xht_vlc_mplayer));
    }

    return -1L;
}

/**
 * Set the movie time (in ms). This has no effect if no media is being played.
 * Not all formats and protocols support this.
 *
 * \param xllt_time the movie time (in ms).
 */
x_void_t vlc_mgrabber_t::set_time(x_llong_t xllt_time)
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        libvlc_media_player_set_time(MPLAYER(m_xht_vlc_mplayer), xllt_time);
    }
}

/**
 * Is this media player seekable?
 *
 * \return true if the media player can seek
 *
 * \libvlc_return_bool
 */
x_bool_t vlc_mgrabber_t::is_seekable()
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        return ((0 != libvlc_media_player_is_seekable(MPLAYER(m_xht_vlc_mplayer))) ?
                    X_TRUE : X_FALSE);
    }

    return X_FALSE;
}

/**
 * Get movie position as percentage between 0.0 and 1.0.
 *
 * \return movie position, or -1. in case of error
 */
x_float_t vlc_mgrabber_t::get_position()
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        return libvlc_media_player_get_position(MPLAYER(m_xht_vlc_mplayer));
    }

    return -1.0F;
}

/**
 * Set movie position as percentage between 0.0 and 1.0. 
 * This has no effect if playback is not enabled.
 * This might not work depending on the underlying input format and protocol.
 *
 * \param xft_pos the position
 */
x_void_t vlc_mgrabber_t::set_position(x_float_t xft_pos)
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        libvlc_media_player_set_position(MPLAYER(m_xht_vlc_mplayer), xft_pos);
    }
}

/**
 * Get the requested movie play rate.
 * @warning Depending on the underlying media, the requested rate may be
 * different from the real playback rate.
 *
 * \return movie play rate
 */
x_float_t vlc_mgrabber_t::get_rate()
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        return libvlc_media_player_get_rate(MPLAYER(m_xht_vlc_mplayer));
    }

    return 1.0F;
}

/**
 * Set movie play rate
 *
 * \param xft_rate movie play rate to set
 * \return -1 if an error was detected, 0 otherwise (but even then, it might
 * not actually work depending on the underlying media protocol)
 */
x_int32_t vlc_mgrabber_t::set_rate(x_float_t xft_rate)
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        return libvlc_media_player_set_rate(MPLAYER(m_xht_vlc_mplayer), xft_rate);
    }

    return -1;
}

/**
 * Can this media player be paused?
 *
 * \return true if the media player can pause
 *
 * \libvlc_return_bool
 */
x_bool_t vlc_mgrabber_t::can_pause()
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        return ((0 != libvlc_media_player_can_pause(MPLAYER(m_xht_vlc_mplayer))) ?
                    X_TRUE : X_FALSE);
    }

    return X_FALSE;
}

/**
 * Pause or resume (no effect if there is no media)
 *
 * \param xbt_pause play/resume if X_FALSE, pause if X_TRUE
 * \version LibVLC 1.1.1 or later
 */
x_void_t vlc_mgrabber_t::set_pause(x_bool_t xbt_pause)
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        libvlc_media_player_set_pause(MPLAYER(m_xht_vlc_mplayer),
                                      (x_int32_t)xbt_pause);
    }
}

/**
 * Toggle pause (no effect if there is no media)
 */
x_void_t vlc_mgrabber_t::pause()
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        libvlc_media_player_pause(MPLAYER(m_xht_vlc_mplayer));
    }
}

/**
 * Toggle mute status.
 *
 * \warning Toggling mute atomically is not always possible: On some platforms,
 * other processes can mute the VLC audio playback stream asynchronously. Thus,
 * there is a small race condition where toggling will not work.
 * See also the limitations of audio_set_mute().
 */
x_void_t vlc_mgrabber_t::audio_toggle_mute()
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        libvlc_audio_toggle_mute(MPLAYER(m_xht_vlc_mplayer));
    }
}

/**
 * Get current mute status.
 *
 * \return the mute status (boolean) if defined, -1 if undefined/unapplicable
 */
x_bool_t vlc_mgrabber_t::audio_get_mute()
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        return ((0 != libvlc_audio_get_mute(MPLAYER(m_xht_vlc_mplayer))) ?
                    X_TRUE : X_FALSE);
    }

    return X_FALSE;
}

/**
 * Set mute status.
 *
 * \param xbt_status If status is X_TRUE then mute, otherwise unmute
 * \warning This function does not always work. If there are no active audio
 * playback stream, the mute status might not be available. If digital
 * pass-through (S/PDIF, HDMI...) is in use, muting may be unapplicable. Also
 * some audio output plugins do not support muting at all.
 * \note To force silent playback, disable all audio tracks. This is more
 * efficient and reliable than mute.
 */
x_void_t vlc_mgrabber_t::audio_set_mute(x_bool_t xbt_status)
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        libvlc_audio_set_mute(MPLAYER(m_xht_vlc_mplayer),
                              (x_int32_t)xbt_status);
    }
}

/**
 * Get current software audio volume.
 *
 * \return the software volume in percents
 * (0 = mute, 100 = nominal / 0dB)
 */
x_int32_t vlc_mgrabber_t::audio_get_volume()
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        return libvlc_audio_get_volume(MPLAYER(m_xht_vlc_mplayer));
    }

    return X_FALSE;
}

/**
 * Set current software audio volume.
 *
 * \param xit_volume the volume in percents (0 = mute, 100 = 0dB)
 * \return 0 if the volume was set, -1 if it was out of range
 */
x_int32_t vlc_mgrabber_t::audio_set_volume(x_int32_t xit_volume)
{
    if (X_NULL != m_xht_vlc_mplayer)
    {
        return libvlc_audio_set_volume(MPLAYER(m_xht_vlc_mplayer),
                                       xit_volume);
    }

    return -1;
}

//====================================================================

// 
// vlc_mgrabber_t : inner invoking
// 

#if 1

/**********************************************************/
/**
 * @brief 执行视频信息解析操作，提取其 宽高信息。
 * @note  废弃的接口。@deprecated
 */
x_bool_t vlc_mgrabber_t::video_parse_wh(x_handle_t xht_vlc_mplayer,
                                        x_int32_t & xit_width,
                                        x_int32_t & xit_height)
{
    x_bool_t xbt_parse = X_FALSE;

    libvlc_media_t * vlc_media_ptr = X_NULL;

    x_int32_t xit_cx = 0;
    x_int32_t xit_cy = 0;

    do
    {
        //======================================

        if (X_NULL == xht_vlc_mplayer)
        {
            break;
        }

        vlc_media_ptr = libvlc_media_player_get_media(MPLAYER(xht_vlc_mplayer));
        if (X_NULL == vlc_media_ptr)
        {
            break;
        }

        libvlc_media_parse(vlc_media_ptr);
#if 0
        libvlc_media_track_info_t * vlc_minfo_ptr = X_NULL;
        x_int32_t xit_es_count = libvlc_media_get_tracks_info(vlc_media_ptr,
                                                              &vlc_minfo_ptr);
        if (xit_es_count <= 0)
        {
            break;
        }

        //======================================

        for (x_int32_t xit_iter = 0; xit_iter < xit_es_count; ++xit_iter)
        {
            if (libvlc_track_video == vlc_minfo_ptr[xit_iter].i_type)
            {
                xit_cx = (x_int32_t)vlc_minfo_ptr[xit_iter].u.video.i_width;
                xit_cy = (x_int32_t)vlc_minfo_ptr[xit_iter].u.video.i_height;
                if ((xit_cx <= 0) || (xit_cy == 0))
                {
                    continue;
                }

                xbt_parse = X_TRUE;
                break;
            }
        }

        libvlc_free(vlc_minfo_ptr);
        vlc_minfo_ptr = X_NULL;

        //======================================
#else
        libvlc_media_track_t ** vlc_mtrack_ptr = X_NULL;
        x_uint32_t xut_count = libvlc_media_tracks_get(vlc_media_ptr,
                                                       &vlc_mtrack_ptr);
        if (xut_count <= 0)
        {
            break;
        }

        //======================================

        for (x_uint32_t xut_iter = 0; xut_iter < xut_count; ++xut_iter)
        {
            if (libvlc_track_video == vlc_mtrack_ptr[xut_iter]->i_type)
            {
                xit_cx = (x_int32_t)vlc_mtrack_ptr[xut_iter]->video->i_width;
                xit_cy = (x_int32_t)vlc_mtrack_ptr[xut_iter]->video->i_height;
                if ((xit_cx <= 0) || (xit_cy == 0))
                {
                    continue;
                }

                xbt_parse = X_TRUE;
                break;
            }
        }

        libvlc_media_tracks_release(vlc_mtrack_ptr, xut_count);
        vlc_mtrack_ptr = X_NULL;
#endif
        //======================================
    } while (0);

    if (X_NULL != vlc_media_ptr)
    {
        libvlc_media_release(vlc_media_ptr);
        vlc_media_ptr = X_NULL;
    }

    if (xbt_parse)
    {
        xit_width  = xit_cx;
        xit_height = xit_cy;
    }

    return xbt_parse;
}

/**********************************************************/
/**
 * @brief 执行视频信息解析操作，提取其 宽高信息;
 *        取代 video_parse_wh() 接口。
 * @note  废弃的接口。@deprecated
 */
x_bool_t vlc_mgrabber_t::video_parse_wh_2(x_handle_t xht_vlc_mplayer,
                                          x_int32_t & xit_width,
                                          x_int32_t & xit_height)
{
    x_bool_t xbt_parse = X_FALSE;

    libvlc_media_t * vlc_media_ptr = X_NULL;

    x_int32_t xit_cx = 0;
    x_int32_t xit_cy = 0;

    do
    {
        //======================================

        if (X_NULL == xht_vlc_mplayer)
        {
            break;
        }

        vlc_media_ptr = libvlc_media_player_get_media(MPLAYER(xht_vlc_mplayer));
        if (X_NULL == vlc_media_ptr)
        {
            break;
        }

        if (libvlc_media_parsed_status_done ==
            libvlc_media_get_parsed_status(vlc_media_ptr))
        {
            break;
        }

        libvlc_media_track_t ** vlc_mtrack_ptr = X_NULL;
        x_uint32_t xut_count = libvlc_media_tracks_get(vlc_media_ptr,
                                                       &vlc_mtrack_ptr);
        if (xut_count <= 0)
        {
            break;
        }

        //======================================

        for (x_uint32_t xut_iter = 0; xut_iter < xut_count; ++xut_iter)
        {
            if (libvlc_track_video == vlc_mtrack_ptr[xut_iter]->i_type)
            {
                xit_cx = (x_int32_t)vlc_mtrack_ptr[xut_iter]->video->i_width;
                xit_cy = (x_int32_t)vlc_mtrack_ptr[xut_iter]->video->i_height;
                if ((xit_cx <= 0) || (xit_cy == 0))
                {
                    continue;
                }

                xbt_parse = X_TRUE;
                break;
            }
        }

        libvlc_media_tracks_release(vlc_mtrack_ptr, xut_count);
        vlc_mtrack_ptr = X_NULL;

        //======================================
    } while (0);

    if (X_NULL != vlc_media_ptr)
    {
        libvlc_media_release(vlc_media_ptr);
        vlc_media_ptr = X_NULL;
    }

    if (xbt_parse)
    {
        xit_width  = xit_cx;
        xit_height = xit_cy;
    }

    return xbt_parse;
}

#endif

/**********************************************************/
/**
 * @brief 视频格式回调接口。
 */
x_uint32_t vlc_mgrabber_t::video_format(x_char_t * xct_chroma,
                                        x_uint32_t * xut_width,
                                        x_uint32_t * xut_height,
                                        x_uint32_t * xut_pitches,
                                        x_uint32_t * xut_lines)
{
    //======================================

    if (!m_xbt_video_parse)
    {
        m_xbt_video_parse = video_parse_wh(m_xht_vlc_mplayer,
                                           m_xit_video_width,
                                           m_xit_video_height);
        if (m_xit_video_height < 0)
        {
            m_xit_video_height = -m_xit_video_height;
        }
    }

    if (!m_xbt_video_parse)
    {
        m_xit_video_width  = (x_int32_t)*xut_width;
        m_xit_video_height = (x_int32_t)*xut_height;
    }

    //======================================

#define RGB_PITCH(width, nbits) ((((width) * (x_int32_t)(nbits)) + 31) / 32 * 4)

    m_xit_video_pitch = RGB_PITCH(m_xit_video_width, m_xit_video_nbits);

    //======================================

    m_xdt_video_data.xht_handle   = X_NULL;
    m_xdt_video_data.xbt_bits_ptr = X_NULL;
    m_xdt_video_data.xit_nbits    = m_xit_video_nbits;
    m_xdt_video_data.xit_pitch    = m_xit_video_pitch;
    m_xdt_video_data.xit_width    = m_xit_video_width;
    m_xdt_video_data.xit_height   = m_xit_video_height;

    if (X_NULL != m_xfunc_video_cbk)
    {
        m_xfunc_video_cbk(VIDEO_CALLBACK_FORMAT,
                          &m_xdt_video_data,
                          m_xpvt_xfunc_ctxt);

        assert((24 == m_xdt_video_data.xit_nbits) ||
               (32 == m_xdt_video_data.xit_nbits));
        assert((m_xdt_video_data.xit_pitch  > 0) &&
               (m_xdt_video_data.xit_width  > 0) &&
               (m_xdt_video_data.xit_height > 0));
        assert(m_xdt_video_data.xit_pitch >=
               RGB_PITCH(m_xdt_video_data.xit_width,
                         m_xdt_video_data.xit_nbits));

        m_xit_video_nbits  = m_xdt_video_data.xit_nbits ;
        m_xit_video_pitch  = m_xdt_video_data.xit_pitch ;
        m_xit_video_width  = m_xdt_video_data.xit_width ;
        m_xit_video_height = m_xdt_video_data.xit_height;
    }

#undef RGB_PITCH

    //======================================

    xct_chroma[0]  = 'R';
    xct_chroma[1]  = 'V';
    xct_chroma[2]  = (32 == m_xit_video_nbits) ? '3' : '2';
    xct_chroma[3]  = (32 == m_xit_video_nbits) ? '2' : '4';
    *xut_width     = (x_uint32_t)m_xit_video_width;
    *xut_height    = (x_uint32_t)m_xit_video_height;
    xut_pitches[0] = (x_uint32_t)m_xit_video_pitch;
    xut_lines  [0] = (x_uint32_t)m_xit_video_height;

    //======================================

    return 1;
}

/**********************************************************/
/**
 * @brief 视频播放回调接口 - 图像解码前的加锁操作（获取图像解码缓冲区）。
 */
x_pvoid_t vlc_mgrabber_t::video_lock(x_pvoid_t * xpvt_planes)
{
    assert((m_xit_video_width > 0) && (m_xit_video_height != 0));

    m_xdt_video_data.xht_handle   = X_NULL;
    m_xdt_video_data.xbt_bits_ptr = X_NULL;
    m_xdt_video_data.xit_nbits    = m_xit_video_nbits;
    m_xdt_video_data.xit_pitch    = m_xit_video_pitch;
    m_xdt_video_data.xit_width    = m_xit_video_width;
    m_xdt_video_data.xit_height   = m_xit_video_height;

    if (X_NULL != m_xfunc_video_cbk)
    {
        m_xfunc_video_cbk(VIDEO_CALLBACK_LOCK,
                          &m_xdt_video_data,
                          m_xpvt_xfunc_ctxt);
    }

    assert(X_NULL != m_xdt_video_data.xbt_bits_ptr);

    *(x_uchar_t **)xpvt_planes = m_xdt_video_data.xbt_bits_ptr;

    return X_NULL;
}

/**********************************************************/
/**
 * @brief 视频播放回调接口 - 图像解码后的解锁操作。
 */
x_void_t vlc_mgrabber_t::video_unlock(x_pvoid_t xpvt_picture,
                                      x_pvoid_t const * xpvt_planes)
{
    if (X_NULL != m_xfunc_video_cbk)
    {
        m_xfunc_video_cbk(VIDEO_CALLBACK_UNLOCK,
                          &m_xdt_video_data,
                          m_xpvt_xfunc_ctxt);
    }
}

/**********************************************************/
/**
 * @brief 视频播放回调接口 - 图像解码后的显示操作。
 */
x_void_t vlc_mgrabber_t::video_display(x_pvoid_t xpvt_picture)
{
    if (X_NULL != m_xfunc_video_cbk)
    {
        m_xfunc_video_cbk(VIDEO_CALLBACK_DISPLAY,
                          &m_xdt_video_data,
                          m_xpvt_xfunc_ctxt);
    }
}

/**********************************************************/
/**
 * Callback prototype to setup the audio playback.
 * This is called when the media player needs to create a new audio output.
 * \param xct_format 4 bytes sample format [IN/OUT]
 * \param rate sample rate [IN/OUT]
 * \param channels channels count [IN/OUT]
 * \return 0 on success, anything else to skip audio playback
 */
x_int32_t vlc_mgrabber_t::audio_setup(x_char_t * xct_format,
                                      x_uint32_t * xut_rate,
                                      x_uint32_t * xut_channels)
{
    m_xut_audio_fourcc   = VLC_FOURCC(xct_format[0], xct_format[1], xct_format[2], xct_format[3]);
    m_xut_audio_bpsamp   = audio_fourcc_bits_per_sample(m_xut_audio_fourcc);
    m_xut_audio_rate     = *xut_rate;
    m_xut_audio_channels = *xut_channels;

    if (X_NULL != m_xfunc_audio_cbk)
    {
        m_xfunc_audio_cbk(AUDIO_CALLBACK_FORMAT, X_NULL, 0, 0, m_xpvt_xfunc_ctxt);
    }

    return 0;
}

/**********************************************************/
/**
 * Callback prototype for audio playback cleanup.
 * This is called when the media player no longer needs an audio output.
 */
x_void_t vlc_mgrabber_t::audio_cleanup()
{

}

/**********************************************************/
/**
 * @brief real audio callback.
 * xit_ptype:
 *      1, Callback prototype for audio format.
 *         xpvt_dptr == X_NULL.
 *      2, Callback prototype for audio playback.
 *         xpvt_dptr == pointer to the first audio sample to play back.
 *      3, Callback prototype for audio pause.
 *         xpvt_dptr == X_NULL.
 *      4, Callback prototype for audio resumption (i.e. restart from pause).
 *         xpvt_dptr == X_NULL.
 *      5, Callback prototype for audio buffer flush.
 *         xpvt_dptr == X_NULL.
 *      6, Callback prototype for audio buffer drain.
 *         xpvt_dptr == X_NULL.
 *      7, Callback prototype for audio volume change.
 *         xpvt_dptr == { x_float_t : volume software volume (1. = nominal, 0. = mute),
 *                        x_int32_t : muted flag. }.
 *
 * @param [in ] xit_ptype : the callback type, @see x_audio_callback_type_t.
 * @param [in ] xpvt_dptr : the callback data.
 * @param [in ] xut_size  : the callback data's size.
 * @param [in ] xit_pts   : time stamp.
 *
 * @return x_void_t
 *
 */
x_void_t vlc_mgrabber_t::real_audio_callback(x_int32_t xit_ptype,
                                             const x_pvoid_t xpvt_dptr,
                                             x_uint32_t xut_size,
                                             x_int64_t xit_pts)
{
    if (X_NULL != m_xfunc_audio_cbk)
    {
        if (AUDIO_CALLBACK_PLAYBACK == xit_ptype)
        {
            xut_size = xut_size * m_xut_audio_channels * m_xut_audio_bpsamp / 8;
        }

        m_xfunc_audio_cbk(xit_ptype, xpvt_dptr, xut_size, xit_pts, m_xpvt_xfunc_ctxt);
    }
}

/**********************************************************/
/**
 * @brief 事件回调处理接口。
 */
x_void_t vlc_mgrabber_t::handle_event_callback(x_handle_t xht_event)
{
    libvlc_event_t * levent_ptr = (libvlc_event_t *)xht_event;

#if 0
    switch (levent_ptr->type)
    {
    case libvlc_MediaPlayerEndReached:
        break;

    case libvlc_MediaPlayerTimeChanged:
        break;

    case libvlc_MediaPlayerPositionChanged:
        break;

    case libvlc_MediaPlayerLengthChanged:
        break;

    default:
        break;
    }
#endif

    if (X_NULL != m_xfunc_event_cbk)
    {
        m_xfunc_event_cbk(levent_ptr->type, 0, 0, m_xpvt_xfunc_ctxt);
    }
}
