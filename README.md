在 vlc_mgrabber.h 和 vlc_mgrabber.cpp 源码文件中的 C++ 类 **vlc_mgrabber_t**，封装了 libvlc 的相关接口，实现 RGB图像 和 PCM音频 数据的攫取功能。

其使用的大致流程，如下描述：
- 1 在程序启动时，调用 vlc_mgrabber_t::startup(0, NULL) 初始化 libvlc 库。
- 2 程序执行过程中，开启一个 vlc_mgrabber 对象 object 的工作流程，需要先后调用如下接口：
  - 2.1 先使用 object.set_callback(...) 设置回调接口；
  - 2.2 然后用 object.open(...) 操作接口打开工作流程；
  - 2.3 期间，用 set_position(...) 设置当前播放进度，audio_set_mute(...) 设置静音状态，audio_set_volume(...) 设置播放音量，或者还可以进行其他的操作（详细请查看源码）；
  - 2.4 另外，libvlc 内部的工作线程，会通过设置的回调函数接口，回调数据（RGB图像 或 PCM 音频数据，以及 通知事件）；
  - 2.5 最后用 object.close(...) 操作接口关闭工作流程。
- 3 程序在退出前，执行 vlc_mgrabber_t::cleanup() 卸载 libvlc 库。

回调接口的设计，请参看 Widget.h、Widget.cpp 中的相关调用流程。

