NAME := KSWriteAOV
LIB := $(NAME).so

RMSTREEE := /opt/pixar/macosx/RenderManStudio-19.0-maya2014
RMANTREE := $(RMSTREE)/rmantree

CXXFLAGS := -I$(RMANTREE)/include
LDFLAGS :=

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CXXFLAGS += -fPIC
    LDFLAGS += -shared
else ifeq ($(UNAME_S),Darwin)
    LDFLAGS += -bundle -undefined dynamic_lookup
else
	$(error Must be OSX or LINUX)
endif


.PHONY: lib clean
.DEFAULT: lib


lib: $(LIB)

build/%.o: %.cpp
	@ mkdir -p $(dir $@)
	$(CXX) -c $(CXXFLAGS) $^ -o $@

%.so: build/%.o
	$(CXX) $(LDFLAGS) $^ -o $@

clean:
	- rm $(LIB)
	- rm -rf build
