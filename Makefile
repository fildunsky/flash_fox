TARGET = flash_fox
CC = gcc
SDIR = src
LIBS = libusb-1.0 libxml-2.0

ROOT_DIR = $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
CPPFLAGS = -D_GNU_SOURCE -DPIC -DDEBUG
CFLAGS = -O3 -fPIC -Wall -ffunction-sections -fdata-sections
LDFLAGS = -s -Wl,--gc-sections
#CFLAGS = -O0 -g -ggdb -fPIC -Wall -ffunction-sections -fdata-sections
#LDFLAGS = -Wl,--gc-sections

ifneq ($(LIBS),)
  CFLAGS += $(shell pkg-config --cflags $(LIBS))
  LDLIBS = $(shell pkg-config --libs $(LIBS))
endif

OBJDIR = .obj
DEPDIR = .dep
VPATH = $(ROOT_DIR)
SRCS = $(patsubst $(ROOT_DIR)/%,%,$(foreach d,$(SDIR),$(wildcard $(ROOT_DIR)/$(d)/*.c)))
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
DEPS = $(patsubst %.c,$(DEPDIR)/%.d,$(SRCS))

DEPFLAGS = -MT $@ -MD -MP -MF $(DEPDIR)/$*.Td
COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@
LINK.o = $(CC) $(CFLAGS) $(LDFLAGS) -o $@
PRECOMPILE = @mkdir -p $(OBJDIR)/$(dir $*) ; mkdir -p $(DEPDIR)/$(dir $*)
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LINK.o) $^ $(LDLIBS)

$(DEPDIR)/%.d: ;

$(OBJDIR)/%.o: %.c

$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d $(MAKEFILE_LIST)
	$(PRECOMPILE)
	$(COMPILE.c) $<
	$(POSTCOMPILE)

clean:
	$(RM) -rf $(OBJDIR) $(DEPDIR)

distclean: clean
	$(RM) $(TARGET)

.PRECIOUS: $(DEPDIR)/%.d

.PHONY: all clean distclean

-include $(DEPS)

