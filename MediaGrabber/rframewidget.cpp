/**
 * @file   rframewidget.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 *
 * 文件名称：rframewidget.cpp
 * 创建日期：2019年07月20日
 * 文件标识：
 * 文件摘要：实时视频帧的图像显示面板控件。
 *
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年07月20日
 * 版本摘要：
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "rframewidget.h"

#include <QPainter>

////////////////////////////////////////////////////////////////////////////////

//====================================================================

//
// RFrameWidget : constructor/destructor
//

RFrameWidget::RFrameWidget(QWidget *parent)
    : QWidget(parent)
    , m_cxImage(0)
    , m_cyImage(0)
    , m_nRgbBits(0)
    , m_cxyPaint(0)
    , m_idxPaint(0)
    , m_idxWrite(0)
{
    m_imgQueue[0] = NULL;
    m_imgQueue[1] = NULL;
}

RFrameWidget::~RFrameWidget(void)
{
    stopRender();
}

//====================================================================

//
// RFrameWidget : overrides
//

void RFrameWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);

    QRect hRect(0, 0, width(), height());

    if (isStart() && (m_idxPaint != m_idxWrite))
    {
        uint32_t cxy = m_cxyPaint;
        int cx = (int)(cxy >> 16);
        int cy = (int)(cxy & 0x0000FFFF);

        if ((cx > 0) && (cy > 0))
        {
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            painter.drawImage(hRect, *m_imgQueue[m_idxPaint % 2], QRect(0, 0, cx, cy));
        }

        m_idxPaint = m_idxWrite;
    }
    else
    {
        painter.fillRect(hRect, QColor(128, 128, 128));
    }
}

//====================================================================

//
// RFrameWidget : public interfaces
//

/**********************************************************/
/**
 * @brief 启动渲染工作。
 *
 * @param [in ] cx    : 缓存的渲染图像的最大宽度。
 * @param [in ] cy    : 缓存的渲染图像的最大高度。
 * @param [in ] nBits : 缓存的渲染图像的像素位数（RGB 24 或 32）。
 *
 * @return bool
 *         - 成功，返回 true；
 *         - 失败，返回 false。
 */
bool RFrameWidget::startRender(int cx, int cy, int nBits)
{
    if (isStart())
    {
        return false;
    }

    if ((cx <= 0) || (cx > 0x00007FFF) ||
        (cy <= 0) || (cy > 0x00007FFF) ||
        ((24 != nBits) && (32 != nBits)))
    {
        return false;
    }

    QImage::Format imgFmt =
            (24 == nBits) ? QImage::Format_RGB888 : QImage::Format_RGB32;

    for (int i = 0; i < 2; ++i)
    {
        m_imgQueue[i] = new QImage(cx, cy, imgFmt);
        assert(NULL != m_imgQueue[i]);
    }

    m_cxyPaint = 0;
    m_idxPaint = 0;
    m_idxWrite = 0;

    m_cxImage  = cx;
    m_cyImage  = cy;
    m_nRgbBits = nBits;

    return true;
}

/**********************************************************/
/**
 * @brief 停止渲染工作。
 */
void RFrameWidget::stopRender(void)
{
    m_cxImage  = 0;
    m_cyImage  = 0;
    m_nRgbBits = 0;

    m_idxPaint = 0;
    m_idxWrite = 0;

    for (int i = 0; i < 2; ++i)
    {
        if (NULL != m_imgQueue[i])
        {
            delete m_imgQueue[i];
            m_imgQueue[i] = NULL;
        }
    }
}

/**********************************************************/
/**
 * @brief 提取 QImage 对象用于渲染图像的像素数据填充；完成后填充后，
 *        再调用 push() 接口将 QImage 回收至控件中待显示。
 */
QImage * RFrameWidget::pull(void)
{
    assert(isStart());
    return m_imgQueue[m_idxWrite % 2];
}

/**********************************************************/
/**
 * @brief 将完成实渲染图像像素数据填充后的 QImage 对象回收至控件中，
 *        并通知控件的刷新显示操作。
 *
 * @param [in ] image : 回收回来的 QImage 对象。
 * @param [in ] cx    : QImage 对象的有效图像区域宽度。
 * @param [in ] cy    : QImage 对象的有效图像区域高度。
 */
void RFrameWidget::push(QImage * image, int cx, int cy)
{
    assert(m_imgQueue[m_idxWrite % 2] == image);
    assert((cx >= 0) && (cx <= cxImage()));
    assert((cy >= 0) && (cy <= cyImage()));

    if (m_idxPaint == m_idxWrite)
    {
        m_cxyPaint = (((uint32_t)cx & 0x0000FFFF) << 16) | ((uint32_t)cy & 0x0000FFFF);
        m_idxWrite += 1;
    }

    update();
}
