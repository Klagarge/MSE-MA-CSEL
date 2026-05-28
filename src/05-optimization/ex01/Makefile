EXE_BASIC=basic
EXE_OPTI=optimized

SRCS_BASIC=basic.c
SRCS_OPTI=optimized.c

ifeq ($(target),)
target=nano
endif

CFLAGS=-Wall -Wextra -g -c -O1 -MD -std=gnu11 -D_GNU_SOURCE

ifeq ($(target),nano)
TOOLCHAIN_PATH=/buildroot/output/host/usr/bin/
TOOLCHAIN=$(TOOLCHAIN_PATH)aarch64-linux-
CFLAGS+=-mcpu=cortex-a53 -funwind-tables
CFLAGS+=-fno-omit-frame-pointer
##CFLAGS+=-O2
EXEC_SUFFIX=
endif

ifeq ($(target),host)
TOOLCHAIN=
EXEC_SUFFIX=_h
endif

CC=$(TOOLCHAIN)gcc
LD=$(TOOLCHAIN)gcc
AR=$(TOOLCHAIN)ar
STRIP=$(TOOLCHAIN)strip
OBJDUMP=$(TOOLCHAIN)objdump

OBJDIR=.obj/$(target)

OBJS_BASIC = $(addprefix $(OBJDIR)/, $(SRCS_BASIC:.c=.o))
OBJS_OPTI  = $(addprefix $(OBJDIR)/, $(SRCS_OPTI:.c=.o))

EXEC_BASIC = $(EXE_BASIC)$(EXEC_SUFFIX)
EXEC_OPTI  = $(EXE_OPTI)$(EXEC_SUFFIX)


all: $(EXEC_BASIC) $(EXEC_OPTI)

basic: $(EXEC_BASIC)
opti: $(EXEC_OPTI)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) $< -o $@

$(EXEC_BASIC): $(OBJS_BASIC)
	$(LD) $(OBJS_BASIC) $(LDFLAGS) -o $@

$(EXEC_OPTI): $(OBJS_OPTI)
	$(LD) $(OBJS_OPTI) $(LDFLAGS) -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)


clean:
	rm -Rf $(OBJDIR) $(EXEC_BASIC) $(EXEC_OPTI) *~ t_*.txt

clean_all: clean
	rm -Rf .obj $(EXE_BASIC)* $(EXE_OPTI)*

dump_basic: $(EXEC_BASIC)
	$(OBJDUMP) -dS $(EXEC_BASIC) > t_basic.txt

dump_opti: $(EXEC_OPTI)
	$(OBJDUMP) -dS $(EXEC_OPTI) > t_opti.txt

-include $(OBJS_BASIC:.o=.d) $(OBJS_OPTI:.o=.d)

.PHONY: all basic opti clean clean_all dump_basic dump_opti