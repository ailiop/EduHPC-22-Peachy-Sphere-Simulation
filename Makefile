# The sources we're building
HEADERS = $(wildcard *.h)
PRODUCT_SOURCES = main.c render.c simulate.c utils/helper.c utils/performance_tester.c
CORRECTNESS_PRODUCT_SOURCES = utils/ref_tester.c

# What we're building
PRODUCT_OBJECTS = $(PRODUCT_SOURCES:.c=.o)
CORRECTNESS_PRODUCT_OBJECTS = $(CORRECTNESS_PRODUCT_SOURCES:.c=.o)
PRODUCT = main
PROFILE_PRODUCT = $(PRODUCT:%=%.prof) #the product, instrumented for gprof
SCALE_PRODUCT = $(PRODUCT)-scale #product for work-span analysis
BENCH_PRODUCT = $(PRODUCT)-benchmark #product for scalability benchmarking
CORRECTNESS_PRODUCT = ref_test #product for generating correctness stats

# What we're building with
OPENCILK_DIR = /opt/opencilk-2
CC := $(OPENCILK_DIR)/bin/clang
CFLAGS = -std=gnu11 -Wall -g -fopencilk
LDFLAGS = -lrt -lm -ldl -lGL -lGLU -lglut -fopencilk

ifeq ($(CILKSAN),1)
  CFLAGS += -fsanitize=cilk -DCILKSAN=1
  LDFLAGS += -fsanitize=cilk
endif

# Determine which profile--debug or release--we should build against, and set
# CFLAGS appropriately.

ifeq ($(DEBUG),1)
  # We want debug mode.
  CFLAGS += -g -Og -gdwarf-3
else
  # We want release mode.
  ifeq ($(CILKSAN),1)
    CFLAGS += -O0 -DNDEBUG
  else
    CFLAGS += -O3 -DNDEBUG
  endif
endif


# By default, make the product.
all:		$(PRODUCT) $(CORRECTNESS_PRODUCT)

# How to build for profiling
prof:		$(PROFILE_PRODUCT)

# Product for work-span analysis
scale:		$(SCALE_PRODUCT)

# Additional product necessary for scalability benchmarking
bench:		$(BENCH_PRODUCT)

# How to clean up
clean:
	$(RM) $(PRODUCT) $(PROFILE_PRODUCT) $(CORRECTNESS_PRODUCT) $(SCALE_PRODUCT) $(BENCH_PRODUCT) *.o *.d *.out framesSimNew.txt framesSimOld.txt framesRenderNew.txt framesRenderOld.txt
	rm -f ./utils/*.o

# How to compile a C file
%.o:		%.c $(HEADERS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<

# How to link the product
$(PRODUCT):	$(PRODUCT_OBJECTS)
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $(PRODUCT_OBJECTS)

# How to build the product, instrumented for profiling
$(PROFILE_PRODUCT): CFLAGS += -DPROFILE_BUILD -pg
$(PROFILE_PRODUCT): LDFLAGS += -pg
$(PROFILE_PRODUCT): $(PRODUCT_OBJECTS)
	$(CC)  $(PRODUCT_OBJECTS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $(PROFILE_PRODUCT)

$(SCALE_PRODUCT): CFLAGS += -fcilktool=cilkscale
$(SCALE_PRODUCT): LDFLAGS += -fcilktool=cilkscale
$(SCALE_PRODUCT): $(PRODUCT_OBJECTS)
	$(CC) $(PRODUCT_OBJECTS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $(SCALE_PRODUCT)

$(BENCH_PRODUCT): CFLAGS += -fcilktool=cilkscale-benchmark
$(BENCH_PRODUCT): LDFLAGS += -fcilktool=cilkscale-benchmark
$(BENCH_PRODUCT): $(PRODUCT_OBJECTS)
	$(CC) $(PRODUCT_OBJECTS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $(BENCH_PRODUCT)

$(CORRECTNESS_PRODUCT): $(CORRECTNESS_PRODUCT_OBJECTS)
	$(CC) $(CORRECTNESS_PRODUCT_OBJECTS) $(LDFLAGS) $(EXTRA_LDFGLAGS) -o $(CORRECTNESS_PRODUCT)
