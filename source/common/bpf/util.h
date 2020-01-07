// NOLINT(namespace-envoy)

#if !defined(__linux__)
#error "Linux platform file is part of non-Linux build."
#endif

#include "bpf.h"
#include "libbpf.h"

class Utils {
public:
  static void bpf_check_ptr(const void* obj, const char* name);
  static bpf_program* do_bpf_get_prog(bpf_object* obj, const char* name, bpf_prog_type type);
  static void do_bpf_attach_prog(bpf_program* prog, int fd, bpf_attach_type type);
  static void do_bpf_cleanup(bpf_object* obj, int map_fd);

  // static int bpf_create_map(bpf_map_type map_type, int key_size, int value_size, int max_entries,
  //                           uint32_t map_flags);
  // static int bpf_load_program(bpf_prog_type prog_type, const bpf_insn* insns, size_t insns_cnt,
  //                             const char* license, uint32_t kern_version, char* log_buf,
  //                             size_t log_buf_sz);
  // static int bpf_prog_attach(int prog_fd, int target_fd, bpf_attach_type type, unsigned int
  // flags); static int bpf_map_update_elem(int fd, const void* key, const void* value, uint64_t
  // flags); static int bpf_map_delete_elem(int fd, const void* key); static int
  // bpf_map_lookup_elem(int fd, const void* key, void* value);
};