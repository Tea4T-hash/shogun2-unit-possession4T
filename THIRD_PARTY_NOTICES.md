# Third-party notices

MinHook source is included in the adapter and shared vendor directories. Its LICENSE.txt files and author notices remain with each copy. Disassembler support code retains its original notices.

First-party mod code and documentation are licensed under the root MIT LICENSE, explicitly approved by the owner on 2026-10-21. This grant excludes vendored code and any third-party/game-derived material. See [license scope and provenance](docs/public-release/PROVENANCE.md). MinHook/HDE retain their own BSD-style terms and notices.

Total War: SHOGUN 2 and its executable/engine, artwork, fonts and game data belong to their respective rights holders. Original game binaries, extracted UI inputs and complete native decoding databases are not included in this source repository. The Test159 public installer ships code and extraction metadata, without Shogun images, unit cards, ability icons, fonts or audio. Setup reads those resources from the player’s owned game into local data files. The voice is cropped locally using Windows Media Foundation. Generated local resources are excluded from Git and source/download packages. Older private preview binaries embedded game resources and remain distinct historical artifacts. Native decoding is analysis and is not a claim of recovered original game source. This project is an unofficial mod and is not an official Creative Assembly or SEGA product.

The v0.32 Workshop pack uses the same local-resource method: its first-party bootstrap reads the original UI script and prepares the game-derived resources locally. The stock script, game images/fonts/audio, debugging tools and private build fixtures are not included in the downloadable pack.

The optional local unit-card reader uses [bcdec](https://github.com/iOrange/bcdec), Copyright (c) 2022 Sergii Kudlai, under its MIT license option. The unmodified header, complete license and pinned provenance are in `native/vendor/bcdec/`. It decodes player-local DDS artwork; no card artwork is distributed. The SHOGUN2 uniforms table layout was checked against the MIT-licensed [RPFM schemas](https://github.com/Frodo45127/rpfm-schemas), commit 42290ba010661ce3ac426a307fd8d6b3ea0f2c5b; the schema database is not bundled.

## bcdec MIT license

Copyright (c) 2022 Sergii Kudlai

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
