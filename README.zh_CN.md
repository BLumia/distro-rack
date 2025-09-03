# DistroRack - 基于 Qt/QML 的 Distrobox 容器图形界面

[English](README.md) | 中文

|CI|构建状态|CI 构建|
|---|---|---|
|deepin 25|[![deepin 25 CI](https://github.com/BLumia/distro-rack/actions/workflows/build-deb.yml/badge.svg)](https://github.com/BLumia/distro-rack/actions/workflows/build-deb.yml)|[获取最新 CI 构建版](https://nightly.link/BLumia/distro-rack/workflows/build-deb/master)|

DistroRack 是一个功能丰富的图形界面程序，用于在 Linux 上创建和管理 [Distrobox](https://github.com/89luca89/distrobox) 容器，使用 C++/QML 编写。

## 功能特性

> [!NOTE]
> 本项目还很年轻，一些功能仍在开发中。

- Distrobox 容器管理的图形界面
  - 创建、导入、克隆、删除容器
  - 将容器内的应用程序导出/取消导出到宿主操作系统
- 基于 Qt/QML 的图形界面，可选桌面环境集成：
  - DDE
  - KDE
- 自定义终端模拟器支持，开箱即用支持以下终端：
  - deepin-terminal
  - konsole
  - kitty
  - alacritty
  - ...以及更多

## 构建

进入项目根目录后，运行以下命令：

```shell
$ cmake -Bbuild .
$ cmake --build build
```

构建完成后，可执行文件将位于 `/build/distro-rack`。

要生成一个 `.deb` 包，只需 `cd` 到构建目录并使用 `CPack`：

```shell
$ cpack -G DEB
```

## 为什么要为 Distrobox 开发另一个图形界面

本项目深受 [DistroShelf](https://github.com/ranfdev/DistroShelf/) 启发。我最初计划将 DistroShelf 移植到 `deepin 25`，但遗憾的是 DistroShelf 需要相当新版本的 `gtk-rs` 和 `libadwaita`，这需要相当新版本的 `gtk4-sys`，而 `deepin 25` 在近期可能无法提供，因此无法构建。虽然 `flatpak` 在 `deepin` 中可用，但如果终端用户没有良好的网络连接，使用 `flathub` 包会极其困难，特别是对于主要生活在中国大陆的 deepin 用户。

通过为 distrobox 制作基于 Qt 的图形界面，我们带来了在基于 Qt 的桌面环境（本例中主要指 DDE，现也支持 KDE。其他桌面环境则回退到通用的 Qt/QML 界面）上使其看起来更加系统原生的可能性，并使其能够在 `deepin 25` 下通过简单的 `dpkg -i`（或者未来可能通过 `apt install`）直接使用，无需折腾 `flatpak` 镜像或代理。

本项目也是一个实验性项目，用于测试基于 LLM 的工具是否以及如何能够帮助项目开发。我可能会在稍后写一篇关于这个话题的博客文章。

## 支持

[![Afdian](https://static.afdiancdn.com/static/img/logo/logo.png)Afdian | 爱发电](https://afdian.com/a/BLumia)

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/blumia)

## 许可证

DistroRack 整体采用 MIT 许可证。个别文件可能有不同但兼容的许可证。
