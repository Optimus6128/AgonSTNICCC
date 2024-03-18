# ----------------------------
# Makefile Options
# ----------------------------

NAME = DEMO
DESCRIPTION = "Ag C Toolchain Demo"
COMPRESSED = NO

CFLAGS = -Wall -Wextra -Wno-unused -Oz
CXXFLAGS = -Wall -Wextra -Wno-unused -Oz

# ----------------------------

include $(shell cedev-config --makefile)
