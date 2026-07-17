# FireflyPlayer (Firefly)

一款轻量级跨平台多媒体播放器，支持视频、音频、图片播放。

## 特点

- 🎬 **多格式支持**：支持常见的视频、音频、图片格式，采用插件化架构
- 🔍 **文件头识别**：基于文件头识别真实格式，不仅依赖后缀名
- 🎨 **双主题**：深色/浅色主题，支持跟随系统
- 🖼️ **现代UI**：绿 + 米白/深灰配色，平面简约风格
- 🖥️ **高DPI适配**：自动适配各种分辨率和DPI
- ⚡ **轻量高效**：基于 Qt6 开发，体积小巧

## 功能

### 视频/音频播放
- 播放/暂停、进度条（绿色渐变）
- 倍速播放（0.1x - 5x）
- 音量调节，支持最高 500% 音量增强
- 快捷键支持

### 图片查看
- 文件夹内图片左右切换
- 滚轮缩放
- 像素化/平滑两种缩放模式
- 半透明切换按钮

### 系统集成
- 文件关联设置
- 插件化格式支持包
- 系统主题跟随

## 技术栈

- **框架**: Qt 6.6+
- **语言**: C++17
- **构建**: CMake
- **多媒体**: Qt Multimedia
- **CI/CD**: GitHub Actions

## 构建

### 依赖
- Qt 6.6+ (Base, Multimedia, Svg)
- CMake 3.21+
- C++17 编译器

### Linux
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### Windows
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## 快捷键

| 快捷键 | 功能 |
|--------|------|
| Space | 播放/暂停 |
| ← / → | 后退/前进 5秒 |
| Shift + ← / → | 后退/前进 30秒 |
| ↑ / ↓ | 音量±10% |
| M | 静音切换 |
| F / F11 | 全屏切换 |
| Esc | 退出全屏 |

## 许可证

MIT License
