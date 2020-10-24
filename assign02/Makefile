# Note: run "make depend" before running "make",
# to ensure that generated source and header files are
# created properly.

C_SRCS = main.c util.c parse.tab.c lex.yy.c grammar_symbols.c node.c treeprint.c value.c
C_OBJS = $(C_SRCS:%.c=%.o)

CXX_SRCS = interp.cpp cpputil.cpp
CXX_OBJS = $(CXX_SRCS:%.cpp=%.o)

CC = gcc
CFLAGS = -g -Wall

CXX = g++
CXXFLAGS = $(CFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -std=c++11 $<

all : interp

interp : $(C_OBJS) $(CXX_OBJS)
	$(CXX) -o $@ $(C_OBJS) $(CXX_OBJS)

parse.tab.c : parse.y
	bison -d parse.y

lex.yy.c : lex.l
	flex lex.l

grammar_symbols.h grammar_symbols.c : parse.y scan_grammar_symbols.rb
	./scan_grammar_symbols.rb < parse.y

clean :
	rm -f interp *.o parse.tab.c lex.yy.c parse.tab.h \
		grammar_symbols.h grammar_symbols.c depend.mak

depend : grammar_symbols.h grammar_symbols.c parse.tab.c lex.yy.c
	$(CC) $(CFLAFGS) -M $(C_SRCS) > depend.mak
	$(CXX) $(CXXFLAGS) -M $(CXX_SRCS) >> depend.mak

depend.mak :
	touch $@

include depend.mak
