#######################################################
#######################################################
#######################################################
##########                                   ##########
########## Author:  Roman Ulan               ##########
########## Mail:    roman.ulan@gmail.com     ##########
##########                                   ##########
#######################################################
#######################################################
#######################################################

#######################################################
##########        BEGIN User part            ##########
##########       You can change it           ##########
#######################################################
NAME = libreactor-c.so
SOURCES = src/os_unix.c src/reactor.c
CXX = gcc
CXXFLAGS = -Wall -Werror -pedantic
LDFLAGS = 
LIBS =
INSTALL_BASE_DIR = /usr
#######################################################
##########        END User part              ##########
#######################################################

#######################################################
##########      BEGIN Automation part        ##########
##########     You shouldn't change it       ##########
#######################################################
INSTALL_BIN = $(INSTALL_BASE_DIR)/bin/$(NAME)
UNINSTALL_BIN = $(INSTALL_BASE_DIR)/bin/$(NAME).uninstall
ifeq ($(suffix $(NAME)),.so)
CXXFLAGS += -fPIC -Iinclude
LDFLAGS += -shared
INCLUDES = $(notdir $(wildcard include/*))
INSTALL_BIN = $(INSTALL_BASE_DIR)/lib/$(NAME)
UNINSTALL_BIN = $(INSTALL_BASE_DIR)/lib/$(NAME).uninstall
INSTALL_INC = $(addsuffix .install,$(addprefix $(INSTALL_BASE_DIR)/include/,$(INCLUDES)))
UNINSTALL_INC = $(addsuffix .uninstall,$(addprefix $(INSTALL_BASE_DIR)/include/,$(INCLUDES)))
endif

OBJECTS = $(SOURCES:.c=.o)
LIBNAMES = $(basename $(notdir $(LIBS)))
LIBS_CLEAN = $(addsuffix .clean,$(LIBS))
LDFLAGS += $(addprefix -L,$(dir $(LIBS))) $(addprefix -l,$(LIBNAMES:lib%=%))

.PHONY: all debug tst install uninstall clean clean_all

all: $(NAME)

debug: CXXFLAGS+=-g
debug: $(NAME)

$(NAME): $(LIBS) $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(NAME)

%.o: %.c
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.so:
	make -C $(dir $@)

tst:
	make -C tst/unit-tests/ coverage

install: $(INSTALL_BIN) $(INSTALL_INC)

$(INSTALL_BIN): $(NAME)
	@echo cp $(NAME) $@

%.install:
	@echo cp -r include/$(notdir $(@:.install=)) $(INSTALL_BASE_DIR)/include/

uninstall: $(UNINSTALL_BIN) $(UNINSTALL_INC)

%.uninstall:
	@echo rm -rf $(@:.uninstall=)

clean:
	rm -f $(OBJECTS)
	rm -f $(NAME)

clean-all: $(LIBS_CLEAN) clean
	make -C tst/unit-tests/ clean

%.clean:
	make -C $(dir $@) clean
#######################################################
##########       END Automation part         ##########
#######################################################

