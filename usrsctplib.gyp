# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'libsctp_target_type%': 'static_library',
  },
  'target_defaults': {
    'defines': [
      'SCTP_PROCESS_LEVEL_LOCKS',
      'SCTP_SIMPLE_ALLOCATOR',
      '__Userspace__',
      'INET=1',
      'INET6=1',
      'SCTP_DEBUG', # Uncomment for SCTP debugging.
    ],
    'include_dirs': [
      'usrsctp/usrsctplib/',
      'usrsctp/usrsctplib/netinet',
      'usrsctp/usrsctplib/netinet6',
    ],
    'direct_dependent_settings': {
      'include_dirs': [
        'usrsctp/usrsctplib/',
        'usrsctp/usrsctplib/netinet',
        'usrsctp/usrsctplib/netinet6',
      ],
    },
    'conditions': [
    ],
  },
  'targets': [
    {
      'target_name': 'usrsctplib',
      'type': 'static_library',
      'sources': [
        'usrsctp/usrsctplib/netinet/sctp.h',
        'usrsctp/usrsctplib/netinet/sctp_asconf.c',
        'usrsctp/usrsctplib/netinet/sctp_asconf.h',
        'usrsctp/usrsctplib/netinet/sctp_auth.c',
        'usrsctp/usrsctplib/netinet/sctp_auth.h',
        'usrsctp/usrsctplib/netinet/sctp_bsd_addr.c',
        'usrsctp/usrsctplib/netinet/sctp_bsd_addr.h',
        'usrsctp/usrsctplib/netinet/sctp_callout.c',
        'usrsctp/usrsctplib/netinet/sctp_callout.h',
        'usrsctp/usrsctplib/netinet/sctp_cc_functions.c',
        'usrsctp/usrsctplib/netinet/sctp_constants.h',
        'usrsctp/usrsctplib/netinet/sctp_crc32.c',
        'usrsctp/usrsctplib/netinet/sctp_crc32.h',
        'usrsctp/usrsctplib/netinet/sctp_header.h',
        'usrsctp/usrsctplib/netinet/sctp_indata.c',
        'usrsctp/usrsctplib/netinet/sctp_indata.h',
        'usrsctp/usrsctplib/netinet/sctp_input.c',
        'usrsctp/usrsctplib/netinet/sctp_input.h',
        'usrsctp/usrsctplib/netinet/sctp_lock_userspace.h',
        'usrsctp/usrsctplib/netinet/sctp_os.h',
        'usrsctp/usrsctplib/netinet/sctp_os_userspace.h',
        'usrsctp/usrsctplib/netinet/sctp_output.c',
        'usrsctp/usrsctplib/netinet/sctp_output.h',
        'usrsctp/usrsctplib/netinet/sctp_pcb.c',
        'usrsctp/usrsctplib/netinet/sctp_pcb.h',
        'usrsctp/usrsctplib/netinet/sctp_peeloff.c',
        'usrsctp/usrsctplib/netinet/sctp_peeloff.h',
        'usrsctp/usrsctplib/netinet/sctp_process_lock.h',
        'usrsctp/usrsctplib/netinet/sctp_sha1.c',
        'usrsctp/usrsctplib/netinet/sctp_sha1.h',
        'usrsctp/usrsctplib/netinet/sctp_ss_functions.c',
        'usrsctp/usrsctplib/netinet/sctp_structs.h',
        'usrsctp/usrsctplib/netinet/sctp_sysctl.c',
        'usrsctp/usrsctplib/netinet/sctp_sysctl.h',
        'usrsctp/usrsctplib/netinet/sctp_timer.c',
        'usrsctp/usrsctplib/netinet/sctp_timer.h',
        'usrsctp/usrsctplib/netinet/sctp_uio.h',
        'usrsctp/usrsctplib/netinet/sctp_userspace.c',
        'usrsctp/usrsctplib/netinet/sctp_usrreq.c',
        'usrsctp/usrsctplib/netinet/sctp_var.h',
        'usrsctp/usrsctplib/netinet/sctputil.c',
        'usrsctp/usrsctplib/netinet/sctputil.h',
        'usrsctp/usrsctplib/netinet6/sctp6_usrreq.c',
        'usrsctp/usrsctplib/netinet6/sctp6_var.h',
        'usrsctp/usrsctplib/user_atomic.h',
        'usrsctp/usrsctplib/user_environment.c',
        'usrsctp/usrsctplib/user_environment.h',
        'usrsctp/usrsctplib/user_inpcb.h',
        'usrsctp/usrsctplib/user_ip6_var.h',
        'usrsctp/usrsctplib/user_ip_icmp.h',
        'usrsctp/usrsctplib/user_malloc.h',
        'usrsctp/usrsctplib/user_mbuf.c',
        'usrsctp/usrsctplib/user_mbuf.h',
        'usrsctp/usrsctplib/user_queue.h',
        'usrsctp/usrsctplib/user_recv_thread.c',
        'usrsctp/usrsctplib/user_recv_thread.h',
        'usrsctp/usrsctplib/user_route.h',
        'usrsctp/usrsctplib/user_socket.c',
        'usrsctp/usrsctplib/user_socketvar.h',
        'usrsctp/usrsctplib/user_uma.h',
        'usrsctp/usrsctplib/usrsctp.h'
      ],  # sources
      'conditions': [
        ['OS=="linux" or OS=="android"', {
          'defines': [
            '__Userspace_os_Linux',
            '_GNU_SOURCE',
          ],
          'cflags!': [ '-Werror', '-Wall' ],
          'cflags': [ '-w', '-pedantic' ],
        }],
        ['OS=="mac" or OS=="ios"', {
          'defines': [
            'HAVE_SA_LEN',
            'HAVE_SCONN_LEN',
            '__APPLE_USE_RFC_2292',
            '__Userspace_os_Darwin',
          ],
          # usrsctp requires that __APPLE__ is undefined for compilation (for
          # historical reasons). There is a plan to change this, and when it
          # happens and we re-roll DEPS for usrsctp, we can remove the manual
          # undefining of __APPLE__.
          'xcode_settings': {
            'OTHER_CFLAGS!': [ '-Werror', '-Wall' ],
            'OTHER_CFLAGS': [ '-U__APPLE__', '-w' ],
          },
        }],
        ['OS=="win"', {
          'defines': [
            '__Userspace_os_Windows',
            # Manually setting WINVER and _WIN32_WINNT is needed because Chrome
            # sets WINVER to a newer version of  windows. But compiling usrsctp
            # this way would is incompatible  with windows XP.
            # 'WINVER=0x0502',
            # '_WIN32_WINNT=0x0502',
          ],
          'defines!': [
            # Remove Chrome's WINVER defines to avoid redefinition warnings.
            # 'WINVER=0x0602',
            # '_WIN32_WINNT=0x0602',
          ],
          # 'cflags!': [ '/W3', '/WX' ],
          # 'cflags': [ '/w' ],
          # TODO(ldixon) : Remove this disabling of warnings by pushing a
          # fix upstream to usrsctp
          # 'msvs_disabled_warnings': [ 4002, 4013, 4018, 4133, 4267, 4313, 4700 ],
        }, {  # OS != "win",
          'defines': [
            'NON_WINDOWS_DEFINE',
          ],
        }],
      ],  # conditions
    },  # target usrsctp
  ],  # targets
}
