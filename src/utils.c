// Copyright (c) 2017 Daniel L. Robertson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/ether.h>

#include <sylkie_config.h>

#ifdef BUILD_SECCOMP
#include <seccomp.h>
#include <sys/prctl.h>
#endif

#include <utils.h>

int lockdown(void) {
#ifdef BUILD_SECCOMP
  scmp_filter_ctx ctx;

  if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
    return -1;
  }

  ctx = seccomp_init(SCMP_ACT_KILL);

  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(socket), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(nanosleep), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(ioctl), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(sendto), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);

  return seccomp_load(ctx);
#else
  return 0;
#endif
}

int parse_hwaddr(const char *arg, u_int8_t *addr) {
  int len = strlen(arg);
  bool colon_next = false;
  const char *end = arg + len;
  char *num_end = NULL;
  int i = 0;
  if (len != 17) {
    return -1;
  }
  while (arg < end && i < ETH_ALEN) {
    if (*arg == ':' && colon_next) {
      ++arg;
      colon_next = false;
    } else if (!colon_next && (arg + 1) < end) {
      addr[i] = (u_int8_t)strtol(arg, &num_end, 16);
      colon_next = true;
      ++i;
      if (num_end) {
        arg = num_end;
      } else {
        return -1;
      }
    } else {
      return -1;
    }
  }
  return 0;
}
