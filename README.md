# DistroRack - A Qt/QML-based GUI for Distrobox Containers

English | [中文](README.zh_CN.md)

DistroRack is a feature-rich graphical interface for creating and managing [Distrobox](https://github.com/89luca89/distrobox) containers on Linux, written in C++/QML.

## Features

> [!NOTE]
> The project is quite young, some of the features are still WIP.

- GUI for Distrobox container management
  - Create, import, clone, remove containers
  - Export/unexport apps inside containers to your host OS
- Qt/QML based GUI with optional desktop environment integration:
  - DDE
- Custom terminal emulator support with the following ones supported out-of-the-box
  - deepin-terminal
  - konsole
  - kitty
  - alacritty
  - ..and more

## Build

Once you `cd` to the project root, run the following commands:

```shell
$ cmake -Bbuild .
$ cmake --build build
```

Once built, binary will be located at `/build/distro-rack`.

## Why another GUI for Distrobox

This project is heavily inspired by [DistroShelf](https://github.com/ranfdev/DistroShelf/). I originally plan to port DistroShelf to `deepin 25`, but sadly DistroShelf requires a pretty up-to-date version of `gtk-rs` and `libadwaita` which requires a pretty recent version of `gtk4-sys` that `deepin 25` might not be able to ship in the near future, thus impossible to build. While `flatpak` is available in `deepin`, using `flathub` packages are extremely hard if the end-user doesn't have a decent network, especially for deepin users who mostly live in China mainland.

By making a Qt-based GUI for distrobox, we bring the possibility to make it look more system-native on Qt-based DEs (DDE in this case, maybe KDE as well in the future, and fallback to a generic Qt/QML GUI for other DEs), and make it directly usable under `deepin 25` with a simple `dpkg -i` (or maybe `apt install` in the future), without the need of fiddling with `flatpak` mirrors or proxies.

This project is also an experimental project to test if and how the LLM-based tooling could help in project development. I might write a blogpost related to this topic later.

## License

DistroRack as a whole is licensed under MIT license. Individual files may have a different, but compatible license.
