top="$(CURDIR)"/

OUTDIR:=
DEPDIR=.deps/
SOURCES=$(shell ls *.c)
OBJS=$(subst .c,.o, $(addprefix $(OUTDIR),$(SOURCES)))
DEP_FILES=$(subst .c,.P, $(addprefix $(DEPDIR),$(SOURCES)))
TARGET=$(OUTDIR)writer
CC=gcc
CFLAGS+= -g -Wall
DEFS+=
CPPFLAGS+=
LDFLAGS+= -lpthread
INCLUDES+= -I/usr/lib/jvm/default-java/include
C_COMPILE=$(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)
C_DEPENDENCIES=-Wp,-MD,$(DEPDIR)$(*F).P
C_LINK=$(CC) $(CFLAGS) $(LDFLAGS)

DEPS_MAGIC := $(shell mkdir $(DEPDIR) $(OUTDIR) > /dev/null 2>&1 || :)

all: $(TARGET)

# simply ignore file which doesn't exist. (At first these files don't exist..)
-include $(DEP_FILES)


$(OUTDIR)%.o: %.c
	@echo '$(C_COMPILE) -o $@ -c $<'; \
	$(C_COMPILE) $(C_DEPENDENCIES) -o $@ -c $<

$(TARGET): $(OBJS)
	@echo '$(C_LINK) -o $@ $^'; \
	$(C_LINK) -o  $@ $^

clean:
	rm -f *.mod *.o *.obj .lo $(TARGET)
	rm -rf $(DEPDIR)
	rm -rf $(OUTDIR)
