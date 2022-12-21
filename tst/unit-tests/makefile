PROD_SRC = ../../src/reactor.c

TST_SRC = tests_reactor.cpp \
	  ../../../googletest/googlemock/src/gmock-all.cc \
	  ../../../googletest/googletest/src/gtest-all.cc \
	  ../../../googletest/googlemock/src/gmock_main.cc

CFLAGS = -Wall -g -I../../include

CXXFLAGS = -Wall -Werror -g \
	   -I../../include \
	   -I../../../C-Mock/include \
	   -I../../../googletest/googlemock/include \
	   -I../../../googletest/googlemock \
	   -I../../../googletest/googletest/include \
	   -I../../../googletest/googletest

LDFLAGS = -lpthread -ldl

OUT = tests_reactor

OBJECTS = $(PROD_SRC:.c=.o)
TST_OBJECTS = $(TST_SRC:.cpp=.o)
OBJECTS += $(TST_OBJECTS:.cc=.o)
OBJECTSGCNO = $(OBJECTS:.o=.gcno)
OBJECTSGCDA = $(OBJECTS:.o=.gcda)
COV = cov.info
HTMLDIR = html

coverage: CFLAGS += -fprofile-arcs -ftest-coverage
coverage: LDFLAGS += -lgcov

all: $(OUT)

run: $(OUT)
	./$(OUT)

coverage: $(COV)
	genhtml --branch-coverage $(COV) -o $(HTMLDIR)

$(COV): run
	lcov --rc lcov_branch_coverage=1 -b . -d $(dir $(PROD_SRC)) -c -o $@

$(OUT): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(CPP_OBJECTS) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OUT) $(OBJECTS) $(OBJECTSGCNO) $(OBJECTSGCDA) $(COV) $(HTMLDIR)

