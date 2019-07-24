/**
 * @file   rframewidget.h
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 *
 * 文件名称：rframewidget.h
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

#ifndef __RFRAMEWIDGET_H__
#define __RFRAMEWIDGET_H__

#include <QWidget>
#include <QImage>

////////////////////////////////////////////////////////////////////////////////

/**
 * @class RFrameWidget
 * @brief 实时视频帧的图像显示面板控件。
 */
class RFrameWidget : public QWidget
{
    Q_OBJECT

    // constructor/destructor
public:
    explicit RFrameWidget(QWidget *parent = nullptr);
    ~RFrameWidget(void);

    // overrides
protected:
    virtual void paintEvent(QPaintEvent *event);

    // public interfaces
public slots:
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
    bool startRender(int cx, int cy, int nBits);

    /**********************************************************/
    /**
     * @brief 停止渲染工作。
     */
    void stopRender(void);

public:
    /**********************************************************/
    /**
     * @brief 判断是否已经启动了渲染工作。
     */
    inline bool isStart(void) const { return (0 != m_nRgbBits); }

    /**********************************************************/
    /**
     * @brief 提取 QImage 对象用于渲染图像的像素数据填充；完成后填充后，
     *        再调用 push() 接口将 QImage 回收至控件中待显示。
     */
    QImage * pull(void);

    /**********************************************************/
    /**
     * @brief 将完成实渲染图像像素数据填充后的 QImage 对象回收至控件中，
     *        并通知控件的刷新显示操作。
     *
     * @param [in ] image : 回收回来的 QImage 对象。
     * @param [in ] cx    : QImage 对象的有效图像区域宽度。
     * @param [in ] cy    : QImage 对象的有效图像区域高度。
     */
    void push(QImage * image, int cx, int cy);

    /**********************************************************/
    /**
     * @brief 缓存的渲染图像的最大宽度。
     */
    inline int cxImage(void) const { return m_cxImage; }

    /**********************************************************/
    /**
     * @brief 缓存的渲染图像的最大高度。
     */
    inline int cyImage(void) const { return m_cyImage; }

    /**********************************************************/
    /**
     * @brief 缓存的渲染图像的像素位数（RGB 24 或 32）。
     */
    inline int nRgbBits(void) const { return m_nRgbBits; }

    // data members
private:
    int      m_cxImage;     ///< 缓存的渲染图像的最大宽度
    int      m_cyImage;     ///< 缓存的渲染图像的最大宽度
    int      m_nRgbBits;    ///< 缓存的渲染图像的像素位数（RGB 24 或 32）

    QImage * m_imgQueue[2]; ///< 渲染工作使用到的 QImage 对象
                            ///< （只有两个，一个用于控件渲染，另一个用于数据填充）

    uint32_t m_cxyPaint;    ///< 记录当前渲染操作的 QImage 对象的有效图像区域大小（高 16 表示宽度，低 16 位表示高度）
    uint     m_idxPaint;    ///< 记录当前渲染操作的 QImage 对象索引号
    uint     m_idxWrite;    ///< 记录当前填充操作的 QImage 对象索引号
};

////////////////////////////////////////////////////////////////////////////////

#endif // __RFRAMEWIDGET_H__
