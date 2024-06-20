#include <mm/shmem.h>
#include <mm/vmm.h>
#include <mm/kmalloc.h>
#include <sys/smp.h>

shmem_chunk* shmem_list[SHMEM_MAX];
u64 shmem_count = 0;

u64 shmem_create(uptr addr, usize size, u16 max_conn, u64 flags) {
  shmem_chunk* shmem = (shmem_chunk*)kmalloc(sizeof(shmem_chunk));
  shmem->id = shmem_count;
  shmem->addr = heap_get_allocation_paddr(this_thread()->heap_area, addr);
  shmem->size = size + sizeof(heap_block);
  shmem->conn_count = 0;
  shmem->max_conn = max_conn;
  shmem->flags = flags;
  shmem_list[shmem_count++] = shmem;
  return shmem_count - 1;
}

int shmem_connect(u64 id) {
  if (id >= SHMEM_MAX) return -1;
  shmem_chunk* shmem = shmem_list[id];
  if (shmem == NULL) return -1;
  if (shmem->conn_count < shmem->max_conn) {
    shmem->conn_count++;
    return shmem->id;
  }
  return -1;
}

void* shmem_get(u64 id) {
  // Allocate a new address in the current pagemap, and map the chunk there
  shmem_chunk* shmem = shmem_list[id];
  // TODO: Check if current proc is connected to the shmem
  if (!shmem) return NULL;
  void* ptr = pmm_alloc(DIV_ROUND_UP(shmem->size, PAGE_SIZE));
  vmm_map_user_range(this_cpu()->pm, (uptr)ptr, (uptr)ALIGN_DOWN(shmem->addr, PAGE_SIZE), DIV_ROUND_UP(shmem->size, PAGE_SIZE), shmem->flags);
  return (void*)(ptr + sizeof(heap_block));
}
