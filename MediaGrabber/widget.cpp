/**
 * @file   widget.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 *
 * 文件名称：widget.cpp
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

#include "widget.h"
#include "ui_widget.h"

#include <string>

#include <QMouseEvent>
#include <QByteArray>
#include <QMessageBox>
#include <QFileDialog>

#include <QDebug>

////////////////////////////////////////////////////////////////////////////////

//====================================================================

//
// Widget : constructor/destructor
//

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->hSlider_duration->setRange(0, 0x7FFFFFFF);
    ui->hSlider_duration->setValue(0);
    ui->hSlider_duration->installEventFilter(this);

    ui->hSlider_volume->setRange(0, 100);
    ui->hSlider_volume->setValue(100);
    ui->hSlider_volume->installEventFilter(this);

    ui->label_volumeInfo->setText(tr("100%"));

    connect(this, SIGNAL(real_start_render(int, int, int)),
            ui->widget_render, SLOT(startRender(int, int, int)),
            Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(real_stop_render()),
            ui->widget_render, SLOT(stopRender()),
            Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(real_end_reached()),
            this, SLOT(on_pushButton_stop_clicked()),
            Qt::QueuedConnection);

    connect(&m_timerIdle, &QTimer::timeout, this, &Widget::on_timer_idle);
    m_timerIdle.start(500);
}

Widget::~Widget(void)
{
    if (m_xvlc_mgrabber.is_open())
    {
        m_xvlc_mgrabber.close();
    }

    m_timerIdle.stop();

    delete ui;
}

//====================================================================

//
// Widget : overrides
//

bool Widget::eventFilter(QObject * watched, QEvent * event)
{
    if ((event->type() == QEvent::MouseButtonPress) &&
        ((watched == ui->hSlider_duration) || (watched == ui->hSlider_volume)))
    {
        on_slider_mouseLButtonPress(watched, event);
    }

    return QWidget::eventFilter(watched, event);
}

void Widget::closeEvent(QCloseEvent * event)
{
    if (m_xvlc_mgrabber.is_open())
    {
        m_xvlc_mgrabber.close();
    }

    QWidget::closeEvent(event);
}

//====================================================================

//
// Widget : inner invoking
//

void Widget::on_slider_mouseLButtonPress(QObject * slider, QEvent * event)
{
    do
    {
        //======================================
        // 只处理 鼠标左键 的按下事件

        QSlider     * sliderCtrl = static_cast< QSlider     * >(slider);
        QMouseEvent * mouseEvent = static_cast< QMouseEvent * >(event );
        if ((Qt::LeftButton != mouseEvent->button()) ||
            !sliderCtrl->isEnabled())
        {
            break;
        }

        //======================================
        // 确定控件操作的基本参数

        int cxctl = 0;  // 滑块宽度
        int cxwnd = 0;  // 滑槽长度
        int mxpos = 0;  // 鼠标按下的位置

        if (Qt::Horizontal == sliderCtrl->orientation())
        {
            // 水平样式的 slider
            cxctl = sliderCtrl->minimumSizeHint().width();
            cxwnd = sliderCtrl->width();
            if (sliderCtrl->invertedAppearance())
                mxpos = cxwnd - mouseEvent->x();
            else
                mxpos = mouseEvent->x();
        }
        else
        {
            // 垂直样式的 slider
            cxctl = sliderCtrl->minimumSizeHint().height();
            cxwnd = sliderCtrl->height();
            if (sliderCtrl->invertedAppearance())
                mxpos = mouseEvent->y();
            else
                mxpos = cxwnd - mouseEvent->y();
        }

        if (cxwnd <= cxctl)
        {
            break;
        }

        //======================================
        // 计算结果，并设置新计算得到的 position 值

        int scpos = sliderCtrl->minimum() +
                    (int)((sliderCtrl->maximum() - sliderCtrl->minimum()) *
                          ((mxpos - cxctl / 2.0) / (cxwnd - cxctl)));

        if (sliderCtrl->sliderPosition() == scpos)
        {
            break;
        }

        sliderCtrl->setSliderPosition(scpos);

        //======================================

    } while (0);
}

/**********************************************************/
/**
 * @brief Callback function for video.
 *
 * @param [in    ] xit_ptype : callback prototype.
 * @param [in,out] xvct_dptr : interactive data for callback operations.
 */
x_void_t Widget::real_video_cbk(x_int32_t xit_ptype, x_vcbk_data_t * xvct_dptr)
{
    switch (xit_ptype)
    {
        // FORMAT 回调时，初始化图像渲染的显示控件,
        // 要注意的是，以 xvct_dptr->xit_height + 16 保证开辟的图像输出缓存足够大
    case vlc_mgrabber_t::VIDEO_CALLBACK_FORMAT:
        if (!ui->widget_render->isStart())
        {
            xvct_dptr->xit_nbits = 32;
#if 0
            ui->widget_render->startRender(xvct_dptr->xit_width,
                                           xvct_dptr->xit_height + 32,
                                           32);
#else
            emit real_start_render(xvct_dptr->xit_width,
                                   xvct_dptr->xit_height + 32,
                                   32);
#endif
        }
        else if ((xvct_dptr->xit_width  > ui->widget_render->cxImage()) ||
                 (xvct_dptr->xit_height > ui->widget_render->cyImage()))
        {
            xvct_dptr->xit_nbits = 32;
#if 0
            ui->widget_render->stopRender();
            ui->widget_render->startRender(xvct_dptr->xit_width,
                                           xvct_dptr->xit_height + 32,
                                           32);
#else
            emit real_stop_render();
            emit real_start_render(xvct_dptr->xit_width,
                                   xvct_dptr->xit_height + 32,
                                   32);
#endif
        }
        break;

        // LOCK 回调时，申请图像输出缓存，回写 x_video_callback_data_t 的 xbt_bits_ptr 字段，
        // 同时设置 xht_handle 字段，是为了在 UNLOCK 回调时，知道原来 bits 缓存关联的对象。
    case vlc_mgrabber_t::VIDEO_CALLBACK_LOCK:
        if (ui->widget_render->isStart())
        {
            QImage * ximage_ptr = ui->widget_render->pull();
            xvct_dptr->xht_handle   = (x_handle_t)ximage_ptr;
            xvct_dptr->xbt_bits_ptr = ximage_ptr->bits();
        }
        break;

        // UNLOCK 回调后，将完成解码后得到的图像帧放入渲染控件，通知其刷新显示
    case vlc_mgrabber_t::VIDEO_CALLBACK_UNLOCK:
        if (ui->widget_render->isStart())
        {
            QImage * ximage_ptr = (QImage *)xvct_dptr->xht_handle;
            ui->widget_render->push(ximage_ptr, xvct_dptr->xit_width, xvct_dptr->xit_height);
        }
        break;

    case vlc_mgrabber_t::VIDEO_CALLBACK_DISPLAY:
        {

        }
        break;

    default:
        break;
    }
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
 */
x_void_t Widget::real_audio_cbk(x_int32_t xit_ptype,
                                x_pvoid_t xpvt_dptr,
                                x_uint32_t xut_size,
                                x_int64_t xit_pts)
{
    switch (xit_ptype)
    {
        // FORMAT 回调，是在 open() 操作后进行的格式通知，
        // 借此，可以知道后续回调的 PCM 音频数据的 通道数量、采样率、每个采样位数 这些信息
    case vlc_mgrabber_t::AUDIO_CALLBACK_FORMAT:
        if (!m_wfile_writer.is_open())
        {
            QByteArray  text_file = ui->lineEdit_audioFile->text().toUtf8();
            std::string xstr_file = text_file.data();

            m_wfile_writer.open(xstr_file.c_str(),
                                (x_uint16_t)m_xvlc_mgrabber.get_audio_channels(),
                                m_xvlc_mgrabber.get_audio_rate(),
                                (x_uint16_t)m_xvlc_mgrabber.get_audio_bits_per_sample());
        }
        break;

        // PLAYBACK 回调，这是回调 PCM 音频数据的
    case vlc_mgrabber_t::AUDIO_CALLBACK_PLAYBACK:
        if (m_wfile_writer.is_open())
        {
            m_wfile_writer.write((x_uchar_t *)xpvt_dptr, xut_size);
        }
        break;

    default:
        break;
    }
}

/**********************************************************/
/**
 * @brief Callback function for event.
 *
 * @param [in ] xit_event   : the callback event code.
 * @param [in ] xlpt_param1 : param1.
 * @param [in ] xlpt_param2 : param2.
 */
x_void_t Widget::real_event_cbk(x_int32_t xit_event,
                                x_lptr_t xlpt_param1,
                                x_lptr_t xlpt_param2)
{
    switch (xit_event)
    {
    case vlc_mgrabber_t::EVENT_CALLBACK_END_REACHED:
        {
            // 异步方式，通知播放结束事件
            emit real_end_reached();
        }
        break;

    default:
        break;
    }
}

//====================================================================

//
// Widget : slots
//

void Widget::on_pushButton_file_clicked()
{
    QUrl urlFile = QFileDialog::
            getOpenFileUrl(this, tr("Open a media file"));
    ui->lineEdit_file->setText(urlFile.toString());
}

void Widget::on_pushButton_audioFile_clicked()
{
    QString strFile = QFileDialog::
            getSaveFileName(this,
                            tr("Save a audio file for output"),
                            tr(""),
                            tr("WAVE格式(*.wav)"));
    ui->lineEdit_audioFile->setText(strFile);
}

void Widget::on_pushButton_play_clicked()
{
    //======================================

    QByteArray  text_file = ui->lineEdit_file->text().toUtf8();
    std::string xstr_file = text_file.data();

    if (xstr_file.empty())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("请选择要打开的视频文件！"));
        msgBox.exec();

        ui->lineEdit_file->setFocus();
        return;
    }

    QByteArray  text_option = ui->lineEdit_option->text().toUtf8();
    std::string xstr_option = text_option.data();

    //======================================

    vlc_mgrabber_t::xfunc_audio_cbk_t xfunc_audio_cbk = X_NULL;
    if (ui->checkBox_audioFile->isChecked() &&
        !ui->lineEdit_audioFile->text().isEmpty())
    {
        xfunc_audio_cbk = &Widget::real_audio_callback;
    }

    //======================================

    m_xvlc_mgrabber.set_callback(&Widget::real_video_callback,
                                 xfunc_audio_cbk,
                                 &Widget::real_event_callback,
                                 (x_pvoid_t)this);

    if (0 == m_xvlc_mgrabber.open(X_NULL,
                                  xstr_file.c_str(),
                                  xstr_option.empty() ? X_NULL : xstr_option.c_str()))
    {
        m_xvlc_mgrabber.set_rate(ui->comboBox_rate->currentText().toFloat());
        m_xvlc_mgrabber.audio_set_volume(ui->hSlider_volume->value());
        m_xvlc_mgrabber.audio_set_mute(ui->checkBox_mute->isChecked());
    }

    //======================================
}

void Widget::on_pushButton_pause_clicked()
{
    if (m_xvlc_mgrabber.can_pause())
    {
        m_xvlc_mgrabber.pause();
    }
}

void Widget::on_pushButton_stop_clicked()
{
    m_xvlc_mgrabber.close();
    m_wfile_writer.close();

    ui->widget_render->stopRender();
    ui->widget_render->update();
}

void Widget::on_hSlider_duration_valueChanged(int value)
{
    if (m_xvlc_mgrabber.is_open())
    {
        x_float_t xft_pos =
                (ui->hSlider_duration->value() - ui->hSlider_duration->minimum()) * 1.0F /
                (ui->hSlider_duration->maximum() - ui->hSlider_duration->minimum());
        m_xvlc_mgrabber.set_position(xft_pos);
    }
}

void Widget::on_comboBox_rate_currentIndexChanged(const QString &arg1)
{
    if (m_xvlc_mgrabber.is_open())
    {
        m_xvlc_mgrabber.set_rate(arg1.toFloat());
    }
}

void Widget::on_hSlider_volume_valueChanged(int value)
{
    ui->label_volumeInfo->setText(QString::asprintf("%3d%%", ui->hSlider_volume->value()));

    if (m_xvlc_mgrabber.is_open())
    {
        m_xvlc_mgrabber.audio_set_volume(ui->hSlider_volume->value());
    }
}

void Widget::on_checkBox_mute_stateChanged(int arg1)
{
    m_xvlc_mgrabber.audio_set_mute(ui->checkBox_mute->isChecked());
}

void Widget::on_timer_idle()
{
    bool isOpen  = m_xvlc_mgrabber.is_open();

    //======================================

    if (ui->lineEdit_file->isReadOnly() != isOpen)
        ui->lineEdit_file->setReadOnly(isOpen);

    if (ui->lineEdit_option->isReadOnly() != isOpen)
        ui->lineEdit_option->setReadOnly(isOpen);

    if (ui->pushButton_file->isEnabled() != !isOpen)
        ui->pushButton_file->setEnabled(!isOpen);

    if (ui->hSlider_duration->isEnabled() != isOpen)
        ui->hSlider_duration->setEnabled(isOpen);

    if (ui->pushButton_play->isEnabled() != !isOpen)
        ui->pushButton_play->setEnabled(!isOpen);

    if (ui->pushButton_pause->isEnabled() != isOpen)
        ui->pushButton_pause->setEnabled(isOpen);

    if (ui->pushButton_stop->isEnabled() != isOpen)
        ui->pushButton_stop->setEnabled(isOpen);

    QString strPause = m_xvlc_mgrabber.is_playing() ? tr("暂停") : tr("恢复");
    if (strPause != ui->pushButton_pause->text())
    {
        ui->pushButton_pause->setText(strPause);
    }

    //======================================

    if (ui->checkBox_audioFile->isEnabled() != !isOpen)
        ui->checkBox_audioFile->setEnabled(!isOpen);

    bool isEnabledBtnAfile = ui->checkBox_audioFile->isChecked() && !isOpen;

    if (ui->pushButton_audioFile->isEnabled() != isEnabledBtnAfile)
        ui->pushButton_audioFile->setEnabled(isEnabledBtnAfile);

    //======================================

    x_int32_t xit_tps = 0; // 当前播放进度的时间值
    x_int32_t xit_tls = 0; // 当前播放的时间总长度
    x_int32_t xit_pos = 0; // 计算进度条的滑块位置

    if (isOpen)
    {
        xit_tps = (x_int32_t)m_xvlc_mgrabber.get_time();
        xit_tls = (x_int32_t)m_xvlc_mgrabber.get_length();

        // 计算进度条的滑块位置
        xit_pos = ui->hSlider_duration->minimum() +
                (x_int32_t)((ui->hSlider_duration->maximum() -
                             ui->hSlider_duration->minimum()) *
                            m_xvlc_mgrabber.get_position());
    }

    if (ui->hSlider_duration->sliderPosition() != xit_pos)
    {
        bool isBlocked = ui->hSlider_duration->signalsBlocked();
        ui->hSlider_duration->blockSignals(true);
        ui->hSlider_duration->setValue(xit_pos);
        ui->hSlider_duration->blockSignals(isBlocked);
    }

    //======================================

    xit_tps /= 1000;
    xit_tls /= 1000;

    QString strTime =
            QString::asprintf("[ %02d:%02d:%02d / %02d:%02d:%02d ]",
                              xit_tps / 3600, (xit_tps % 3600) / 60, xit_tps % 60,
                              xit_tls / 3600, (xit_tls % 3600) / 60, xit_tls % 60);
    if (strTime != ui->label_duration->text())
    {
        ui->label_duration->setText(strTime);
    }

    //======================================
}

