
OSL_SRCS := $(wildcard rmantools/pattern/*.osl)
OSL_SHADERS := $(OSL_SRCS:%.osl=%.oso)

PATTERN_SRCS := $(wildcard rmantools/pattern/*.cpp)
PATTERNS := $(PATTERN_SRCS:%.cpp=%.so)


UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	RMSTREEE ?= /opt/pixar/linux/RenderManStudio-19.0-maya2014
    CXXFLAGS += -fPIC
    LDFLAGS += -shared
else ifeq ($(UNAME_S),Darwin)
	RMSTREEE ?= /opt/pixar/macosx/RenderManStudio-19.0-maya2014
    LDFLAGS += -bundle -undefined dynamic_lookup
else
	$(error Must be OSX or LINUX)
endif

RMANTREE ?= $(RMSTREE)/rmantree
CXXFLAGS += -I$(RMANTREE)/include


.PHONY: build patterns clean
.DEFAULT: build

build: patterns shaders

patterns: $(PATTERNS)

shaders: $(OSL_SHADERS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $^ -o $@

%.so: %.o
	$(CXX) $(LDFLAGS) $^ -o $@

%.oso: %.osl
	oslc -o $@ $^

clean:
	- rm -rf build
