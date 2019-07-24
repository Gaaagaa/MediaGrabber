/**
 * @file   vlc_mgrabber.h
 * <pre>
 * Copyright (c) 2017, Gaaagaa All rights reserved.
 *
 * 文件名称：vlc_mgrabber.h
 * 创建日期：2017年09月20日
 * 文件标识：
 * 文件摘要：使用 libvlc 对音视频文件解码后提取 视频图像、音频数据 的工具类。
 *
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2017年09月20日
 * 版本摘要：
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef __VLC_MGRABBER_H__
#define __VLC_MGRABBER_H__

#include "xtypes.h"
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// vlc_mgrabber_t

/**
 * @class vlc_mgrabber_t
 * @brief 使用 libvlc 对音视频文件解码后提取 视频图像、音频数据 的工具类。
 */
class vlc_mgrabber_t
{
    // common data types
public:
    /**
     * @enum  x_video_callback_prototype_t
     * @brief Callback prototype for video.
     */
    typedef enum __video_callback_prototype__
    {
        VIDEO_CALLBACK_FORMAT  = 1,  ///< Callback prototype to notice the frame's size format.
        VIDEO_CALLBACK_LOCK    = 2,  ///< Callback prototype to allocate and lock a frame buffer.
        VIDEO_CALLBACK_UNLOCK  = 3,  ///< Callback prototype to unlock a frame buffer.
        VIDEO_CALLBACK_DISPLAY = 4,  ///< Callback prototype to display a frame.
    } x_video_callback_prototype_t;

    /**
     * @struct x_video_callback_data_t
     * @brief  Callbakc data for video.
     */
    typedef struct __video_callback_data__
    {
        x_handle_t   xht_handle;   ///< the user-defined handle.
        x_byte_t   * xbt_bits_ptr; ///< the buffer for video frame output.
        x_int32_t    xit_pitch;    ///< the buffer line stride.
        x_int32_t    xit_width;    ///< the real frame's width.
        x_int32_t    xit_height;   ///< the real frame's height.
    } x_video_callback_data_t;

    /**
     * @brief Callback function type for video.
     *
     * @param [in    ] xit_ptype : callback prototype, @see x_video_callback_prototype_t.
     * @param [in,out] xvct_dptr : interactive data for callback operations.
     * @param [in    ] xpvt_ctxt : the user context description.
     */
    typedef x_void_t (* xfunc_video_cbk_t)(x_int32_t xit_ptype,
                                           x_video_callback_data_t * xvct_dptr,
                                           x_pvoid_t xpvt_ctxt);

    /**
     * @enum  x_audio_callback_prototype_t
     * @brief Callback prototype for audio.
     */
    typedef enum __audio_callback_prototype__
    {
        AUDIO_CALLBACK_FORMAT        = 1,   ///< Callback prototype for audio format.
        AUDIO_CALLBACK_PLAYBACK      = 2,   ///< Callback prototype for audio playback.
        AUDIO_CALLBACK_PAUSE         = 3,   ///< Callback prototype for audio pause.
        AUDIO_CALLBACK_RESUMPTION    = 4,   ///< Callback prototype for audio resumption (i.e. restart from pause).
        AUDIO_CALLBACK_BUFFER_FLUSH  = 5,   ///< Callback prototype for audio buffer flush.
        AUDIO_CALLBACK_BUFFER_DRAIN  = 6,   ///< Callback prototype for audio buffer drain.
        AUDIO_CALLBACK_VOLUME_CHANGE = 7,   ///< Callback prototype for audio volume change.
    } x_audio_callback_prototype_t;

    /**
     * @brief Callback function type for audio.
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
     * @param [in ] xit_ptype : the callback type, @see x_audio_callback_prototype_t.
     * @param [in ] xpvt_dptr : the callback data.
     * @param [in ] xut_size  : the callback data's size.
     * @param [in ] xit_pts   : time stamp.
     * @param [in ] xpvt_ctxt : the user context description.
     */
    typedef x_void_t (* xfunc_audio_cbk_t)(x_int32_t xit_ptype,
                                           x_pvoid_t xpvt_dptr,
                                           x_uint32_t xut_size,
                                           x_int64_t xit_pts,
                                           x_pvoid_t xpvt_ctxt);

    /**
     * @enum  x_event_callback_prototype_t
     * @brief Callback prototype for event.
     */
    typedef enum __event_callback_prototype__
    {
        EVENT_CALLBACK_END_REACHED      = 265,   ///< media player end reached.
        EVENT_CALLBACK_TIME_CHANGED     = 267,   ///< media player time changed.
        EVENT_CALLBACK_POSITION_CHANGED = 268,   ///< media player position changed.
        EVENT_CALLBACK_LENGTH_CHANGED   = 273,   ///< media player length changed.
    } x_event_callback_prototype_t;

    /**
     * @brief Callback function type for event.
     *
     * @param [in ] xit_event   : the callback event code.
     * @param [in ] xlpt_param1 : param1.
     * @param [in ] xlpt_param2 : param2.
     * @param [in ] xpvt_ctxt   : the user context description.
     */
    typedef x_void_t (* xfunc_event_cbk_t)(x_int32_t xit_event,
                                           x_lptr_t xlpt_param1,
                                           x_lptr_t xlpt_param2,
                                           x_pvoid_t xpvt_ctxt);

    // common invoking
public:
    /**********************************************************/
    /**
     * @brief Startup the libvlc.
     */
    static x_int32_t startup(x_int32_t xit_argc, const x_char_t * const * xct_argv);

    /**********************************************************/
    /**
     * @brief Cleanup the libvlc.
     */
    static x_void_t cleanup(void);

    /**********************************************************/
    /**
     * @brief Create and initialize a libvlc instance.
     */
    static x_handle_t create_instance(x_int32_t xit_argc, const x_char_t * const * xct_argv);

    /**********************************************************/
    /**
     * @brief Create and initialize a libvlc instance.
     */
    static x_void_t release_instance(x_handle_t xht_instance);

protected:
    /**
     * Callback prototype to configure picture buffers format.
     */
    static x_uint32_t video_format_cbk(x_pvoid_t * xpvt_context,
                                       x_char_t * xct_chroma,
                                       x_uint32_t * xut_width,
                                       x_uint32_t * xut_height,
                                       x_uint32_t * xut_pitches,
                                       x_uint32_t * xut_lines);

    /**
     * Callback prototype to allocate and lock a picture buffer.
     */
    static x_pvoid_t video_lock_cbk(x_pvoid_t xpvt_context,
                                    x_pvoid_t * xpvt_planes);

    /**
     * Callback prototype to unlock a picture buffer.
     */
    static x_void_t video_unlock_cbk(x_pvoid_t xpvt_context,
                                     x_pvoid_t xpvt_picture,
                                     x_pvoid_t const * xpvt_planes);

    /**
     * Callback prototype to display a picture.
     */
    static x_void_t video_display_cbk(x_pvoid_t xpvt_context,
                                      x_pvoid_t xpvt_picture);

    /**
     * Callback prototype to setup the audio playback.
     * This is called when the media player needs to create a new audio output.
     * \param xpvt_context pointer to the data pointer passed to libvlc_audio_set_callbacks() [IN/OUT]
     * \param xct_format 4 bytes sample format [IN/OUT]
     * \param rate sample rate [IN/OUT]
     * \param channels channels count [IN/OUT]
     * \return 0 on success, anything else to skip audio playback
     */
    static x_int32_t audio_setup_cbk(x_pvoid_t * xpvt_context,
                                     x_char_t * xct_format,
                                     x_uint32_t * xut_rate,
                                     x_uint32_t * xut_channels);

    /**
     * Callback prototype for audio playback cleanup.
     * This is called when the media player no longer needs an audio output.
     * \param xpvt_context data pointer as passed to libvlc_audio_set_callbacks() [IN]
     */
    static x_void_t audio_cleanup_cbk(x_pvoid_t xpvt_context);

    /**
     * Callback prototype for audio playback.
     * \param xpvt_context data pointer as passed to libvlc_audio_set_callbacks() [IN]
     * \param xpvt_samples pointer to the first audio sample to play back [IN]
     * \param xut_count number of audio samples to play back
     * \param xit_pts expected play time stamp (see libvlc_delay())
     */
    static x_void_t audio_play_cbk(x_pvoid_t xpvt_context,
                                   const x_pvoid_t xpvt_samples,
                                   x_uint32_t xut_count,
                                   x_int64_t xit_pts);

    /**
     * Callback prototype for audio pause.
     * \note The pause callback is never called if the audio is already paused.
     * \param xpvt_context data pointer as passed to libvlc_audio_set_callbacks() [IN]
     * \param xit_pts time stamp of the pause request (should be elapsed already)
     */
    static x_void_t audio_pause_cbk(x_pvoid_t xpvt_context,
                                    x_int64_t xit_pts);

    /**
     * Callback prototype for audio resumption (i.e. restart from pause).
     * \note The resume callback is never called if the audio is not paused.
     * \param xpvt_context data pointer as passed to libvlc_audio_set_callbacks() [IN]
     * \param xit_pts time stamp of the resumption request (should be elapsed already)
     */
    static x_void_t audio_resume_cbk(x_pvoid_t xpvt_context,
                                     x_int64_t xit_pts);

    /**
     * Callback prototype for audio buffer flush
     * (i.e. discard all pending buffers and stop playback as soon as possible).
     * \param xpvt_context data pointer as passed to libvlc_audio_set_callbacks() [IN]
     */
    static x_void_t audio_flush_cbk(x_pvoid_t xpvt_context,
                                    x_int64_t xit_pts);

    /**
     * Callback prototype for audio buffer drain
     * (i.e. wait for pending buffers to be played).
     * \param xpvt_context data pointer as passed to libvlc_audio_set_callbacks() [IN]
     */
    static x_void_t audio_drain_cbk(x_pvoid_t xpvt_context);

    /**
     * Callback prototype for audio volume change.
     * \param xpvt_context data pointer as passed to libvlc_audio_set_callbacks() [IN]
     * \param xft_volume software volume (1. = nominal, 0. = mute)
     * \param xct_muted muted flag
     */
    static x_void_t audio_set_volume_cbk(x_pvoid_t xpvt_context,
                                         x_float_t xft_volume,
                                         x_uchar_t xct_muted);

    /**
     * Callback function notification.
     */
    static x_void_t handle_event_cbk(x_handle_t xht_event,
                                     x_pvoid_t xpvt_context);

	// common data members
private:
    static x_handle_t ms_xht_vlc_instance; ///< the global libvlc instance.

    // constructor/destructor
public:
    explicit vlc_mgrabber_t(void);
    ~vlc_mgrabber_t(void);

    // public interfaces
public:
    /**********************************************************/
    /**
     * @brief 设置回调接口。
     *
     * @param [in ] xfunc_vptr   : 视频图像的回调函数接口。
     * @param [in ] xfunc_aptr   : 音频数据的回调函数接口。
     * @param [in ] xfunc_eptr   : 操作事件的回调函数接口。
     * @param [in ] xpvt_context : 回调的用户上下文描述信息。
     */
    inline x_void_t set_callback(xfunc_video_cbk_t xfunc_vptr,
                                 xfunc_audio_cbk_t xfunc_aptr,
                                 xfunc_event_cbk_t xfunc_eptr,
                                 x_pvoid_t xpvt_context)
    {
        m_xfunc_video_cbk = xfunc_vptr;
        m_xfunc_audio_cbk = xfunc_aptr;
        m_xfunc_event_cbk = xfunc_eptr;
        m_xpvt_xfunc_ctxt = xpvt_context;
    }

	/**********************************************************/
    /**
     * @brief 打开工作流程（操作前请先调用 set_callback() 设置好回调参数）。
     *
     * @param [in ] xht_instance    : 关联的 libvlc 实例句柄（若为 X_NULL，则取全局的实例句柄）。
     * @param [in ] xszt_media_file : 音视频文件路径名。
     * @param [in ] xszt_options    : 附加的参数选项（使用 " :" （空格+':'）进行分隔的字符串集，为 X_NULL 时，则忽略）。
     * @param [in ] xut_video_nbits : 视频回调操作时的 RGB 图像位数（24 或 32）。
     *
     * @return x_int32_t
     *         - 成功，返回 0；
     *         - 失败，返回 错误码。
     */
    x_int32_t open(x_handle_t xht_instance,
                   x_cstring_t xszt_media_file,
                   x_cstring_t xszt_options,
                   x_uint32_t xut_video_nbits);

    /**********************************************************/
    /**
     * @brief 关闭工作流程。
     */
    x_void_t close(void);

    /**********************************************************/
    /**
     * @brief 工作流程是否已经打开。
     */
    x_bool_t is_open(void) const;

    /**********************************************************/
    /**
     * @brief 当前视频图像的 RGB 位数（24 或 32）。
     */
    inline x_uint32_t get_vidoe_nbits(void) const
    {
        return m_xut_video_nbits;
    }

    /**********************************************************/
    /**
     * @brief 当前视频图像宽度。
     */
    inline x_int32_t get_video_width(void) const
    {
        return m_xit_video_width;
    }

    /**********************************************************/
    /**
     * @brief 当前视频图像高度。
     */
    inline x_int32_t get_video_height(void) const
    {
        return m_xit_video_height;
    }

    /**********************************************************/
    /**
     * @brief 当前音频数据的格式。
     */
    inline x_uint32_t get_audio_fourcc(void) const
    {
        return m_xut_audio_fourcc;
    }

    /**********************************************************/
    /**
     * @brief 当前音频数据的采样比特率。
     */
    inline x_uint32_t get_audio_rate(void) const
    {
        return m_xut_audio_rate;
    }

    /**********************************************************/
    /**
     * @brief 当前音频数据的采样通道数。
     */
    inline x_uint32_t get_audio_channels(void) const
    {
        return m_xut_audio_channels;
    }

    /**********************************************************/
    /**
     * @brief 根据当前回调的音频数据格式，获取其对应的采样点的 bit 数；
     *        一般为 8, 16, 24, 32 等。
     * @return x_uint32_t
     *         - 返回 0，表示未知；
     *         - 返回 非 0 值，表示格式可识别。
     */
    inline x_uint32_t get_audio_bits_per_sample(void) const
    {
        return m_xut_audio_bpsamp;
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
    x_bool_t is_playing();

    /**
     * Get the current movie length (in ms).
     *
     * \return the movie length (in ms), or -1 if there is no media.
     */
    x_llong_t get_length();

    /**
     * Get the current movie time (in ms).
     *
     * \return the movie time (in ms), or -1 if there is no media.
     */
    x_llong_t get_time();

    /**
     * Set the movie time (in ms). This has no effect if no media is being played.
     * Not all formats and protocols support this.
     *
     * \param xllt_time the movie time (in ms).
     */
    x_void_t set_time(x_llong_t xllt_time);

    /**
     * Is this media player seekable?
     *
     * \return true if the media player can seek
     *
     * \libvlc_return_bool
     */
    x_bool_t is_seekable();

    /**
     * Get movie position as percentage between 0.0 and 1.0.
     *
     * \return movie position, or -1. in case of error
     */
    x_float_t get_position();

    /**
     * Set movie position as percentage between 0.0 and 1.0. 
     * This has no effect if playback is not enabled.
     * This might not work depending on the underlying input format and protocol.
     *
     * \param xft_pos the position
     */
    x_void_t set_position(x_float_t xft_pos);

    /**
     * Can this media player be paused?
     *
     * \return true if the media player can pause
     *
     * \libvlc_return_bool
     */
    x_bool_t can_pause();

    /**
     * Pause or resume (no effect if there is no media)
     *
     * \param xbt_pause play/resume if X_FALSE, pause if X_TRUE
     * \version LibVLC 1.1.1 or later
     */
    x_void_t set_pause(x_bool_t xbt_pause);

    /**
     * Toggle pause (no effect if there is no media)
     */
    x_void_t pause();

    /**
     * Toggle mute status.
     *
     * \warning Toggling mute atomically is not always possible: On some platforms,
     * other processes can mute the VLC audio playback stream asynchronously. Thus,
     * there is a small race condition where toggling will not work.
     * See also the limitations of audio_set_mute().
     */
    x_void_t audio_toggle_mute();

    /**
     * Get current mute status.
     *
     * \return the mute status (boolean) if defined, -1 if undefined/unapplicable
     */
    x_bool_t audio_get_mute();

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
    x_void_t audio_set_mute(x_bool_t xbt_status);

    /**
     * Get current software audio volume.
     *
     * \return the software volume in percents
     * (0 = mute, 100 = nominal / 0dB)
     */
    x_int32_t audio_get_volume();

    /**
     * Set current software audio volume.
     *
     * \param xit_volume the volume in percents (0 = mute, 100 = 0dB)
     * \return 0 if the volume was set, -1 if it was out of range
     */
    x_int32_t audio_set_volume(x_int32_t xit_volume);

    // inner invoking
protected:
#if 0
    /**********************************************************/
    /**
     * @brief 执行视频信息解析操作，提取其 宽高信息。
     * @note  废弃的接口。@deprecated
     */
    x_bool_t video_parse_wh(x_handle_t xht_vlc_mplayer,
                            x_int32_t & xit_width,
                            x_int32_t & xit_height);

    /**********************************************************/
    /**
     * @brief 执行视频信息解析操作，提取其 宽高信息;
     *        取代 video_parse_wh() 接口。
     * @note  废弃的接口。@deprecated
     */
    x_bool_t video_parse_wh_2(x_handle_t xht_vlc_mplayer,
                              x_int32_t & xit_width,
                              x_int32_t & xit_height);
#endif

    /**********************************************************/
    /**
     * @brief 视频格式回调接口。
     */
    x_uint32_t video_format(x_char_t * xct_chroma,
                            x_uint32_t * xut_width,
                            x_uint32_t * xut_height,
                            x_uint32_t * xut_pitches,
                            x_uint32_t * xut_lines);

    /**********************************************************/
    /**
     * @brief 视频播放回调接口 - 图像解码前的加锁操作（获取图像解码缓冲区）。
     */
    x_pvoid_t video_lock(x_pvoid_t * xpvt_planes);

    /**********************************************************/
    /**
     * @brief 视频播放回调接口 - 图像解码后的解锁操作。
     */
    x_void_t video_unlock(x_pvoid_t xpvt_picture, x_pvoid_t const * xpvt_planes);

    /**********************************************************/
    /**
     * @brief 视频播放回调接口 - 图像解码后的显示操作。
     */
    x_void_t video_display(x_pvoid_t xpvt_picture);

    /**********************************************************/
    /**
     * Callback prototype to setup the audio playback.
     * This is called when the media player needs to create a new audio output.
     * \param xct_format 4 bytes sample format [IN/OUT]
     * \param rate sample rate [IN/OUT]
     * \param channels channels count [IN/OUT]
     * \return 0 on success, anything else to skip audio playback
     */
    x_int32_t audio_setup(x_char_t * xct_format,
                          x_uint32_t * xut_rate,
                          x_uint32_t * xut_channels);

    /**********************************************************/
    /**
     * Callback prototype for audio playback cleanup.
     * This is called when the media player no longer needs an audio output.
     */
    x_void_t audio_cleanup();

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
     */
    x_void_t real_audio_callback(x_int32_t xit_ptype,
                                 const x_pvoid_t xpvt_dptr,
                                 x_uint32_t xut_size,
                                 x_int64_t xit_pts);

    /**********************************************************/
    /**
     * @brief 事件回调处理接口。
     */
    x_void_t handle_event(x_handle_t xht_event);

    // data members
protected:
    //======================================

    x_handle_t         m_xht_vlc_instance; ///< 关联的 libvlc 的实例句柄
    x_handle_t         m_xht_vlc_mplayer;  ///< VLC 播放器对象句柄

    xfunc_video_cbk_t  m_xfunc_video_cbk;  ///< 视频回调接口
    xfunc_audio_cbk_t  m_xfunc_audio_cbk;  ///< 音频回调接口
    xfunc_event_cbk_t  m_xfunc_event_cbk;  ///< 事件回调接口
    x_pvoid_t          m_xpvt_xfunc_ctxt;  ///< 回调的上下文

    //======================================

    typedef x_video_callback_data_t x_vcbk_data_t;

    x_uint32_t     m_xut_video_nbits;  ///< 视频图像的像素位数（24 或 32）
    x_int32_t      m_xit_video_pitch;  ///< 视频图像解码缓存的行步长（字节数）
    x_int32_t      m_xit_video_width;  ///< 视频图像的宽度
    x_int32_t      m_xit_video_height; ///< 视频图像的高度
    x_vcbk_data_t  m_xdt_video_data;   ///< 视频回调时的传递参数

    //======================================

    x_uint32_t     m_xut_audio_fourcc;    ///< 音频数据的采样格式（FOURCC 宏生成的值）
    x_uint32_t     m_xut_audio_bpsamp;    ///< 音频数据格式对应的采样点的 bit 数
    x_uint32_t     m_xut_audio_rate;      ///< 音频数据的采样比特率
    x_uint32_t     m_xut_audio_channels;  ///< 音频数据的采样通道数量

    //======================================
};

////////////////////////////////////////////////////////////////////////////////

#endif // __VLC_MGRABBER_H__
