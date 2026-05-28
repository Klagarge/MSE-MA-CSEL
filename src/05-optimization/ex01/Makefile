# Noms de tes deux exécutables
EXE_BASIC=basic
EXE_OPTI=optimized

# Fichiers sources pour chaque version (à adapter avec tes vrais noms de fichiers)
SRCS_BASIC=basic.c
SRCS_OPTI=optimized.c

ifeq ($(target),)
target=nano
endif

CFLAGS=-Wall -Wextra -g -c -O1 -MD -std=gnu11 -D_GNU_SOURCE

# Configuration Nano
ifeq ($(target),nano)
TOOLCHAIN_PATH=/buildroot/output/host/usr/bin/
TOOLCHAIN=$(TOOLCHAIN_PATH)aarch64-linux-
CFLAGS+=-mcpu=cortex-a53 -funwind-tables
CFLAGS+=-fno-omit-frame-pointer
EXEC_SUFFIX=
endif

# Configuration Host
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

# Génération des listes de fichiers objets (.o) pour chaque programme
OBJS_BASIC = $(addprefix $(OBJDIR)/, $(SRCS_BASIC:.c=.o))
OBJS_OPTI  = $(addprefix $(OBJDIR)/, $(SRCS_OPTI:.c=.o))

# Noms finaux des exécutables en fonction de la cible (nano ou host)
EXEC_BASIC = $(EXE_BASIC)$(EXEC_SUFFIX)
EXEC_OPTI  = $(EXE_OPTI)$(EXEC_SUFFIX)

# --- RÈGLES DE COMPILATION ---

# Règle par défaut : build les deux programmes
all: $(EXEC_BASIC) $(EXEC_OPTI)

# Règles pour build individuellement : "make basic" ou "make opti"
basic: $(EXEC_BASIC)
opti: $(EXEC_OPTI)

# Règle de compilation des .c en .o
# Le "| $(OBJDIR)" signifie que le dossier doit exister, mais que sa date de modification ne force pas la recompilation
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) $< -o $@

# Édition de liens (Linker) pour le programme basique
$(EXEC_BASIC): $(OBJS_BASIC)
	$(LD) $(OBJS_BASIC) $(LDFLAGS) -o $@

# Édition de liens (Linker) pour le programme optimisé
$(EXEC_OPTI): $(OBJS_OPTI)
	$(LD) $(OBJS_OPTI) $(LDFLAGS) -o $@

# Création du dossier d'objets
$(OBJDIR):
	mkdir -p $(OBJDIR)

# --- RÈGLES DE NETTOYAGE ET DUMP ---

clean:
	rm -Rf $(OBJDIR) $(EXEC_BASIC) $(EXEC_OPTI) *~ t_*.txt

clean_all: clean
	rm -Rf .obj $(EXE_BASIC)* $(EXE_OPTI)*

dump_basic: $(EXEC_BASIC)
	$(OBJDUMP) -dS $(EXEC_BASIC) > t_basic.txt

dump_opti: $(EXEC_OPTI)
	$(OBJDUMP) -dS $(EXEC_OPTI) > t_opti.txt

# Inclusion des dépendances générées par l'option -MD
-include $(OBJS_BASIC:.o=.d) $(OBJS_OPTI:.o=.d)

.PHONY: all basic opti clean clean_all dump_basic dump_opti