# vitaPDF ![Github latest downloads](https://img.shields.io/github/downloads/joel16/vitaPDF/total.svg)

A simple homebrew file browser that is used for viewing various documents on the PlayStation VITA. vitaPDF utilizes various libraries to offer a simple and user friendly expereince.

<p align="center">
<img src="https://i.imgur.com/ynEaaei.png" alt="VITAlbum Screenshot" width="640" height="362"/>
</p>

# Supported Features:
- File browser:
  - Display folders and supported pdf/book formats only.
  - Supports sorting using file name and size.
  - Ability to navigate to multiple devices (ux0:/, ur0:/)
  - Saves last visited directory.
- GUI:
  - Supports dark/light theme modes.
  - Has a toggle for displaying title bar.
- Automatically saves page state via sqlite database. (Moving the pdf/book to another location will lose it's page state at the moment)
- Supports reading the following formats:
  - CBT
  - CBZ
  - EPUB
  - FB2
  - MOBI
  - PDF
  - XPS

# Credits:
- [MuPDF](https://mupdf.com/)
- ocornut and contributors for [upstream imgui](https://github.com/ocornut/imgui)
- PreetiSketch for the LiveArea assets
- [SDL3](https://github.com/libsdl-org/SDL)
- [vitasdk](https://github.com/vitasdk)
