# ----------------------------
# Makefile Options
# ----------------------------

NAME = SOLITRCE
ICON = icon.png
DESCRIPTION = "Colection of solitaire games"
COMPRESSED = NO
ARCHIVED = YES

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
