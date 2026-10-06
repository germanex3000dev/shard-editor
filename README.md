# shard

A very small text editor in C++. One source file, no dependencies.

```
shard file.py
```

Opens `file.py`, or starts empty if it doesn't exist yet.

This editor is intentionally made as minmal as possible, so there is no real "UI" except the file contents.

## Keys

| Key       | Action                       |
| --------- | ---------------------------- |
| typing    | insert text                  |
| Enter     | split the line               |
| Backspace | delete backwards             |
| arrows    | move the cursor              |
| `Ctrl-S`  | save                         |
| `Ctrl-Q`  | quit (unsaved edits are lost) |

The cursor starts at the top of the file. Quitting discards anything you have
not saved. Lines wider than the window wrap instead of scrolling sideways.

## Build

```sh
cmake -B build
cmake --build build
./build/shard file.py
```

## Memory

Measured on Linux x86-64 with gcc 16.2 and glibc 2.44, so treat the figures as
indicative rather than a spec.

While running, resident set size from `/proc`:

| File   | RSS     | vs file size |
| ------ | ------- | ------------ |
| empty  | 3.8 MiB | —            |
| 1 MiB  | 7.0 MiB | 6.7x         |
| 10 MiB | 35.8 MiB | 3.4x        |

The 3.8 MiB floor is almost entirely the dynamic loader, libc and libstdc++,
not the editor. Peak RSS equals RSS, since the buffer is the only thing that
grows.

Buffer memory scales with the number of lines rather than the number of bytes,
because each line is its own `std::string`. For 1 MiB of text:

| Line length | Lines   | RSS     | Over floor |
| ----------- | ------- | ------- | ---------- |
| 10 B        | 104,857 | 7.2 MiB | 3.4 MiB    |
| 100 B       | 10,485  | 5.3 MiB | 1.5 MiB    |
| 400 B       | 2,621   | 5.0 MiB | 1.2 MiB    |

So long lines settle at roughly the file size plus 1.2 MiB, but short lines cost
up to 7x. Keeping the buffer as one string instead would land near 1.1x.

On disk at runtime, from the files actually mapped in `/proc/<pid>/maps`:

| File              | Size       |
| ----------------- | ---------- |
| `libstdc++.so.6`  | 3.3 MiB    |
| `libc.so.6`       | 2.4 MiB    |
| `libm.so.6`       | 1.2 MiB    |
| `ld-linux-x86-64` | 270 KiB    |
| `libgcc_s.so.1`   | 197 KiB    |
| the binary itself | 26 KiB     |
| **total**         | **7.4 MiB** |

The editor is 0.3% of that. The binary alone is 26 KiB at `-O2` (13 KiB of
which is text, data and bss) and 22 KiB stripped.

Building it costs far more than running it:

|                     |                       |
| ------------------- | --------------------- |
| compile time (`-O2`) | 0.6 s (0.7 s with the link) |
| compiler peak memory | 91 MiB                |
| object file         | 19 KiB                |
| CMake build tree    | 372 KiB               |
| g++ on disk         | 388 MiB               |
| g++ + cmake + make  | 479 MiB               |

The toolchain is roughly 1,200 times the size of the program it builds. For the
smallest possible setup, skip CMake and compile by hand:

```sh
g++ -O2 -o shard src/shard.cpp
```

## Nix

```sh
nix build          # ./result/bin/shard
nix run . -- file.py
nix develop        # dev shell with cmake, gcc, gnumake
```

## Notes

Keys are read one byte at a time with the terminal in raw mode, so there is no
curses dependency. `Ctrl-S` and `Ctrl-Q` reach the editor because raw mode
clears `IXON` and `ISIG`.
