/**
 * @file   widget.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 *
 * 文件名称：widget.h
 * 创建日期：2019年07月18日
 * 文件标识：
 * 文件摘要：程序主窗口。
 *
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年07月18日
 * 版本摘要：
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#ifndef WIDGET_H
#define WIDGET_H

#include "vlc_mgrabber.h"
#include "wave_file.h"

#include <QWidget>
#include <QTimer>

////////////////////////////////////////////////////////////////////////////////

namespace Ui
{
    class Widget;
}

/**
 * @class Widget
 * @brief 程序主窗口类。
 */
class Widget : public QWidget
{
    Q_OBJECT

    // common data types
protected:
    typedef vlc_mgrabber_t::x_video_callback_data_t x_vcbk_data_t;

    // common invoking
protected:
    /**********************************************************/
    /**
     * @brief Callback function for video.
     *
     * @param [in    ] xit_ptype : callback prototype.
     * @param [in,out] xvct_dptr : interactive data for callback operations.
     * @param [in    ] xpvt_ctxt : the user context description.
     */
    static x_void_t real_video_callback(x_int32_t xit_ptype,
                                        x_vcbk_data_t * xvct_dptr,
                                        x_pvoid_t xpvt_ctxt)
    {
        static_cast< Widget *>(xpvt_ctxt)->real_video_cbk(xit_ptype, xvct_dptr);
    }

    /**********************************************************/
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
    static x_void_t real_audio_callback(x_int32_t xit_ptype,
                                        x_pvoid_t xpvt_dptr,
                                        x_uint32_t xut_size,
                                        x_int64_t xit_pts,
                                        x_pvoid_t xpvt_ctxt)
    {
        static_cast< Widget *>(xpvt_ctxt)->real_audio_cbk(
                                        xit_ptype,
                                        xpvt_dptr,
                                        xut_size,
                                        xit_pts);
    }

    /**********************************************************/
    /**
     * @brief Callback function for event.
     *
     * @param [in ] xit_event   : the callback event code.
     * @param [in ] xlpt_param1 : param1.
     * @param [in ] xlpt_param2 : param2.
     * @param [in ] xpvt_ctxt   : the user context description.
     */
    static x_void_t real_event_callback(x_int32_t xit_event,
                                        x_lptr_t xlpt_param1,
                                        x_lptr_t xlpt_param2,
                                        x_pvoid_t xpvt_ctxt)
    {
        static_cast< Widget *>(xpvt_ctxt)->real_event_cbk(
                                        xit_event,
                                        xlpt_param1,
                                        xlpt_param2);
    }

    // constructor/destructor
public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget(void);

    // overrides
protected:
    virtual bool eventFilter(QObject * watched, QEvent * event);
    virtual void closeEvent(QCloseEvent * event);

    // inner invoking
protected:
    void on_slider_mouseLButtonPress(QObject * slider, QEvent * event);

    /**********************************************************/
    /**
     * @brief Callback function for video.
     *
     * @param [in    ] xit_ptype : callback prototype.
     * @param [in,out] xvct_dptr : interactive data for callback operations.
     */
    x_void_t real_video_cbk(x_int32_t xit_ptype, x_vcbk_data_t * xvct_dptr);

    /**********************************************************/
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
     */
    x_void_t real_audio_cbk(x_int32_t xit_ptype,
                            x_pvoid_t xpvt_dptr,
                            x_uint32_t xut_size,
                            x_int64_t xit_pts);

    /**********************************************************/
    /**
     * @brief Callback function for event.
     *
     * @param [in ] xit_event   : the callback event code.
     * @param [in ] xlpt_param1 : param1.
     * @param [in ] xlpt_param2 : param2.
     */
    x_void_t real_event_cbk(x_int32_t xit_event,
                            x_lptr_t xlpt_param1,
                            x_lptr_t xlpt_param2);

    // signals
Q_SIGNALS:
    void real_start_render(int cx, int cy, int nBits);
    void real_stop_render();
    void real_end_reached();

    // slots
private slots:
    void on_pushButton_file_clicked();
    void on_pushButton_audioFile_clicked();
    void on_pushButton_play_clicked();
    void on_pushButton_pause_clicked();
    void on_pushButton_stop_clicked();
    void on_hSlider_duration_valueChanged(int value);
    void on_comboBox_rate_currentIndexChanged(const QString &arg1);
    void on_hSlider_volume_valueChanged(int value);
    void on_checkBox_mute_stateChanged(int arg1);
    void on_timer_idle();

    // data members
private:
    Ui::Widget   * ui;
    QTimer         m_timerIdle;

    vlc_mgrabber_t        m_xvlc_mgrabber;
    x_wave_file_writer_t  m_wfile_writer;
};

////////////////////////////////////////////////////////////////////////////////

#endif // WIDGET_H
