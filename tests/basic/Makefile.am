# Copyright 2012 Christian Eder
# 
# This file is part of XMALLOC, licensed under the GNU General Public
# License version 3. See COPYING for more information.

INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/include -I$(top_srcdir)/src -I$(top_builddir)
AM_CPPFLAGS = -g3 -ggdb -Wall -D__XMALLOC_DEBUG -O0 -fomit-frame-pointer -pthread

BENCHMARK_CXXFLAGS = -O2 -pthread

SUBDIRS =

if ENABLE_DEBUG
LDADD = $(top_builddir)/src/.libs/libxmalloc_debug.la
else
LDADD = $(top_builddir)/src/.libs/libxmalloc.la
endif

test_malloc_test_LDFLAGS = -pthread
# Put an entry for your test-mycomponent.C here (or in one of the other test groups).  
# Don't forget to add a test_mycomponent_SOURCES entry below.
#
# Once present, tests should remain present, being moved from one group to another.
# Note that among the available groups are NONCOMPILING_TESTS and FAILING_TESTS, which
# will be ignored by "make check" and can be temporary storage for problematic tests.
# a benchmarker, not to be included in check.

BASIC_TESTS =  															\
				test-spec-bins-small-blocks					\
				test-spec-bins-large-blocks  				\
				test-small-blocks										\
				test-large-blocks										\
				test-malloc-small-blocks						\
				test-malloc-large-blocks						\
				test-Realloc-small-blocks						\
				test-Realloc-large-blocks						\
				test-realloc-small-blocks						\
				test-realloc-large-blocks						\
				test-Realloc-size-small-blocks			\
				test-Realloc-size-large-blocks			\
				test-setting-values									\
				test-t4															\
				test-malloc_test

BENCHMARKS =            

EXTRA_PROGRAMS = $(NON_COMPILING_TESTS) $(BENCHMARKS)

check_PROGRAMS = $(BASIC_TESTS)

test_spec_bins_small_blocks_SOURCES =						\
   	test-spec-bins-small-blocks.c

test_spec_bins_large_blocks_SOURCES =						\
    test-spec-bins-large-blocks.c

test_small_blocks_SOURCES =											\
   test-small-blocks.c

test_large_blocks_SOURCES =											\
   test-large-blocks.c

test_malloc_small_blocks_SOURCES =							\
   test-malloc-small-blocks.c

test_malloc_large_blocks_SOURCES =							\
   test-malloc-large-blocks.c

test_Realloc_small_blocks_SOURCES =							\
   test-Realloc-small-blocks.c

test_Realloc_large_blocks_SOURCES =							\
   test-Realloc-large-blocks.c

test_realloc_small_blocks_SOURCES =							\
   test-realloc-small-blocks.c

test_realloc_large_blocks_SOURCES =							\
 		test-realloc-large-blocks.c

test_Realloc_size_small_blocks_SOURCES =				\
		test-Realloc-size-small-blocks.c

test_Realloc_size_large_blocks_SOURCES =				\
		test-Realloc-size-large-blocks.c

test_setting_values_SOURCES =										\
        test-setting-values.c

test_t4_SOURCES =																\
        test-t4.c

test_malloc_test_SOURCES =											\
        test-malloc_test.c

noinst_HEADERS =	
