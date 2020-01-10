CXX=g++
FLAGS=--std=c++14 -Wall 
LIBS=-I overrides

# This is an utterly terrifying makefile, yes
settings_test: settings.cpp overrides/*
	$(CXX) $(FLAGS) $(LIBS) settings.cpp overrides/serial_console.cpp ../console.cpp -o settings_test
