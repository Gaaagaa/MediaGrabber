/**
 * @file   main.cpp
 * <pre>
 * Copyright (c) 2019, Gaaagaa All rights reserved.
 *
 * 文件名称：main.cpp
 * 创建日期：2019年07月19日
 * 文件标识：
 * 文件摘要：测试程序。
 *
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2019年07月19日
 * 版本摘要：
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "widget.h"
#include <QApplication>

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    QApplication xApp(argc, argv);

    vlc_mgrabber_t::startup(0, X_NULL);

    Widget xWidget;
    xWidget.show();

    x_int32_t xit_error = xApp.exec();

    vlc_mgrabber_t::cleanup();

    return xit_error;
}
