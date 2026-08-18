
#pragma once

#include <simd.h>
#include <assert.h>

#include <stdint.h>

typedef uint64_t T;
#define T LDM_ADDR_TO_INT_TYPE

#define _SIMD_VCONH(x, y, z) simd_vconh(x, y, z)

#define ALIGN_LEFT(x, n) (char *)((T)x & -n)
#define ALIGN_RIGHT(x, n) ALIGN_LEFT(x + n - 1, n)
#define RI(x) ALIGN_RIGHT(x, N)
#define LE(x) ALIGN_LEFT(x, N)

#undef T

// #ifdef __sw_slave__
// static __thread_local volatile int __rt_local_rply;
// static __thread_local volatile int __rt_remote_rply;

typedef _Float16 half;
#ifdef MAX
#undef MAX
#endif
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define __ldm_op(name, type, vec_type, align)                                            \
    __device__ static void name(type *a, type *b, int len) {                             \
        type *pa, *pb;                                                                   \
        vec_type a0, b0, a1, b1, a2, b2, a3, b3;                                         \
        if ((((size_t)(a) & (align - 1)) == 0) && (((size_t)(b) & (align - 1)) == 0)) {  \
            int i;                                                                       \
            pa = a;                                                                      \
            pb = b;                                                                      \
            for (i = 0; i <= len - 64; i += 64) {                                        \
                simd_load(a0, pa);                                                       \
                simd_load(b0, pb);                                                       \
                simd_load(a1, pa + 16);                                                  \
                simd_load(b1, pb + 16);                                                  \
                simd_load(a2, pa + 32);                                                  \
                simd_load(b2, pb + 32);                                                  \
                a0 += b0;                                                                \
                simd_load(a3, pa + 48);                                                  \
                simd_load(b3, pb + 48);                                                  \
                a1 += b1;                                                                \
                a2 += b2;                                                                \
                a3 += b3;                                                                \
                simd_store(a0, pa);                                                      \
                simd_store(a1, pa + 16);                                                 \
                simd_store(a2, pa + 32);                                                 \
                simd_store(a3, pa + 48);                                                 \
                pa += 64;                                                                \
                pb += 64;                                                                \
            }                                                                            \
            for (; i <= len - 16; i += 16) {                                             \
                simd_load(a0, pa);                                                       \
                simd_load(b0, pb);                                                       \
                a0 += b0;                                                                \
                simd_store(a0, pa);                                                      \
                pa += 16;                                                                \
                pb += 16;                                                                \
            }                                                                            \
            for (; i < len; i++) a[i] += b[i];                                           \
        } else {                                                                         \
            int i;                                                                       \
            for (i = 0; i <= len - 16; i += 16) {                                        \
                simd_loadu(a0, a + i);                                                   \
                simd_loadu(b0, b + i);                                                   \
                a0 += b0;                                                                \
                simd_storeu(a0, a + i);                                                  \
            }                                                                            \
            for (; i < len; i++) a[i] += b[i];                                           \
        }                                                                                \
    }

#define __ldm_op_short(name, type, vec_type, align)                                      \
    __device__ static void name(type *a, type *b, int len) {                             \
        type *pa, *pb;                                                                   \
        vec_type a0, b0, a1, b1, a2, b2, a3, b3;                                         \
        if ((((size_t)(a) & (align - 1)) == 0) && (((size_t)(b) & (align - 1)) == 0)) {  \
            int i;                                                                       \
            pa = a;                                                                      \
            pb = b;                                                                      \
            for (i = 0; i <= len - 128; i += 128) {                                      \
                simd_load(a0, pa);                                                       \
                simd_load(b0, pb);                                                       \
                simd_load(a1, pa + 32);                                                  \
                simd_load(b1, pb + 32);                                                  \
                simd_load(a2, pa + 64);                                                  \
                simd_load(b2, pb + 64);                                                  \
                a0 += b0;                                                                \
                simd_load(a3, pa + 96);                                                  \
                simd_load(b3, pb + 96);                                                  \
                a1 += b1;                                                                \
                a2 += b2;                                                                \
                a3 += b3;                                                                \
                simd_store(a0, pa);                                                      \
                simd_store(a1, pa + 32);                                                 \
                simd_store(a2, pa + 64);                                                 \
                simd_store(a3, pa + 96);                                                 \
                pa += 128;                                                               \
                pb += 128;                                                               \
            }                                                                            \
            for (; i <= len - 32; i += 32) {                                             \
                simd_load(a0, pa);                                                       \
                simd_load(b0, pb);                                                       \
                a0 += b0;                                                                \
                simd_store(a0, pa);                                                      \
                pa += 32;                                                                \
                pb += 32;                                                                \
            }                                                                            \
            for (; i < len; i++) a[i] += b[i];                                           \
        } else {                                                                         \
            int i;                                                                       \
            for (i = 0; i <= len - 32; i += 32) {                                        \
                simd_loadu(a0, a + i);                                                   \
                simd_loadu(b0, b + i);                                                   \
                a0 += b0;                                                                \
                simd_storeu(a0, a + i);                                                  \
            }                                                                            \
            for (; i < len; i++) a[i] += b[i];                                           \
        }                                                                                \
    }

#define _LE_CORE_LOOP_N(_load, _store, N)                                                \
    do {                                                                                 \
        char *psrc = LE(src + lcnt);                                                     \
        char *pdst = RI(dst);                                                            \
        shortv32 cur0, cur1, tmp0;                                                       \
        shortv32 cur2, cur3, cur4, cur5, tmp1, tmp2, tmp3, tmp4;                         \
        _load(cur0, psrc);                                                               \
        int i = 0;                                                                       \
        psrc += N;                                                                       \
        for (; i <= mcnt - N * 5; i += N * 5) {                                          \
            _load(cur1, psrc + i + N * 0);                                               \
            _load(cur2, psrc + i + N * 1);                                               \
            _load(cur3, psrc + i + N * 2);                                               \
            _load(cur4, psrc + i + N * 3);                                               \
            _load(cur5, psrc + i + N * 4);                                               \
            tmp0 = _SIMD_VCONH(cur0, cur1, off);                                         \
            tmp1 = _SIMD_VCONH(cur1, cur2, off);                                         \
            tmp2 = _SIMD_VCONH(cur2, cur3, off);                                         \
            tmp3 = _SIMD_VCONH(cur3, cur4, off);                                         \
            tmp4 = _SIMD_VCONH(cur4, cur5, off);                                         \
            _store(tmp0, pdst + i + N * 0);                                              \
            _store(tmp1, pdst + i + N * 1);                                              \
            _store(tmp2, pdst + i + N * 2);                                              \
            _store(tmp3, pdst + i + N * 3);                                              \
            _store(tmp4, pdst + i + N * 4);                                              \
            cur0 = cur5;                                                                 \
        }                                                                                \
        for (; i <= mcnt - N; i += N) {                                                  \
            _load(cur1, (short *)(psrc + i));                                            \
            tmp0 = _SIMD_VCONH(cur0, cur1, off);                                         \
            _store(tmp0, (short *)(pdst + i));                                           \
            cur0 = cur1;                                                                 \
        }                                                                                \
    } while (0)

#define _RI_CORE_LOOP_N(_load, _store, N)                                                \
    do {                                                                                 \
        char *psrc = LE(src - lcnt);                                                     \
        char *pdst = LE(dst) - N;                                                        \
        shortv32 cur0, cur1, tmp0;                                                       \
        shortv32 cur2, cur3, cur4, cur5, tmp1, tmp2, tmp3, tmp4;                         \
        _load(cur0, psrc);                                                               \
        int i = 0;                                                                       \
        psrc -= N;                                                                       \
        for (; i <= mcnt - N * 5; i += N * 5) {                                          \
            _load(cur1, psrc - i - N * 0);                                               \
            _load(cur2, psrc - i - N * 1);                                               \
            _load(cur3, psrc - i - N * 2);                                               \
            _load(cur4, psrc - i - N * 3);                                               \
            _load(cur5, psrc - i - N * 4);                                               \
            tmp0 = _SIMD_VCONH(cur1, cur0, off);                                         \
            tmp1 = _SIMD_VCONH(cur2, cur1, off);                                         \
            tmp2 = _SIMD_VCONH(cur3, cur2, off);                                         \
            tmp3 = _SIMD_VCONH(cur4, cur3, off);                                         \
            tmp4 = _SIMD_VCONH(cur5, cur4, off);                                         \
            _store(tmp0, pdst - i - N * 0);                                              \
            _store(tmp1, pdst - i - N * 1);                                              \
            _store(tmp2, pdst - i - N * 2);                                              \
            _store(tmp3, pdst - i - N * 3);                                              \
            _store(tmp4, pdst - i - N * 4);                                              \
            cur0 = cur5;                                                                 \
        }                                                                                \
        for (; i <= mcnt - N; i += N) {                                                  \
            _load(cur1, psrc - i);                                                       \
            tmp0 = _SIMD_VCONH(cur1, cur0, off);                                         \
            _store(tmp0, pdst - i);                                                      \
            cur0 = cur1;                                                                 \
        }                                                                                \
    } while (0)

#define _MEMCPY_REV_(le, ri)                                                             \
    for (int i = le; i < ri; ++i) dst[-1 - i] = src[-1 - i];
#define _MEMCPY_(le, ri)                                                                 \
    for (int i = le; i < ri; ++i) dst[i] = src[i];

__device__ __ldm_op(__ldm_add_ft32, float, floatv16, 64);
__device__ __ldm_op(__ldm_add_ft16, half, float16v16, 32);
__device__ __ldm_op(__ldm_add_int, int32_t, intv16, 64);
__device__ __ldm_op(__ldm_add_uint, uint32_t, uintv16, 64);
__device__ __ldm_op_short(__ldm_add_short, int16_t, shortv32, 64);
__device__ __ldm_op_short(__ldm_add_ushort, uint16_t, ushortv32, 64);

__device__ static inline __attribute__((always_inline)) void __ldm_op_dispatcher(
    void *a, void *b, int len, RED_TYPE type) {
    switch (type) {
    case FLOAT_TYPE: __ldm_add_ft32((float *)a, (float *)b, len); break;
    case HALF_TYPE: __ldm_add_ft16((half *)a, (half *)b, len); break;
    // case INT_TYPE: __ldm_add_int((int32_t *)a, (int32_t *)b, len); break;
    // case UINT_TYPE: __ldm_add_uint((uint32_t *)a, (uint32_t *)b, len); break;
    // case SHORT_TYPE: __ldm_add_short((int16_t *)a, (int16_t *)b, len); break;
    // case USHORT_TYPE:
    //    __ldm_add_ushort((uint16_t *)a, (uint16_t *)b, len);
    //    break;
    default: assert(0 && "unsupported reduce type");
    }
}

__device__ static inline __attribute__((always_inline)) int datatype_to_size(
    RED_TYPE type) {
    switch (type) {
    case FLOAT_TYPE: return sizeof(float);
    case HALF_TYPE: return sizeof(half);
    default: assert(0 && "unsupported reduce type");
    }
    return 0;
}

static __device__ void my_memmove(void *dst, const void *src, size_t num) {
    const char *src_p = (char *)src;
    char *dst_p = (char *)dst;
    if (dst_p > src_p) {
        // 从右向左
        src_p = src_p + num - 1;
        dst_p = dst_p + num - 1;
        while (num > 0) {
            *dst_p = *src_p;
            dst_p--;
            src_p--;
            num--;
        }
    } else {
        // 从左向右的
        while (num > 0) {
            *dst_p = *src_p;
            dst_p++;
            src_p++;
            num--;
        }
    }
    // return dst;
}

static __device__ void rt_rma_allreduce(void *data, void *buf, int len, RED_TYPE type) {
    const int rid = _ROW; // row
    const int cid = _COL; // col
    const int tid = _PEN;
    const int data_type_size = datatype_to_size(type);
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    rt_synchronized_array();
    if (cid % 2 == 0) { // 1,3,5,7 -> 0,2,4,6
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 1);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else {
        while (__rt_remote_rply == 0)
            ;
    }
    __rt_local_rply = 0;
    __rt_remote_rply = 0;

    if (cid % 4 == 0) {                                              // 2,6 -> 0,4
        asm volatile("ldi $1, 0x55\n synr $1\n" ::: "memory", "$1"); // synr 0, 2, 4, 6
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 2);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid % 4 == 2) {
        asm volatile("ldi $1, 0x55\n synr $1\n" ::: "memory", "$1"); // synr 0, 2, 4, 6
        while (__rt_remote_rply == 0)
            ;
    }
    // reset reply words
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    if (cid == 0) {                                                  // 4 -> 0
        asm volatile("ldi $1, 0x11\n synr $1\n" ::: "memory", "$1"); // synr 0, 4
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 4);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid == 4) {
        asm volatile("ldi $1, 0x11\n synr $1\n" ::: "memory", "$1"); // synr 0, 4
        while (__rt_remote_rply == 0)
            ;
    }

    // reset reply words
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    if (cid == 0 && rid % 2 == 0) {                                 // rid 1,3 -> 0,2
        asm volatile("ldi $1, 0xF\n sync $1\n" ::: "memory", "$1"); // sync 0,1,2,3
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 8);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid == 0 && rid % 2 == 1) {
        asm volatile("ldi $1, 0xF\n sync $1\n" ::: "memory", "$1"); // sync 0,1,2,3
        while (__rt_remote_rply == 0)
            ;
    }
    // reset reply words
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    if (cid == 0 && rid == 0) {                                     // 2->0
        asm volatile("ldi $1, 0x5\n sync $1\n" ::: "memory", "$1"); // sync 0,2
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 16);
        while (__rt_local_rply == 0)
            ;
        // !!!note that results are stored in buf, not data in last round
        __ldm_op_dispatcher(buf, data, len, type);
    } else if (cid == 0 && rid == 2) {
        asm volatile("ldi $1, 0x5\n sync $1\n" ::: "memory", "$1"); // sync 0,2
        while (__rt_remote_rply == 0)
            ;
    }

    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    rt_synchronized_array();
    // broad cast
    if (tid == 0) {
        rt_rma(0x5, 0x0, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0x0F, 0);
    }
    while (__rt_remote_rply == 0)
        ;
}

static __device__ void rt_rma_reduce(void *data, void *buf, int len, RED_TYPE type) {
    const int rid = _ROW; // row
    const int cid = _COL; // col
    const int tid = _PEN;
    const int data_type_size = datatype_to_size(type);
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    rt_synchronized_array();
    if (cid % 2 == 0) { // 1,3,5,7 -> 0,2,4,6
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 1);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else {
        while (__rt_remote_rply == 0)
            ;
    }
    __rt_local_rply = 0;
    __rt_remote_rply = 0;

    if (cid % 4 == 0) {                                              // 2,6 -> 0,4
        asm volatile("ldi $1, 0x55\n synr $1\n" ::: "memory", "$1"); // synr 0, 2, 4, 6
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 2);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid % 4 == 2) {
        asm volatile("ldi $1, 0x55\n synr $1\n" ::: "memory", "$1"); // synr 0, 2, 4, 6
        while (__rt_remote_rply == 0)
            ;
    }
    // reset reply words
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    if (cid == 0) {                                                  // 4 -> 0
        asm volatile("ldi $1, 0x11\n synr $1\n" ::: "memory", "$1"); // synr 0, 4
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 4);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid == 4) {
        asm volatile("ldi $1, 0x11\n synr $1\n" ::: "memory", "$1"); // synr 0, 4
        while (__rt_remote_rply == 0)
            ;
    }

    // reset reply words
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    if (cid == 0 && rid % 2 == 0) {                                 // rid 1,3 -> 0,2
        asm volatile("ldi $1, 0xF\n sync $1\n" ::: "memory", "$1"); // sync 0,1,2,3
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 8);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid == 0 && rid % 2 == 1) {
        asm volatile("ldi $1, 0xF\n sync $1\n" ::: "memory", "$1"); // sync 0,1,2,3
        while (__rt_remote_rply == 0)
            ;
    }
    // reset reply words
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    if (cid == 0 && rid == 0) {                                     // 2->0
        asm volatile("ldi $1, 0x5\n sync $1\n" ::: "memory", "$1"); // sync 0,2
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 16);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid == 0 && rid == 2) {
        asm volatile("ldi $1, 0x5\n sync $1\n" ::: "memory", "$1"); // sync 0,2
        while (__rt_remote_rply == 0)
            ;
    }
}

static __device__ void rt_rma_allreduce_to(int core_id, void *data, void *buf, int len,
                                           RED_TYPE type) {
    const int rid = _ROW; // row
    const int cid = _COL; // col
    const int tid = _PEN;
    const int data_type_size = datatype_to_size(type);
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    rt_synchronized_array();
    if (cid % 2 == 0) { // 1,3,5,7 -> 0,2,4,6
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 1);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else {
        while (__rt_remote_rply == 0)
            ;
    }
    __rt_local_rply = 0;
    __rt_remote_rply = 0;

    if (cid % 4 == 0) {                                              // 2,6 -> 0,4
        asm volatile("ldi $1, 0x55\n synr $1\n" ::: "memory", "$1"); // synr 0, 2, 4, 6
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 2);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid % 4 == 2) {
        asm volatile("ldi $1, 0x55\n synr $1\n" ::: "memory", "$1"); // synr 0, 2, 4, 6
        while (__rt_remote_rply == 0)
            ;
    }
    // reset reply words
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    if (cid == 0) {                                                  // 4 -> 0
        asm volatile("ldi $1, 0x11\n synr $1\n" ::: "memory", "$1"); // synr 0, 4
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 4);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid == 4) {
        asm volatile("ldi $1, 0x11\n synr $1\n" ::: "memory", "$1"); // synr 0, 4
        while (__rt_remote_rply == 0)
            ;
    }

    // reset reply words
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    if (cid == 0 && rid % 2 == 0) {                                 // rid 1,3 -> 0,2
        asm volatile("ldi $1, 0xF\n sync $1\n" ::: "memory", "$1"); // sync 0,1,2,3
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 8);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid == 0 && rid % 2 == 1) {
        asm volatile("ldi $1, 0xF\n sync $1\n" ::: "memory", "$1"); // sync 0,1,2,3
        while (__rt_remote_rply == 0)
            ;
    }
    // reset reply words
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    if (cid == 0 && rid == 0) {                                     // 2->0
        asm volatile("ldi $1, 0x5\n sync $1\n" ::: "memory", "$1"); // sync 0,2
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 16);
        while (__rt_local_rply == 0)
            ;
        // !!!note that results are stored in buf, not data in last round
        __ldm_op_dispatcher(buf, data, len, type);
    } else if (cid == 0 && rid == 2) {
        asm volatile("ldi $1, 0x5\n sync $1\n" ::: "memory", "$1"); // sync 0,2
        while (__rt_remote_rply == 0)
            ;
    }
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    rt_synchronized_array();
    if (tid == core_id) {
        rt_rma(0x0, 0x1, buf, data, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, 0);
        while (__rt_local_rply == 0)
            ;
    } else if (tid == 0) {
        while (__rt_remote_rply == 0)
            ;
    }
}

static __device__ void rt_rma_row_reduce(void *data, void *buf, int len, RED_TYPE type) {
    const int rid = _ROW; // row
    const int cid = _COL; // col
    const int tid = _PEN;
    const int data_type_size = datatype_to_size(type);
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    rt_synchronized_row();
    if (cid % 2 == 0) { // 1,3,5,7 -> 0,2,4,6
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 1);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else {
        while (__rt_remote_rply == 0)
            ;
    }
    __rt_local_rply = 0;
    __rt_remote_rply = 0;

    if (cid % 4 == 0) {                                              // 2,6 -> 0,4
        asm volatile("ldi $1, 0x55\n synr $1\n" ::: "memory", "$1"); // synr 0, 2, 4, 6
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 2);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid % 4 == 2) {
        asm volatile("ldi $1, 0x55\n synr $1\n" ::: "memory", "$1"); // synr 0, 2, 4, 6
        while (__rt_remote_rply == 0)
            ;
    }
    // reset reply words
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    if (cid == 0) {                                                  // 4 -> 0
        asm volatile("ldi $1, 0x11\n synr $1\n" ::: "memory", "$1"); // synr 0, 4
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 4);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid == 4) {
        asm volatile("ldi $1, 0x11\n synr $1\n" ::: "memory", "$1"); // synr 0, 4
        while (__rt_remote_rply == 0)
            ;
    }
}

static __device__ void rt_rma_row_reduce_to(int target_cid, void *data, void *buf,
                                            int len, RED_TYPE type) {
    const int rid = _ROW; // row
    const int cid = _COL; // col
    const int tid = _PEN;
    const int data_type_size = datatype_to_size(type);
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    rt_synchronized_row();
    if (cid % 2 == 0) { // 1,3,5,7 -> 0,2,4,6
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 1);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else {
        while (__rt_remote_rply == 0)
            ;
    }
    __rt_local_rply = 0;
    __rt_remote_rply = 0;

    if (cid % 4 == 0) {                                              // 2,6 -> 0,4
        asm volatile("ldi $1, 0x55\n synr $1\n" ::: "memory", "$1"); // synr 0, 2, 4, 6
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 2);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid % 4 == 2) {
        asm volatile("ldi $1, 0x55\n synr $1\n" ::: "memory", "$1"); // synr 0, 2, 4, 6
        while (__rt_remote_rply == 0)
            ;
    }
    // reset reply words
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    if (cid == 0) {                                                  // 4 -> 0
        asm volatile("ldi $1, 0x11\n synr $1\n" ::: "memory", "$1"); // synr 0, 4
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 4);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid == 4) {
        asm volatile("ldi $1, 0x11\n synr $1\n" ::: "memory", "$1"); // synr 0, 4
        while (__rt_remote_rply == 0)
            ;
    }

    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    rt_synchronized_row();
    if (tid == (rid * 8 + target_cid)) {
        rt_rma(0x0, 0x1, data, data, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, rid * 8);
        while (__rt_local_rply == 0)
            ;
    } else if (tid == (rid * 8)) {
        while (__rt_remote_rply == 0)
            ;
    }
}

static __device__ void rt_rma_col_reduce(void *data, void *buf, int len, RED_TYPE type) {
    const int rid = _ROW; // row
    const int cid = _COL; // col
    const int tid = _PEN;
    const int data_type_size = datatype_to_size(type);
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    rt_synchronized_col();
    if (rid % 2 == 0) { // rid 1,3 -> 0,2
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 8);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid == 0 && rid % 2 == 1) {
        while (__rt_remote_rply == 0)
            ;
    }
    // reset reply words
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    if (rid == 0) {                                                 // 2->0
        asm volatile("ldi $1, 0x5\n sync $1\n" ::: "memory", "$1"); // sync 0,2
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 16);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (rid == 2) {
        asm volatile("ldi $1, 0x5\n sync $1\n" ::: "memory", "$1"); // sync 0,2
        while (__rt_remote_rply == 0)
            ;
    }
}

static __device__ void rt_rma_col_reduce_to(int target_rid, void *data, void *buf,
                                            int len, RED_TYPE type) {
    const int rid = _ROW; // row
    const int cid = _COL; // col
    const int tid = _PEN;
    const int data_type_size = datatype_to_size(type);
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    rt_synchronized_col();
    if (rid % 2 == 0) { // rid 1,3 -> 0,2
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 8);
        while (__rt_local_rply == 0)
            ;
        __ldm_op_dispatcher(data, buf, len, type);
    } else if (cid == 0 && rid % 2 == 1) {
        while (__rt_remote_rply == 0)
            ;
    }
    // reset reply words
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    rt_synchronized_col();
    if (rid == 0) { // 2->0
        rt_rma(0x0, 0x1, data, buf, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, tid + 16);
        while (__rt_local_rply == 0)
            ;
        // !!!note that results are stored in buf, not data in last round
        __ldm_op_dispatcher(buf, data, len, type);
    } else if (rid == 2) {
        while (__rt_remote_rply == 0)
            ;
    }
    __rt_local_rply = 0;
    __rt_remote_rply = 0;
    rt_synchronized_col();
    if (tid == (target_rid * 8 + cid)) {
        rt_rma(0x0, 0x1, buf, data, len * data_type_size, &__rt_remote_rply,
               &__rt_local_rply, 0, cid);
        while (__rt_local_rply == 0)
            ;
    } else if (tid == cid) {
        while (__rt_remote_rply == 0)
            ;
    }
}

static __device__ void rt_unaligned_dma_get(void *dest, void *src, int len) {
    char *unaligned_mem_addr = (char *)src;
    char *unaligned_ldm_addr = (char *)dest;
    char *aligned_mem_addr, *aligned_ldm_addr;

    if (((unsigned long long)unaligned_mem_addr & 0x3) != 0)
        aligned_mem_addr =
            (char *)((((unsigned long long)unaligned_mem_addr + 4) / 4) * 4);
    else
        aligned_mem_addr = unaligned_mem_addr;

    if (((unsigned long long)unaligned_ldm_addr & 0x3) != 0)
        aligned_ldm_addr =
            (char *)((((unsigned long long)unaligned_ldm_addr + 4) / 4) * 4);
    else
        aligned_ldm_addr = unaligned_ldm_addr;

    int offset_mem_addr =
        (unsigned long long)aligned_mem_addr - (unsigned long long)unaligned_mem_addr;
    int offset_ldm_addr =
        (unsigned long long)aligned_ldm_addr - (unsigned long long)unaligned_ldm_addr;

    int candidate_dma_trans_size = len - MAX(offset_mem_addr, offset_ldm_addr);
    if (candidate_dma_trans_size < 4) {
        for (int i = 0; i < len; i++) {
            unaligned_ldm_addr[i] = unaligned_mem_addr[i];
        }
    } else {
        int dma_trans_size = (candidate_dma_trans_size / 4) * 4;
        rt_dma_get(aligned_ldm_addr, aligned_mem_addr, dma_trans_size);
        my_memmove(unaligned_ldm_addr + offset_mem_addr, aligned_ldm_addr,
                   dma_trans_size);
        for (int i = 0; i < offset_mem_addr; i++) {
            unaligned_ldm_addr[i] = unaligned_mem_addr[i];
        }
        int idx = offset_mem_addr + dma_trans_size;
        for (int i = 0; i < (len - offset_mem_addr - dma_trans_size); i++) {
            unaligned_ldm_addr[idx + i] = unaligned_mem_addr[idx + i];
        }
    }
    return;
}

static __device__ void rt_unaligned_dma_put(void *dest, void *src, int len) {
    char *unaligned_mem_addr = (char *)dest;
    char *unaligned_ldm_addr = (char *)src;
    char *aligned_mem_addr, *aligned_ldm_addr;

    if (((unsigned long long)unaligned_mem_addr & 0x3) != 0)
        aligned_mem_addr =
            (char *)((((unsigned long long)unaligned_mem_addr + 4) / 4) * 4);
    else
        aligned_mem_addr = unaligned_mem_addr;

    if (((unsigned long long)unaligned_ldm_addr & 0x3) != 0)
        aligned_ldm_addr =
            (char *)((((unsigned long long)unaligned_ldm_addr + 4) / 4) * 4);
    else
        aligned_ldm_addr = unaligned_ldm_addr;

    int offset_mem_addr =
        (unsigned long long)aligned_mem_addr - (unsigned long long)unaligned_mem_addr;
    int offset_ldm_addr =
        (unsigned long long)aligned_ldm_addr - (unsigned long long)unaligned_ldm_addr;

    int candidate_dma_trans_size = len - MAX(offset_mem_addr, offset_ldm_addr);
    if (candidate_dma_trans_size < 4) {
        for (int i = 0; i < len; i++) {
            unaligned_mem_addr[i] = unaligned_ldm_addr[i];
        }
    } else {
        int dma_trans_size = (candidate_dma_trans_size / 4) * 4;
        rt_dma_put(aligned_mem_addr, aligned_ldm_addr, dma_trans_size);
        my_memmove(unaligned_mem_addr + offset_ldm_addr, aligned_mem_addr,
                   dma_trans_size);
        for (int i = 0; i < offset_ldm_addr; i++) {
            unaligned_mem_addr[i] = unaligned_ldm_addr[i];
        }
        int idx = offset_ldm_addr + dma_trans_size;
        for (int i = 0; i < (len - offset_ldm_addr - dma_trans_size); i++) {
            unaligned_mem_addr[idx + i] = unaligned_ldm_addr[idx + i];
        }
    }
    return;
}

// 跨步转化
static __device__ void rt_unaligned_dma_get_stride(void *dest, void *src, int len,
                                                   int bsize, int stride) {
    int ST = len / bsize;
    int mem_bsize = bsize + stride;
    int ldm_bsize = bsize;
    for (int i = 0; i < ST; i++) {
        rt_unaligned_dma_get((char *)dest + i * ldm_bsize, (char *)src + i * mem_bsize,
                             bsize);
    }
}

static __device__ void rt_unaligned_dma_put_stride(void *dest, void *src, int len,
                                                   int bsize, int stride) {
    int ST = len / bsize;
    int mem_bsize = bsize + stride;
    int ldm_bsize = bsize;
    for (int i = 0; i < ST; i++) {
        rt_unaligned_dma_put((char *)dest + i * mem_bsize, (char *)src + i * ldm_bsize,
                             bsize);
    }
}

// 为正确处理重叠情况, 倒序拷贝
static __device__ void memcpy_right(void *_dst, void *_src, int ln) {
    char *dst = (char *)_dst, *src = (char *)_src;
    uint64_t off_int = src - dst;

    const int ALIGN_1B = off_int & 0x1;
    if (ALIGN_1B) off_int <<= 1;
    void *off = (void *)off_int;

    const int N = ALIGN_1B ? 32 : 64;
    dst += ln, src += ln;
    int lcnt = dst - LE(dst);
    int hcnt = RI(dst - ln) - (dst - ln);
    int mcnt = LE(dst) - RI(dst - ln);
    if (mcnt > 0) {
        _MEMCPY_REV_(0, lcnt);
        if (ALIGN_1B)
            _RI_CORE_LOOP_N(simd_load_ext, simd_store_ext, 32);
        else
            _RI_CORE_LOOP_N(simd_load, simd_store, 64);
        _MEMCPY_REV_(ln - hcnt, ln);
    } else {
        _MEMCPY_REV_(0, ln);
    }
}

// 正序拷贝
static __device__ void memcpy_left(void *_dst, void *_src, int ln) {
    char *dst = (char *)_dst, *src = (char *)_src;
    uint64_t off_int = src - dst;

    const int ALIGN_1B = off_int & 0x1;
    if (ALIGN_1B) off_int <<= 1;
    void *off = (void *)off_int;

    const int N = ALIGN_1B ? 32 : 64;
    int lcnt = RI(dst) - dst;
    int mcnt = LE(dst + ln) - RI(dst);
    int hcnt = dst + ln - LE(dst + ln);
    if (mcnt > 0) {
        _MEMCPY_(0, lcnt);
        if (ALIGN_1B)
            _LE_CORE_LOOP_N(simd_load_ext, simd_store_ext, 32);
        else
            _LE_CORE_LOOP_N(simd_load, simd_store, 64);
        _MEMCPY_(ln - hcnt, ln);
    } else {
        _MEMCPY_(0, ln);
    }
}

static __device__ void rt_memmove(void *dst, void *src, int ln) {
    if (src < dst)
        memcpy_right(dst, src, ln);
    else
        memcpy_left(dst, src, ln);
}

static __device__ void rt_memcpy(void *dst, void *src, int ln) {
    memcpy_left(dst, src, ln);
}

#define MAX_SPM_MEMSET_SIZE 239616  // 234KB for SPM

static __device__ void inline __attribute__((always_inline)) 
memset_any(void *dest, int data, size_t count) {
    if (count > MAX_SPM_MEMSET_SIZE) {
        return;
    }
    unsigned char tmp = (unsigned char)data;
    unsigned int temp = (unsigned int)tmp;
    unsigned char *ori_dest = (unsigned char *)dest;
    unsigned int bias = (64 - (unsigned long)(dest) & 0x3f);

    for (int i = 0; i < bias && i < count; i++) {
        ori_dest[i] = tmp;
    }
    if (bias >= count) {
        return;
    }

    // reorder ptr
    unsigned int *x = (unsigned int *)(ori_dest + bias);
    const unsigned int data_size = count - bias;

    int num = data_size / sizeof(unsigned int);
    int remain_size = data_size - num * sizeof(unsigned int);
    unsigned int *x_buf_temp = x;
    int i = 0;
    // data_value 每字节赋值相同的值
    unsigned int data_value = 0;
    data_value |= temp;
    data_value |= (temp << 8);
    data_value |= (temp << 16);
    data_value |= (temp << 24);
    uintv16 value = data_value;
    for (; i <= (num - 512); i += 512) {
        simd_store(value, x);
        simd_store(value, x + 16);
        simd_store(value, x + 32);
        simd_store(value, x + 48);
        simd_store(value, x + 64);
        simd_store(value, x + 80);
        simd_store(value, x + 96);
        simd_store(value, x + 112);
        simd_store(value, x + 128);
        simd_store(value, x + 144);
        simd_store(value, x + 160);
        simd_store(value, x + 176);
        simd_store(value, x + 192);
        simd_store(value, x + 208);
        simd_store(value, x + 224);
        simd_store(value, x + 240);
        simd_store(value, x + 256);
        simd_store(value, x + 272);
        simd_store(value, x + 288);
        simd_store(value, x + 304);
        simd_store(value, x + 320);
        simd_store(value, x + 336);
        simd_store(value, x + 352);
        simd_store(value, x + 368);
        simd_store(value, x + 384);
        simd_store(value, x + 400);
        simd_store(value, x + 416);
        simd_store(value, x + 432);
        simd_store(value, x + 448);
        simd_store(value, x + 464);
        simd_store(value, x + 480);
        simd_store(value, x + 496);
        x += 512;
    }
    for (; i <= (num - 128); i += 128) {
        simd_store(value, x);
        simd_store(value, x + 16);
        simd_store(value, x + 32);
        simd_store(value, x + 48);
        simd_store(value, x + 64);
        simd_store(value, x + 80);
        simd_store(value, x + 96);
        simd_store(value, x + 112);
        x += 128;
    }
    for (; i <= (num - 16); i += 16) {
        simd_store(value, x);
        x += 16;
    }
    for (; i < num; i++) {
        *x = value;
        x += 1;
    }
    unsigned char *from = (unsigned char *)(x);
    for (int j = 0; j < remain_size; j++) from[j] = tmp;
}

static __device__ void inline __attribute__((always_inline)) 
rt_memset(void *dest, int data, size_t len) {
    memset_any(dest, data, len);
}

#undef MAX_SPM_MEMSET_SIZE
#undef _SIMD_VCONH
#undef ALIGN_LEFT
#undef ALIGN_RIGHT
#undef RI
#undef LE
#undef MAX
#undef __ldm_op
#undef __ldm_op_short
#undef _LE_CORE_LOOP_N
#undef _RI_CORE_LOOP_N
#undef _MEMCPY_REV_
#undef _MEMCPY_

// #endif  // __sw_slave__
