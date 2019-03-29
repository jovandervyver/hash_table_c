# define the C compiler to use
CC = gcc

# define any compile-time flags
CFLAGS = -std=c99 -Wall -Wextra -pedantic -Wmissing-prototypes -Wshadow -O3 -flto -fomit-frame-pointer

# define any directories containing header files other than /usr/include
IDIR = include
DEPS = $(wildcard $(IDIR)/*.h)

# add includes
CFLAGS += -I$(IDIR)

# define the C source files
SDIR = src
SRCS = $(wildcard $(SDIR)/*.c)

# Object files
ODIR = obj
OBJS = $(patsubst %,$(ODIR)/%,$(notdir $(SRCS:.c=.o)))

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

default: hash_table_c

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

hash_table_c: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	$(RM) $(ODIR)/*.o hash_table_c *~ core $(INCDIR)/*~
