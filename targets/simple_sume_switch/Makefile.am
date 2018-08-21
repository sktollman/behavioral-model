noinst_PROGRAMS = simple_sume_switch
simple_sume_switch_SOURCES = simple_sume_switch.h simple_sume_switch.cpp
simple_sume_switch_LDADD = $(top_builddir)/src/bm_runtime/libbmruntime.la \
$(top_builddir)/src/bm_sim/libbmsim.la \
$(top_builddir)/src/bf_lpm_trie/libbflpmtrie.la \
$(top_builddir)/thrift_src/libruntimestubs.la \
$(top_builddir)/src/BMI/libbmi.la \
$(top_builddir)/third_party/jsoncpp/libjson.la \
-lboost_system $(THRIFT_LIB) -lboost_program_options -lboost_filesystem

if COND_NANOMSG
MAYBE_TESTS = tests
endif

SUBDIRS = . $(MAYBE_TESTS)

libsimplesumeswitch_la_SOURCES = \
simple_sume_switch.cpp \
simple_sume_switch.h

libsimplesumeswitch_runner_la_SOURCES = \
runner.cpp

nobase_include_HEADERS = bm/simple_switch/runner.h

libsimplesumeswitch_la_LIBADD = \
$(top_builddir)/src/bm_sim/libbmsim.la \
$(top_builddir)/src/bf_lpm_trie/libbflpmtrie.la \
$(top_builddir)/src/BMI/libbmi.la \
$(top_builddir)/third_party/jsoncpp/libjson.la \
-lboost_system $(THRIFT_LIB) -lboost_program_options -lboost_filesystem

libsimplesumeswitch_runner_la_LIBADD = \
$(PI_LIB) \
libsimplesumeswitch.la

