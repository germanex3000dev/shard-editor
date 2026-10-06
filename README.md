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
