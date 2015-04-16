
OSL_SRCS := $(wildcard pattern/*.osl)
OSL_SHADERS := $(OSL_SRCS:pattern/%.osl=build/%.oso)

PATTERN_SRCS := $(wildcard pattern/*.cpp)
PATTERNS := $(PATTERN_SRCS:pattern/%.cpp=build/%.so)


UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	RMSTREEE := /opt/pixar/linux/RenderManStudio-19.0-maya2014
    CXXFLAGS += -fPIC
    LDFLAGS += -shared
else ifeq ($(UNAME_S),Darwin)
	RMSTREEE := /opt/pixar/macosx/RenderManStudio-19.0-maya2014
    LDFLAGS += -bundle -undefined dynamic_lookup
else
	$(error Must be OSX or LINUX)
endif

RMANTREE := $(RMSTREE)/rmantree
CXXFLAGS += -I$(RMANTREE)/include


.PHONY: build patterns clean
.DEFAULT: build

build: patterns shaders

patterns: $(PATTERNS)

shaders: $(OSL_SHADERS)

build/%.o: pattern/%.cpp
	@ mkdir -p $(dir $@)
	$(CXX) -c $(CXXFLAGS) $^ -o $@

build/%.so: build/%.o
	$(CXX) $(LDFLAGS) $^ -o $@

build/%.oso: pattern/%.osl
	oslc -o $@ $^

clean:
	- rm -rf build
