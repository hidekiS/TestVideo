TARGET = TestVideo
 
CXXFLAGS =  -O2 -g -Wall -MMD -MP

INCLUDES = -I/usr/local/gmp-5.0.1/include -I/usr/local/include -I/usr/include 
LIBS = -L/usr/local/gmp-5.0.1/lib -lgmpxx -lgmp -lmp -L/usr/lib -lavformat -lavcodec -lswscale -lavdevice

SRCS = $(wildcard *.cpp)
OBJDIR   = ../Debug/src
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))
HEADS = $(wildcard *.h)

# リンク時にライブラリを追加
$(TARGET): $(OBJS) $(HEADS)
	ccache $(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS) $(INCLUDES)

# ソースをコンパイルするときにインクルードディレクトリを指定
#$(OBJDIR)/%.o: $(SRCS)
#	$(CXX) $(INCLUDES) -c $(SRCS)

$(OBJDIR)/%.o: %.cpp
	ccache $(CXX) $(INCLUDES) -c -o $@ -c $<

all: $(TARGET)
 
run: all
	./$(TARGET)
 
##depend:
#	$(CXX) -MM -MG $(SRCS) > Makefile.depend
#	cat Makefile.depend
 
clean:
	rm -f $(OBJS)  *~ ¥#*¥#

-include Makefile.depend
