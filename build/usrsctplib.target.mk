# This file is generated by gyp; do not edit.

TOOLSET := target
TARGET := usrsctplib
DEFS_Debug := \
	'-DSCTP_PROCESS_LEVEL_LOCKS' \
	'-DSCTP_SIMPLE_ALLOCATOR' \
	'-D__Userspace__' \
	'-DINET=1' \
	'-DINET6=1' \
	'-DNODE_GYP_MODULE_NAME=usrsctplib' \
	'-D_LARGEFILE_SOURCE' \
	'-D_FILE_OFFSET_BITS=64' \
	'-D__Userspace_os_Linux' \
	'-DNON_WINDOWS_DEFINE' \
	'-DDEBUG' \
	'-D_DEBUG'

# Flags passed to all source files.
CFLAGS_Debug := \
	-fPIC \
	-pthread \
	-Wextra \
	-Wno-unused-parameter \
	-m64 \
	-w \
	-g \
	-O0

# Flags passed to only C files.
CFLAGS_C_Debug :=

# Flags passed to only C++ files.
CFLAGS_CC_Debug := \
	-fno-rtti \
	-fno-exceptions

INCS_Debug := \
	-I$(srcdir)/usrsctp/usrsctplib \
	-I$(srcdir)/usrsctp/usrsctplib/netinet \
	-I/home/swordfeng/.node-gyp/0.12.5/src \
	-I/home/swordfeng/.node-gyp/0.12.5/deps/uv/include \
	-I/home/swordfeng/.node-gyp/0.12.5/deps/v8/include

DEFS_Release := \
	'-DSCTP_PROCESS_LEVEL_LOCKS' \
	'-DSCTP_SIMPLE_ALLOCATOR' \
	'-D__Userspace__' \
	'-DINET=1' \
	'-DINET6=1' \
	'-DNODE_GYP_MODULE_NAME=usrsctplib' \
	'-D_LARGEFILE_SOURCE' \
	'-D_FILE_OFFSET_BITS=64' \
	'-D__Userspace_os_Linux' \
	'-DNON_WINDOWS_DEFINE'

# Flags passed to all source files.
CFLAGS_Release := \
	-fPIC \
	-pthread \
	-Wextra \
	-Wno-unused-parameter \
	-m64 \
	-w \
	-O3 \
	-ffunction-sections \
	-fdata-sections \
	-fno-tree-vrp \
	-fno-omit-frame-pointer

# Flags passed to only C files.
CFLAGS_C_Release :=

# Flags passed to only C++ files.
CFLAGS_CC_Release := \
	-fno-rtti \
	-fno-exceptions

INCS_Release := \
	-I$(srcdir)/usrsctp/usrsctplib \
	-I$(srcdir)/usrsctp/usrsctplib/netinet \
	-I/home/swordfeng/.node-gyp/0.12.5/src \
	-I/home/swordfeng/.node-gyp/0.12.5/deps/uv/include \
	-I/home/swordfeng/.node-gyp/0.12.5/deps/v8/include

OBJS := \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_asconf.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_auth.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_bsd_addr.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_callout.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_cc_functions.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_crc32.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_indata.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_input.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_output.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_pcb.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_peeloff.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_sha1.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_ss_functions.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_sysctl.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_timer.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_userspace.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctp_usrreq.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet/sctputil.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/netinet6/sctp6_usrreq.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/user_environment.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/user_mbuf.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/user_recv_thread.o \
	$(obj).target/$(TARGET)/usrsctp/usrsctplib/user_socket.o

# Add to the list of files we specially track dependencies for.
all_deps += $(OBJS)

# CFLAGS et al overrides must be target-local.
# See "Target-specific Variable Values" in the GNU Make manual.
$(OBJS): TOOLSET := $(TOOLSET)
$(OBJS): GYP_CFLAGS := $(DEFS_$(BUILDTYPE)) $(INCS_$(BUILDTYPE))  $(CFLAGS_$(BUILDTYPE)) $(CFLAGS_C_$(BUILDTYPE))
$(OBJS): GYP_CXXFLAGS := $(DEFS_$(BUILDTYPE)) $(INCS_$(BUILDTYPE))  $(CFLAGS_$(BUILDTYPE)) $(CFLAGS_CC_$(BUILDTYPE))

# Suffix rules, putting all outputs into $(obj).

$(obj).$(TOOLSET)/$(TARGET)/%.o: $(srcdir)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)

# Try building from generated source, too.

$(obj).$(TOOLSET)/$(TARGET)/%.o: $(obj).$(TOOLSET)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)

$(obj).$(TOOLSET)/$(TARGET)/%.o: $(obj)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)

# End of this set of suffix rules
### Rules for final target.
LDFLAGS_Debug := \
	-pthread \
	-rdynamic \
	-m64

LDFLAGS_Release := \
	-pthread \
	-rdynamic \
	-m64

LIBS :=

$(obj).target/usrsctplib.a: GYP_LDFLAGS := $(LDFLAGS_$(BUILDTYPE))
$(obj).target/usrsctplib.a: LIBS := $(LIBS)
$(obj).target/usrsctplib.a: TOOLSET := $(TOOLSET)
$(obj).target/usrsctplib.a: $(OBJS) FORCE_DO_CMD
	$(call do_cmd,alink)

all_deps += $(obj).target/usrsctplib.a
# Add target alias
.PHONY: usrsctplib
usrsctplib: $(obj).target/usrsctplib.a

# Add target alias to "all" target.
.PHONY: all
all: usrsctplib

# Add target alias
.PHONY: usrsctplib
usrsctplib: $(builddir)/usrsctplib.a

# Copy this to the static library output path.
$(builddir)/usrsctplib.a: TOOLSET := $(TOOLSET)
$(builddir)/usrsctplib.a: $(obj).target/usrsctplib.a FORCE_DO_CMD
	$(call do_cmd,copy)

all_deps += $(builddir)/usrsctplib.a
# Short alias for building this static library.
.PHONY: usrsctplib.a
usrsctplib.a: $(obj).target/usrsctplib.a $(builddir)/usrsctplib.a

# Add static library to "all" target.
.PHONY: all
all: $(builddir)/usrsctplib.a

