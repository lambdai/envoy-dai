// NOLINT(namespace-envoy)
#include "common/bpf/util.h"

#include <errno.h>
#include <error.h>
#include <unistd.h>

void Utils::bpf_check_ptr(const void* obj, const char* name) {
  if (!obj) {
    ::error(1, 0, "bpf: %s: NULL ptr\n", name);
  }

  long err = libbpf_get_error(obj);
  if (err) {
    char err_buf[255];
    libbpf_strerror(err, err_buf, sizeof(err_buf));
    ::error(1, 0, "bpf: %s: %s\n", name, err_buf);
  }
}

bpf_program* Utils::do_bpf_get_prog(bpf_object* obj, const char* name, bpf_prog_type type) {
  auto prog = bpf_object__find_program_by_title(obj, name);
  bpf_check_ptr(obj, name);
  bpf_program__set_type(prog, type);

  return prog;
}

void Utils::do_bpf_attach_prog(bpf_program* prog, int fd, bpf_attach_type type) {
  int prog_fd = bpf_program__fd(prog);
  int ret = bpf_prog_attach(prog_fd, fd, type, 0);
  if (ret) {
    ::error(1, ret, "bpf attach prog %d", type);
  }

  if (::close(prog_fd)) {
    ::error(1, errno, "bpf close prog %d", type);
  }
}

void Utils::do_bpf_cleanup(bpf_object* obj, int map_fd) {
  if (!obj) {
    return;
  }

  if (::close(map_fd)) {
    ::error(1, errno, "close sockmap");
  }

  bpf_object__close(obj);
  obj = nullptr;
}

// #include <sys/syscall.h>
// #include <unistd.h>

// static inline int sys_bpf(bpf_cmd cmd, union bpf_attr* attr, unsigned int size) {
//   return syscall(__NR_bpf, cmd, attr, size);
// }

// int BpfUtils::bpf_create_map(bpf_map_type map_type, int key_size, int value_size, int
// max_entries,
//                              uint32_t map_flags) {
//   union bpf_attr attr = {};
//   attr.map_type = map_type;
//   attr.key_size = key_size;
//   attr.value_size = value_size;
//   attr.max_entries = max_entries;
//   attr.map_flags = map_flags;
//   return sys_bpf(BPF_MAP_CREATE, &attr, sizeof(attr));
// }

// int BpfUtils::bpf_load_program(bpf_prog_type prog_type, const bpf_insn* insns, size_t insns_cnt,
//                                const char* license, uint32_t kern_version, char* log_buf,
//                                size_t log_buf_sz) {
//   union bpf_attr attr = {};
//   attr.prog_type = prog_type;
//   attr.insns = reinterpret_cast<uint64_t>(insns);
//   attr.insn_cnt = insns_cnt;
//   attr.license = reinterpret_cast<uint64_t>(license);
//   attr.log_buf = 0;
//   attr.log_size = 0;
//   attr.log_level = 0;
//   attr.kern_version = kern_version;

//   int fd = sys_bpf(BPF_PROG_LOAD, &attr, sizeof(attr));
//   if (fd >= 0 || !log_buf || !log_buf_sz) {
//     return fd;
//   }

//   /* Try again with log */
//   attr.log_buf = reinterpret_cast<uint64_t>(log_buf);
//   attr.log_size = log_buf_sz;
//   attr.log_level = 1;
//   log_buf[0] = 0;
//   return sys_bpf(BPF_PROG_LOAD, &attr, sizeof(attr));
// }

// int BpfUtils::bpf_prog_attach(int prog_fd, int target_fd, bpf_attach_type type,
//                               unsigned int flags) {
//   union bpf_attr attr = {};

//   attr.target_fd = target_fd;
//   attr.attach_bpf_fd = prog_fd;
//   attr.attach_type = type;
//   attr.attach_flags = flags;

//   return sys_bpf(BPF_PROG_ATTACH, &attr, sizeof(attr));
// }

// int BpfUtils::bpf_map_update_elem(int fd, const void* key, const void* value, uint64_t flags) {
//   union bpf_attr attr = {};

//   attr.map_fd = fd;
//   attr.key = reinterpret_cast<uint64_t>(key);
//   attr.value = reinterpret_cast<uint64_t>(value);
//   attr.flags = flags;

//   return sys_bpf(BPF_MAP_UPDATE_ELEM, &attr, sizeof(attr));
// }

// int BpfUtils::bpf_map_delete_elem(int fd, const void* key) {
//   union bpf_attr attr = {};

//   attr.map_fd = fd;
//   attr.key = reinterpret_cast<uint64_t>(key);

//   return sys_bpf(BPF_MAP_DELETE_ELEM, &attr, sizeof(attr));
// }

// int BpfUtils::bpf_map_lookup_elem(int fd, const void* key, void* value) {
//   union bpf_attr attr = {};

//   attr.map_fd = fd;
//   attr.key = reinterpret_cast<uint64_t>(key);
//   attr.value = reinterpret_cast<uint64_t>(value);

//   return sys_bpf(BPF_MAP_LOOKUP_ELEM, &attr, sizeof(attr));
// }