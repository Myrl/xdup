xdup
-
Duplicate X Window

Build
-
> make xdup

Usage
-
> xdup window-id

You can obtain the window-id through xwininfo.

Problems
-
As observed in urxvt and xterm, the xwininfo returns the program's root window, and not the drawing window. Please use `xwininfo -children` to find the appropriate child window.
