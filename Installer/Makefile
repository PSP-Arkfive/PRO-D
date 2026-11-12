TARGET = proinstaller
OBJS = main.o kernel.o package.o

CFLAGS = -O2 -G0 -Wall -Wextra -std=c99
BUILD_PRX = 1
ENCRYPT = 1
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PRO Installer
LIBS = -lpspexploit -lpsppower
INCDIR = ../Rebootex_bin

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
