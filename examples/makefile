

BBCAT_BASE_CFLAGS=$(shell pkg-config --cflags bbcat-base-0.1 | sed -E 's/\\\"/\"/g')
BBCAT_BASE_LDFLAGS=$(shell pkg-config --libs bbcat-base-0.1)
BBCAT_DSP_CFLAGS=$(shell pkg-config --cflags bbcat-dsp-0.1 | sed -E 's/\\\"/\"/g')
BBCAT_DSP_LDFLAGS=$(shell pkg-config --libs bbcat-dsp-0.1)
BBCAT_CONTROL_CFLAGS=$(shell pkg-config --cflags bbcat-control-0.1 | sed -E 's/\\\"/\"/g')
BBCAT_CONTROL_LDFLAGS=$(shell pkg-config --libs bbcat-control-0.1)
BBCAT_AUDIOOBJECTS_CFLAGS=$(shell pkg-config --cflags bbcat-audioobjects-0.1 | sed -E 's/\\\"/\"/g')
BBCAT_AUDIOOBJECTS_LDFLAGS=$(shell pkg-config --libs bbcat-audioobjects-0.1)

CFLAGS=-Wall -Werror -Wextra -msse2 -O0 -g
LDFLAGS=

CXX = g++
LD = g++

APPLICATIONS=read-adm-bwf write-adm-bwf create-adm map-adm-bwf load-xml write-separate-adm udp-demo play-metadata modify-adm-bwf

all: $(APPLICATIONS)

clean:
	-rm -f $(APPLICATIONS)

%: %.cpp
	$(CXX) $(BBCAT_BASE_CFLAGS) $(BBCAT_DSP_CFLAGS) $(BBCAT_CONTROL_CFLAGS) $(BBCAT_AUDIOOBJECTS_CFLAGS) $(CFLAGS) $< $(LDFLAGS) $(BBCAT_AUDIOOBJECTS_LDFLAGS) $(BBCAT_CONTROL_LDFLAGS) $(BBCAT_DSP_LDFLAGS) $(BBCAT_BASE_LDFLAGS) -o $@
