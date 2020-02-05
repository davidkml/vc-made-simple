# Makefile written based on generic template shared by Hilton Lipschitz (https://hiltmon.com)

CC = g++
BUILDDIR = build
SRCDIR = src
TARGETDIR = bin

TARGET = bin/vms

SRCEXT = cpp
SOURCES = $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

CFLAGS = -Wall -g 
LIB = -lboost_iostreams -lboost_serialization -std=c++11
# Don't forget to add dependencies on headers
$(TARGET): $(OBJECTS)
	@echo "Linking..."
	mkdir -p $(TARGETDIR)
	$(CC) $(CFLAGS) $(LIB) $^ -o $@
	

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning...";
	rm -r $(BUILDDIR) $(TARGET)

.PHONY: clean