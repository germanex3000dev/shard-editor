// shard - a very small text editor.
//
//   shard FILE   open FILE, or start empty if it does not exist
//   ^S save      ^Q or ^C quit (quit discards unsaved edits)
//
// Plus typing, Enter, Backspace and the arrow keys. Lines wider than the
// screen wrap, so the rows below them can drift out of place.

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  if (argc != 2 || !isatty(0)) { fprintf(stderr, "usage: shard FILE\n"); return 1; }
  std::vector<std::string> L;                              // the file, as lines
  if (FILE* f = fopen(argv[1], "r")) {                     // no file yet: start empty
    char b[4096];
    while (fgets(b, sizeof b, f)) {
      L.emplace_back(b);
      if (L.back().back() == '\n') L.back().pop_back();
    }
    fclose(f);
  }
  if (L.empty()) L.emplace_back();                         // always one line
  termios s; tcgetattr(0, &s);
  termios t = s;
  t.c_iflag &= ~IXON;                                     // keep ^S/^Q, not XON/XOFF
  t.c_lflag &= ~(ICANON | ECHO | ISIG);                   // one key per read, no echo
  t.c_cc[VMIN] = 1;
  tcsetattr(0, TCSANOW, &t);
  size_t y = 0, x = 0;                                     // cursor row, column
  bool run = true;
  while (run) {
    winsize w{}; int rows = ioctl(1, TIOCGWINSZ, &w) || !w.ws_row ? 24 : w.ws_row;
    size_t top = y + 2 < size_t(rows) ? 0 : y - rows + 3;  // keep cursor in window
    size_t end = top + rows - 1 < L.size() ? top + rows - 1 : L.size();
    printf("\x1b[H\x1b[2J");                               // home, erase, repaint
    for (size_t i = top; i < end; i++) printf("%s\r\n", L[i].c_str());
    printf("\x1b[%d;%dH", int(y - top) + 1, int(x) + 1);   // put the cursor back
    fflush(stdout);
    char k, q[2] = {};
    if (read(0, &k, 1) != 1) break;                        // ESC [ A..D is an arrow
    if (k == 27 && read(0, q, 2) == 2) k = q[1];
    switch (k) {
      case 19: {                                            // ^S: write it all back
        FILE* f = fopen(argv[1], "w");
        if (f) { for (auto& l : L) fprintf(f, "%s\n", l.c_str()); fclose(f); }
        else fprintf(stderr, "shard: cannot write %s\n", argv[1]);
        break;
      }
      case 17: case 3: run = false; break;                 // ^Q, ^C
      case 'A': if (y) y--; break;                         // up
      case 'B': if (y + 1 < L.size()) y++; break;           // down
      case 'C': if (x < L[y].size()) x++; else if (y + 1 < L.size()) y++, x = 0; break;
      case 'D': if (x) x--; else if (y) y--, x = L[y].size(); break;
      case '\r': case '\n': {                              // split the line here
        std::string tail = L[y].substr(x);
        L[y].resize(x);
        L.insert(L.begin() + y + 1, tail);
        y++; x = 0;
        break;
      }
      case 8: case 127:                                    // backspace
        if (x) L[y].erase(--x, 1);
        else if (y) x = L[--y].size(), L[y] += L[y + 1], L.erase(L.begin() + y + 1);
        break;
      default: if (k >= ' ' && k < 127) L[y].insert(x++, 1, k);
    }
    if (x > L[y].size()) x = L[y].size();                  // keep x in range
  }
  tcsetattr(0, TCSANOW, &s);                               // restore the terminal
}