APP = socod-server

# Add any other object files to this list below
APP_SRC_CPP = $(wildcard *.cpp)
APP_SRC_C = $(wildcard *.c)
APP_SRC_CC = $(wildcard *.cc)
APP_OBJS = $(APP_SRC_CPP:.cpp=.o)
APP_OBJS += $(APP_SRC_C:.c=.o)
APP_OBJS += $(APP_SRC_CC:.cc=.o)
LDLIBS += -lpthread
LDLIBS += -ldl
LDLIBS += -lstdc++
#APP_OBJS = dimex-server.o fpga.o JWrite.o jsmn.outils.o mongoose.o

all: build

build: $(APP)

$(APP): $(APP_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(APP_OBJS) $(LDLIBS)

.PHONY: clean
clean:
	rm -f $(APP_OBJS) $(APP)
