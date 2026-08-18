// BSD 3- Clause License Copyright (c) 2024, Tecorigin Co., Ltd. All rights
// reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
// Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
// OF SUCH DAMAGE.

#ifndef TECOOPS_UAL_COM_RT_H_
#define TECOOPS_UAL_COM_RT_H_

#define CHECK_SPM_OVERFLOW
// #define NOT_KILL_WHEN_SPM_OVERFLOW

#ifdef CHECK_SPM_OVERFLOW
#ifdef NOT_KILL_WHEN_SPM_OVERFLOW
#define RAISE_EXCEPTION (void)0
#else
#define RAISE_EXCEPTION                                     \
    do {                                                    \
        asm volatile("rcsr %0,5\n" : "=&r"(tmp)::"memory"); \
    } while (0)
#endif
#endif

#ifdef SPM_MALLOC_MEMSET
#include <stdlib.h>
#define __safe_spm_malloc(size, section, _filename, _lineno)                             \
    ({                                                                                   \
        void *ptr = (void *)malloc(size, AddressLowToHigh);                              \
        if (ptr == NULL || (int64_t)ptr < 0) {                                           \
            ai_printf("[%s:%d] spm malloc failed, size %u\n", _filename, _lineno, size); \
            return;                                                                      \
        }                                                                                \
        memset(ptr, 0, size);                                                            \
        ptr;                                                                             \
    })
#elif defined(CHECK_SPM_OVERFLOW)
#define __safe_spm_malloc(size, section, _filename, _lineno)                             \
    ({                                                                                   \
        void *ptr = (void *)malloc((size) + 128, section);                               \
        void *real_ptr = NULL;                                                           \
        if (ptr == NULL || (int64_t)ptr < 0) {                                           \
            ai_printf("[%s:%d] spm malloc failed, size %u\n", _filename, _lineno, size); \
            return;                                                                      \
        } else {                                                                         \
            for (int __ik = 0; __ik < 64; __ik++) {                                      \
                ((unsigned char *)ptr)[__ik] = 0xFF;                                     \
                ((unsigned char *)ptr)[(size) + 64 + __ik] = 0xFF;                       \
            }                                                                            \
            ((size_t *)ptr)[0] = size;                                                   \
            real_ptr = (void *)((unsigned long)ptr + 64);                                \
        }                                                                                \
        real_ptr;                                                                        \
    })
#else
#define __safe_spm_malloc(size, section, _filename, _lineno)                             \
    ({                                                                                   \
        void *ptr = (void *)malloc(size, section);                                       \
        if (ptr == NULL || (int64_t)ptr < 0) {                                           \
            ai_printf("[%s:%d] spm malloc failed, size %u\n", _filename, _lineno, size); \
            return;                                                                      \
        }                                                                                \
        ptr;                                                                             \
    })
#endif

#define rt_spm_malloc_try_left(size) __safe_spm_malloc(size, AddressLowToHigh, __FILE__, __LINE__)
#define rt_spm_malloc_try_right(size) __safe_spm_malloc(size, AddressHighToLow, __FILE__, __LINE__)
#define rt_spm_malloc(size) rt_spm_malloc_try_left(size)

#if defined(CHECK_SPM_OVERFLOW)
#define __safe_spm_free(ptr, _filename, _lineno)                                           \
    ({                                                                                     \
        if (ptr != NULL) {                                                                 \
            int __sum_pre = 0, __sum_suf = 0;                                              \
            constexpr int pre_count = 64 - sizeof(size_t);                                 \
            constexpr int suf_count = 64;                                                  \
            size_t size = ((size_t *)ptr)[-(64 / sizeof(size_t))];                         \
            for (int __ik = -pre_count; __ik < 0; __ik++) {                                \
                __sum_pre += ((unsigned char *)ptr)[__ik];                                 \
            }                                                                              \
            for (int __ik = 0; __ik < suf_count; __ik++) {                                 \
                __sum_suf += ((unsigned char *)ptr)[size + __ik];                          \
            }                                                                              \
            if (__sum_pre != ((unsigned char)0xFF) * pre_count ||                          \
                __sum_suf != ((unsigned char)0xFF) * suf_count) {                          \
                ai_printf("[%s:%d]: ERROR!!!!!!! spm overflow detected ptr %p size %lu\n", \
                          _filename, _lineno, ptr, size);                                  \
                unsigned int tmp;                                                          \
                RAISE_EXCEPTION;                                                           \
            }                                                                              \
            free(((unsigned char *)ptr) - 64);                                             \
        }                                                                                  \
    })
#else
#define __safe_spm_free(ptr, _filename, _lineno) \
    ({                                           \
        if (ptr != NULL) {                       \
            free((void *)ptr);                   \
        }                                        \
    })
#endif
#define rt_spm_free(ptr) __safe_spm_free(ptr, __FILE__, __LINE__);

#ifdef __cplusplus
extern "C" {
#endif

#include <slave.h>
#include <stdio.h>

#define CHECK_LDM_OVERFLOW // check ldm overflow
// #define NOT_KILL_WHEN_LDM_OVERFLOW

#ifdef CHECK_LDM_OVERFLOW
#ifdef NOT_KILL_WHEN_LDM_OVERFLOW
#define RAISE_EXCEPTION (void)0
#else
#define RAISE_EXCEPTION                                                                  \
    do {                                                                                 \
        asm volatile("rcsr %0,5\n" : "=&r"(tmp)::"memory");                              \
    } while (0)
#endif
#endif

#define USE_BUILT_IN_ACE
#define USE_BUILT_IN_MEM

// #ifdef __sw_slave__

typedef volatile int rt_rply_t;

typedef enum RED_TYPE {
    FLOAT_TYPE = 0,
    HALF_TYPE = 1,
    INT_TYPE = 2,
    UINT_TYPE = 3,
    SHORT_TYPE = 4,
    USHORT_TYPE = 5
} RED_TYPE;

#define rt_tid (_PEN)
#define rt_rid (_ROW)
#define rt_cid (_COL)
#define rt_id() (rt_tid)
#define rt_row_id() (rt_rid)
#define rt_col_id() (rt_cid)

#define rt_remote_load(var, remote_core_id, r_ptr)                                       \
    ({                                                                                   \
        void *_ptr = (void *)(0x200000000000L | (((uint64_t)(remote_core_id)) << 20)     \
                              | ((size_t)(r_ptr) & 0x3FFFF));                            \
        decltype(r_ptr) _p = (decltype(r_ptr))_ptr;                                      \
        var = *_p;                                                                       \
    })

#define rt_remote_store(val, remote_core_id, r_ptr)                                      \
    ({                                                                                   \
        void *_ptr = (void *)(0x200000000000L | (((uint64_t)(remote_core_id)) << 20)     \
                              | ((size_t)(r_ptr) & 0x3FFFF));                            \
        decltype(r_ptr) _p = (decltype(r_ptr))_ptr;                                      \
        *_p = (val);                                                                     \
    })

#define rt_time_cycle()                                                                  \
    ({                                                                                   \
        unsigned long tmp;                                                               \
        asm volatile("rcsr %0,4\n" : "=&r"(tmp)::"memory");                              \
        tmp;                                                                             \
    })

/****************************************************************************
 * CHECK functions
 ****************************************************************************/

/************align check *******************************/
#define rt_align_n(v, n) (((unsigned long long)(v)) % n == 0)
/*************floating point compare**********************/
#define __F_COM_EPS 1e-5
#define rt_equal_f(a, b) (fabs((a) - (b)) < __F_COM_EPS)
#define rt_nequal_f(a, b) (fabs((a) - (b)) >= __F_COM_EPS)

#define rt_equal_zero_f(a) (fabs(a) < __F_COM_EPS)
#define rt_nequal_zero_f(a) (fabs(a) >= __F_COM_EPS)

/****************************************************************************
 * PRINT functions
 ****************************************************************************/
#define __LOG_INFO_PREFIX "INFO"
#define __LOG_ERR_PREFIX "ERROR"

#define rt_log(format, ...)                                                              \
    do {                                                                                 \
        ai_printf("%s [%s %d]: %s => " format " \n", __LOG_INFO_PREFIX, __FILE__,        \
                  __LINE__, __FUNCTION__, ##__VA_ARGS__);                                \
    } while (0)

#define rt_log_if(cond, format, ...)                                                     \
    do {                                                                                 \
        if (cond)                                                                        \
            ai_printf("%s [%s %d]: %s => " format " \n", __LOG_INFO_PREFIX, __FILE__,    \
                      __LINE__, __FUNCTION__, ##__VA_ARGS__);                            \
    } while (0)

#define rt_warning(format, ...)                                                          \
    do {                                                                                 \
        ai_printf("%s [%s %d]: => " format " \n", __LOG_ERR_PREFIX, __FUNCTION__,        \
                  __LINE__, ##__VA_ARGS__);                                              \
    } while (0)

#define rt_warning_if(cond, format, ...)                                                 \
    do {                                                                                 \
        if (cond)                                                                        \
            ai_printf("%s [%s %d]: %s => " format " \n", __LOG_ERR_PREFIX, __FILE__,     \
                      __LINE__, __FUNCTION__, ##__VA_ARGS__);                            \
    } while (0)

/****************************************************************************
 * ACE functions
 ****************************************************************************/
#ifdef USE_BUILT_IN_ACE

#define rt_ace_config_kernel(Comptype) __builtin_sw_slave_ace_config(Comptype)

#define rt_ace_load_north(Waddr, Loadstride)                                             \
    __builtin_sw_slave_ace_load_north(Waddr, Loadstride)

#define rt_ace_load_west(Xaddr, Accumaddr, Accumflag, Xsize, Loadstride, Lastflag,       \
                         Precision, Northflag)                                           \
    __builtin_sw_slave_ace_load_west(Xaddr, Accumaddr, Accumflag, Xsize, Loadstride,     \
                                     Lastflag, Precision, Northflag)

#define rt_ace_writeback(Yaddr, Ysize, Yrply, Precisionflag)                             \
    do {                                                                                 \
        asm volatile("memb");                                                            \
        __builtin_sw_slave_ace_return(Yaddr, Ysize, Yrply, Precisionflag);               \
    } while (0)

#define rt_ace_barrier_kernel() __builtin_sw_slave_ace_barrier()

#else // use asm function

#define rt_ace_config_kernel(Comptype)                                                   \
    ({                                                                                   \
        uint64_t __rt_reg_config = (uint64_t)((((uint64_t)(Comptype)) & 0xfUL));         \
        __asm__ __volatile__("ace %0, 0x11\n" : : "r"(__rt_reg_config) : "memory");      \
        0;                                                                               \
    })

#define rt_ace_load_north(Waddr, Loadstride)                                             \
    ({                                                                                   \
        uint64_t __rt_reg_north =                                                        \
            (uint64_t)((((uint64_t)(Loadstride)) & 0xffffffffUL) << 32)                  \
            | ((((uint64_t)(Waddr)) & 0xfffffUL) >> 6);                                  \
        __asm__ __volatile__("ace %0, 0x12\n" : : "r"(__rt_reg_north) : "memory");       \
        0;                                                                               \
    })

#define rt_ace_load_west(Xaddr, Accumaddr, Accumflag, Xsize, Loadstride, Lastflag,       \
                         Precision, Northflag)                                           \
    ({                                                                                   \
        uint64_t __rt_reg_west = (uint64_t)((((uint64_t)(Northflag)) & 0x1UL) << 63)     \
                                 | ((((uint64_t)(Lastflag)) & 0x1UL) << 56)              \
                                 | ((((uint64_t)(Loadstride)) & 0xffUL) << 48)           \
                                 | ((((uint64_t)(Xsize)) & 0xffUL) << 32)                \
                                 | ((((uint64_t)(Accumflag)) & 0x1UL) << 23)             \
                                 | ((((uint64_t)(Accumaddr)) & 0x7fUL) << 16)            \
                                 | ((((uint64_t)(Xaddr)) & 0xfffffUL) >> 6);             \
        __asm__ __volatile__("ace %0, 0x13\n" : : "r"(__rt_reg_west) : "memory");        \
        0;                                                                               \
    })

#define rt_ace_writeback(Yaddr, Ysize, Yrply, Precisionflag)                             \
    ({                                                                                   \
        uint64_t __rt_reg_writeback =                                                    \
            (uint64_t)((((uint64_t)(Precisionflag)) & 0x1UL) << 63)                      \
            | ((((uint64_t)(Yrply)) & 0xfffffUL) << 32)                                  \
            | ((((uint64_t)(Ysize)) & 0xffUL) << 16)                                     \
            | ((((uint64_t)(Yaddr)) & 0xfffffUL) >> 6);                                  \
        __asm__ __volatile__("memb");                                                    \
        __asm__ __volatile__("ace %0, 0x14\n" : : "r"(__rt_reg_writeback) : "memory");   \
        0;                                                                               \
    })

#define rt_ace_barrier()                                                                 \
    ({                                                                                   \
        __asm__ __volatile__("ace $32, 0x18\n" : : : "memory", "$32");                   \
        0;                                                                               \
    })

#define __builtin_sw_slave_ace_config(Comptype) rt_ace_config_kernel(Comptype)

#define __builtin_sw_slave_ace_load_north(Waddr, Loadstride)                             \
    rt_ace_north_kernel(Waddr, Loadstride)

#define __builtin_sw_slave_ace_load_west(Xaddr, Accumaddr, Accumflag, Xsize, Loadstride, \
                                         Lastflag, HighLowFlag, Northflag)               \
    rt_ace_west_kernel(Xaddr, Accumaddr, Accumflag, Xsize, Loadstride, Lastflag,         \
                       Northflag)

#define __builtin_sw_slave_ace_return(Yaddr, Ysize, Yrply, Precisionflag)                \
    rt_ace_writeback_kernel(Yaddr, Ysize, Yrply, Precisionflag)

#define __builtin_sw_slave_ace_barrier() rt_ace_barrier_kernel()

#endif // USE_BUILT_IN_ACE

#define rt_ace_north_count()                                                             \
    ({                                                                                   \
        unsigned int tmp[2], index = 0;                                                  \
        asm volatile("rcsr %0,0x34\n" : "=&r"(tmp[0])::"memory");                        \
        asm volatile("rcsr %0,0x34\n" : "=&r"(tmp[1])::"memory");                        \
        while (tmp[0] != tmp[1]) {                                                       \
            index ^= 1;                                                                  \
            asm volatile("rcsr %0,0x34\n" : "=&r"(tmp[index])::"memory");                \
        }                                                                                \
        tmp[0];                                                                          \
    })

#define rt_ace_west_count()                                                              \
    ({                                                                                   \
        unsigned int tmp[2], index = 0;                                                  \
        asm volatile("rcsr %0,0x35\n" : "=&r"(tmp[0])::"memory");                        \
        asm volatile("rcsr %0,0x35\n" : "=&r"(tmp[1])::"memory");                        \
        while (tmp[0] != tmp[1]) {                                                       \
            index ^= 1;                                                                  \
            asm volatile("rcsr %0,0x35\n" : "=&r"(tmp[index])::"memory");                \
        }                                                                                \
        tmp[0];                                                                          \
    })

/* 等待ACE北向描述符完成 */
#define rt_wait_north(end, start)                                                        \
    do {                                                                                 \
        end = rt_ace_north_count();                                                      \
        if (end == start) break;                                                         \
    } while (1)

/* 等待ACE西向描述符完成*/
#define rt_wait_west(end, start)                                                         \
    do {                                                                                 \
        end = rt_ace_west_count();                                                       \
        if (end == start) break;                                                         \
    } while (1)

/****************************************************************************
 * Synchronize functions
 ****************************************************************************/

#define rt_synchronized_array()                                                          \
    ({                                                                                   \
        __asm__ __volatile__("ldi  $1,  0xff\n"                                          \
                             "synr $1       \n"                                          \
                             "ldi  $1,  0xf\n"                                           \
                             "sync $1       \n" ::                                       \
                                 : "memory", "$1");                                      \
        0;                                                                               \
    })

#define rt_synchronized_row()                                                            \
    ({                                                                                   \
        __asm__ __volatile__("ldi  $1,  0xff\n"                                          \
                             "synr $1       \n" ::                                       \
                                 : "memory", "$1");                                      \
        0;                                                                               \
    })

#define rt_synchronized_col()                                                            \
    ({                                                                                   \
        __asm__ __volatile__("ldi  $1,  0xf\n"                                           \
                             "sync $1       \n" ::                                       \
                                 : "memory", "$1");                                      \
        0;                                                                               \
    })

#define rt_synchronized_peer(tid)                                                        \
    ({                                                                                   \
        __asm__ __volatile__("ldi  $1,  0(%0)\n"                                         \
                             "synp $1       \n"                                          \
                             :                                                           \
                             : "r"(tid)                                                  \
                             : "memory", "$1");                                          \
        0;                                                                               \
    })

#define rt_synchronized_row_p2p(col_ssync_mask)                                          \
    ({                                                                                   \
        __asm__ __volatile__("ldi  $1,  0(%0)\n"                                         \
                             "synr $1       \n"                                          \
                             :                                                           \
                             : "r"(col_ssync_mask)                                       \
                             : "memory", "$1");                                          \
        0;                                                                               \
    })

#define rt_synchronized_col_p2p(row_ssync_mask)                                          \
    ({                                                                                   \
        __asm__ __volatile__("ldi  $1,  0(%0)\n"                                         \
                             "sync $1       \n"                                          \
                             :                                                           \
                             : "r"(row_ssync_mask)                                       \
                             : "memory", "$1");                                          \
        0;                                                                               \
    })

#define rt_synchronized_col_lefthalf()                                                   \
    ({                                                                                   \
        asm volatile("ldi  $1, 0x0f \n"                                                  \
                     "synr $1       \n"                                                  \
                     "ldi  $1,  0xf\n"                                                   \
                     "sync $1       \n" ::                                               \
                         : "memory", "$1");                                              \
        0;                                                                               \
    })

#define rt_synchronized_col_righthalf()                                                  \
    ({                                                                                   \
        asm volatile("ldi  $1, 0xf0 \n"                                                  \
                     "synr $1       \n"                                                  \
                     "ldi  $1,  0xf\n"                                                   \
                     "sync $1       \n" ::                                               \
                         : "memory", "$1");                                              \
        0;                                                                               \
    })

/****************************************************************************
 * Memory functions
 ****************************************************************************/
#ifdef ADD_TECOCOV

#ifdef USE_BUILT_IN_MEM

// Note that the
#define rt_dma(Mode, Op, Addr, Ldm_addr, Len, Rrply, Bcast_mask, Stride, Bsize)              \
    ({                                                                                       \
       asm volatile("memb");                                                                 \
       __builtin_sw_slave_athread_dma(Mode, Op, (void *)(Addr), (void *)(Ldm_addr),          \
	                               (int)(Len), (void *)(Rrply), (int)(Bcast_mask),       \
	                               (int)(Stride), (int)(Bsize));                         \
    })

#define rt_rma(Mode, Op, Remote_ldm_ddr, Ldm_addr, Len, Remote_rrply, Rrply, Bcast_mask, \
               Remote_id)                                                                \
    ({                                                                                   \
        asm volatile("memb");                                                                \
        __builtin_sw_slave_athread_rma(                                                      \
            Mode, Op, (void *)(Remote_ldm_ddr), (void *)(Ldm_addr), (int)(Len),              \
            (void *)(Remote_rrply), (void *)(Rrply), (int)(Bcast_mask), (int)(Remote_id));   \
    })

#else

#define rt_dma(Mode, Op, Addr, Ldm_addr, Len, Rrply, Bcast_mask, Stride, Bsize)          \
    ({                                                                                   \
	asm volatile("memb");                                                            \
        uint64_t __rt_reg_dma[2] __attribute__((aligned(64)));                           \
        __rt_reg_dma[1] = (uint64_t)(((((uint64_t)(Stride)) & 0xffffffffUL) << 32)       \
                                     | ((((uint64_t)(Bcast_mask)) & 0xffUL) << 24)       \
                                     | (((uint64_t)(Rrply)) & 0x3ffffUL));               \
        __rt_reg_dma[0] = (uint64_t)((((uint64_t)(Mode) & 0xfUL) << 60)                  \
                                     | (((uint64_t)(Op) & 0xfUL) << 56)                  \
                                     | (((uint64_t)(Bsize) & 0x3ffffUL) << 32)           \
                                     | ((uint64_t)(Len) & 0x3ffffUL));                   \
        __rt_reg_tmp0 = (uint64_t)(((uint64_t)(Addr) & 0xffffffffffffUL));               \
        __rt_reg_tmp1 = (uint64_t)(((uint64_t)(Ldm_addr)) & 0x3ffffUL);                  \
        __asm__ __volatile__("vldw $32,0(%0)\n"                                          \
                             "dma  $32,%1,%2 \n"                                         \
                             :                                                           \
                             : "r"(__rt_reg_dma), "r"(__rt_reg_tmp0), "r"(__rt_reg_tmp1) \
                             : "memory", "$32");                                         \
        0;                                                                               \
    })

#define rt_rma(Mode, Op, Remote_ldm_ddr, Ldm_addr, Len, Remote_rrply, Rrply, Bcast_mask, \
               Remote_id)                                                                \
    ({                                                                                   \
	asm volatile("memb");                                                            \
        uint64_t __rt_reg_dma[2] __attribute__((aligned(64)));                           \
        __rt_reg_dma[1] = (uint64_t)(((((uint64_t)(Remote_rrply)) & 0x3ffffUL) << 32)    \
                                     | ((((uint64_t)(Bcast_mask)) & 0xffUL) << 24)       \
                                     | (((uint64_t)(Rrply)) & 0x3ffffUL));               \
        __rt_reg_dma[0] = (uint64_t)((((uint64_t)(Mode) & 0xfUL) << 60)                  \
                                     | (((uint64_t)(Op) & 0xfUL) << 56)                  \
                                     | ((uint64_t)(Len) & 0x3ffffUL));                   \
        __rt_reg_tmp0 = (uint64_t)((((uint64_t)(Remote_id) & 0x3f) << 20)                \
                                   | ((uint64_t)(Remote_ldm_ddr) & 0x3ffffUL));          \
        __rt_reg_tmp1 = (uint64_t)(((uint64_t)(Ldm_addr)) & 0x3ffffUL);                  \
        __asm__ __volatile__("vldw $32,0(%0)\n"                                          \
                             "rma  $32,%1,%2 \n"                                         \
                             :                                                           \
                             : "r"(__rt_reg_dma), "r"(__rt_reg_tmp0), "r"(__rt_reg_tmp1) \
                             : "memory", "$32");                                         \
        0;                                                                               \
    })

#endif // USE_BUILT_IN_MEM

#else

#ifdef USE_BUILT_IN_MEM

// Note that the
#define rt_dma(Mode, Op, Addr, Ldm_addr, Len, Rrply, Bcast_mask, Stride, Bsize)          \
    __builtin_sw_slave_athread_dma(Mode, Op, (void *)(Addr), (void *)(Ldm_addr),         \
                                   (int)(Len), (void *)(Rrply), (int)(Bcast_mask),       \
                                   (int)(Stride), (int)(Bsize))

#define rt_rma(Mode, Op, Remote_ldm_ddr, Ldm_addr, Len, Remote_rrply, Rrply, Bcast_mask, \
               Remote_id)                                                                \
    __builtin_sw_slave_athread_rma(                                                      \
        Mode, Op, (void *)(Remote_ldm_ddr), (void *)(Ldm_addr), (int)(Len),              \
        (void *)(Remote_rrply), (void *)(Rrply), (int)(Bcast_mask), (int)(Remote_id))

#else

#define rt_dma(Mode, Op, Addr, Ldm_addr, Len, Rrply, Bcast_mask, Stride, Bsize)          \
    ({                                                                                   \
        uint64_t __rt_reg_dma[2] __attribute__((aligned(64)));                           \
        __rt_reg_dma[1] = (uint64_t)(((((uint64_t)(Stride)) & 0xffffffffUL) << 32)       \
                                     | ((((uint64_t)(Bcast_mask)) & 0xffUL) << 24)       \
                                     | (((uint64_t)(Rrply)) & 0x3ffffUL));               \
        __rt_reg_dma[0] = (uint64_t)((((uint64_t)(Mode) & 0xfUL) << 60)                  \
                                     | (((uint64_t)(Op) & 0xfUL) << 56)                  \
                                     | (((uint64_t)(Bsize) & 0x3ffffUL) << 32)           \
                                     | ((uint64_t)(Len) & 0x3ffffUL));                   \
        __rt_reg_tmp0 = (uint64_t)(((uint64_t)(Addr) & 0xffffffffffffUL));               \
        __rt_reg_tmp1 = (uint64_t)(((uint64_t)(Ldm_addr)) & 0x3ffffUL);                  \
        __asm__ __volatile__("vldw $32,0(%0)\n"                                          \
                             "dma  $32,%1,%2 \n"                                         \
                             :                                                           \
                             : "r"(__rt_reg_dma), "r"(__rt_reg_tmp0), "r"(__rt_reg_tmp1) \
                             : "memory", "$32");                                         \
        0;                                                                               \
    })

#define rt_rma(Mode, Op, Remote_ldm_ddr, Ldm_addr, Len, Remote_rrply, Rrply, Bcast_mask, \
               Remote_id)                                                                \
    ({                                                                                   \
        uint64_t __rt_reg_dma[2] __attribute__((aligned(64)));                           \
        __rt_reg_dma[1] = (uint64_t)(((((uint64_t)(Remote_rrply)) & 0x3ffffUL) << 32)    \
                                     | ((((uint64_t)(Bcast_mask)) & 0xffUL) << 24)       \
                                     | (((uint64_t)(Rrply)) & 0x3ffffUL));               \
        __rt_reg_dma[0] = (uint64_t)((((uint64_t)(Mode) & 0xfUL) << 60)                  \
                                     | (((uint64_t)(Op) & 0xfUL) << 56)                  \
                                     | ((uint64_t)(Len) & 0x3ffffUL));                   \
        __rt_reg_tmp0 = (uint64_t)((((uint64_t)(Remote_id) & 0x3f) << 20)                \
                                   | ((uint64_t)(Remote_ldm_ddr) & 0x3ffffUL));          \
        __rt_reg_tmp1 = (uint64_t)(((uint64_t)(Ldm_addr)) & 0x3ffffUL);                  \
        __asm__ __volatile__("vldw $32,0(%0)\n"                                          \
                             "rma  $32,%1,%2 \n"                                         \
                             :                                                           \
                             : "r"(__rt_reg_dma), "r"(__rt_reg_tmp0), "r"(__rt_reg_tmp1) \
                             : "memory", "$32");                                         \
        0;                                                                               \
    })

#endif // USE_BUILT_IN_MEM
#endif // ADD_TECOCOV

// typedef volatile int rt_rply_t;
// extern __thread_local_fix uint64_t reg_tmp[2];
// extern __thread_local_fix uint64_t RB;
// extern __thread_local_fix uint64_t RC;
// extern __thread_local_fix rt_rply_t rt_rply;

// __macro_rma(0x6, 0x0, a_recv_buf, a_keep_buf, buf_size * sizeof(_Float16),
// &remote_reply, &local_reply, 0xFF, 0);// 发送给同一行里的其他
#define RMA_MODE_SINGLE_CORE 0ULL
#define RMA_MODE_ROW_BCAST 1ULL
#define RMA_MODE_COL_BCAST 2ULL
#define RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA 5ULL
#define RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA 6ULL

#define RMA_OPCODE_PUT 0ULL
#define RMA_OPCODE_GET 1ULL
#define RMA_OPCODE_BARRIER 5ULL
#define RMA_OPCODE_ALL_BARRIER 6ULL

static __thread_local volatile int __rt_local_rply;
static __thread_local volatile int __rt_remote_rply;

// 阻塞get
#define rt_rma_get(l_addr, len, r_tid, r_addr, r_rply)                                   \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_rma(RMA_MODE_SINGLE_CORE, RMA_OPCODE_GET, (r_addr), (l_addr), (len),          \
               (r_rply), &__rt_local_rply, 0, (r_tid));                                  \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

// 阻塞put
#define rt_rma_put(l_addr, len, r_tid, r_addr, r_rply)                                   \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_rma(RMA_MODE_SINGLE_CORE, RMA_OPCODE_PUT, r_addr, l_addr, len, r_rply,        \
               &__rt_local_rply, 0, r_tid);                                              \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

// 非阻塞 get
#define rt_rma_iget(l_addr, l_rply, len, r_tid, r_addr, r_rply)                          \
    ({                                                                                   \
        rt_rma(RMA_MODE_SINGLE_CORE, RMA_OPCODE_GET, r_addr, l_addr, len, r_rply,        \
               l_rply, 0, r_tid);                                                        \
        0;                                                                               \
    })

// 非阻塞 put
#define rt_rma_iput(l_addr, l_rply, len, r_tid, r_addr, r_rply)                          \
    ({                                                                                   \
        rt_rma(RMA_MODE_SINGLE_CORE, RMA_OPCODE_PUT, r_addr, l_addr, len, r_rply,        \
               l_rply, 0, r_tid);                                                        \
        0;                                                                               \
    })

/* rma broadcast */
// 广播给所有 阻塞
#define rt_rma_bcast(dst, src, len, r_rply)                                              \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_rma(RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA, RMA_OPCODE_PUT, dst, src, len,     \
               r_rply, &__rt_local_rply, 0xff, rt_tid);                                  \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

// 广播给所有 非阻塞
#define rt_rma_ibcast(dst, src, l_rply, len, r_rply)                                     \
    ({                                                                                   \
        rt_rma(RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA, RMA_OPCODE_PUT, dst, src, len,     \
               r_rply, l_rply, 0xff, rt_tid);                                            \
        0;                                                                               \
    })

// 列广播的那个调 阻塞
#define rt_rma_col_bcast(dst, src, len, r_rply)                                          \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_rma(RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA, RMA_OPCODE_PUT, dst, src, len,     \
               r_rply, &__rt_local_rply, 1 << rt_cid, rt_tid);                           \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

// 列广播的那个调 非阻塞
#define rt_rma_col_ibcast(dst, src, len, l_rply, r_rply)                                 \
    ({                                                                                   \
        rt_rma(RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA, RMA_OPCODE_PUT, dst, src, len,     \
               r_rply, l_rply, 1 << rt_cid, rt_tid);                                     \
        0;                                                                               \
    })

// 列广播 所有都调 阻塞
#define rt_rma_col_bcast_coll(dst, src, len, col_root)                                   \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        __rt_remote_rply = 0;                                                            \
        rt_synchronized_col();                                                           \
        if (rt_rid == (col_root)) {                                                      \
            rt_rma(RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA, RMA_OPCODE_PUT, dst, src, len, \
                   &__rt_remote_rply, &__rt_local_rply, 1 << rt_cid, rt_tid);            \
        }                                                                                \
        while (__rt_remote_rply == 0)                                                    \
            ;                                                                            \
        rt_synchronized_col();                                                           \
        0;                                                                               \
    })

// 行广播的那个调 阻塞
#define rt_rma_row_bcast(dst, src, len, r_rply)                                          \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_rma(RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA, RMA_OPCODE_PUT, dst, src, len,     \
               r_rply, &__rt_local_rply, 1 << rt_rid, rt_tid);                           \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

// 行广播的那个调 非阻塞
#define rt_rma_row_ibcast(dst, src, len, l_rply, r_rply)                                 \
    ({                                                                                   \
        rt_rma(RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA, RMA_OPCODE_PUT, dst, src, len,     \
               r_rply, l_rply, 1 << rt_rid, rt_tid);                                     \
        0;                                                                               \
    })

// 行广播 所有都调 阻塞
#define rt_rma_row_bcast_coll(dst, src, len, row_root)                                   \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        __rt_remote_rply = 0;                                                            \
        rt_synchronized_row();                                                           \
        if (rt_cid == (row_root)) {                                                      \
            rt_rma(RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA, RMA_OPCODE_PUT, dst, src, len, \
                   &__rt_remote_rply, &__rt_local_rply, 1 << rt_rid, rt_tid);            \
        }                                                                                \
        while (__rt_remote_rply == 0)                                                    \
            ;                                                                            \
        rt_synchronized_row();                                                           \
        0;                                                                               \
    })

// 广播 阻塞 自己不接收
#define rt_rma_bcast_other(dst, src, len, r_rply)                                        \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_rma(RMA_MODE_COL_BCAST, RMA_OPCODE_PUT, dst, src, len, r_rply,                \
               &__rt_local_rply, 0xff, rt_tid);                                          \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

// 列广播的那个调 阻塞 自己不接收
#define rt_rma_col_bcast_other(dst, src, len, r_rply)                                    \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_rma(RMA_MODE_COL_BCAST, RMA_OPCODE_PUT, dst, src, len, r_rply,                \
               &__rt_local_rply, 1 << rt_cid, rt_tid);                                   \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

// 列广播的调 非阻塞 自己不接收
#define rt_rma_col_ibcast_other(dst, src, len, l_rply, r_rply)                           \
    ({                                                                                   \
        rt_rma(RMA_MODE_COL_BCAST, RMA_OPCODE_PUT, dst, src, len, r_rply, l_rply,        \
               1 << rt_cid, rt_tid);                                                     \
        0;                                                                               \
    })

// 列广播都调 阻塞 自己不接收
#define rt_rma_col_bcast_coll_other(dst, src, len, col_root)                             \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        __rt_remote_rply = 0;                                                            \
        rt_synchronized_col();                                                           \
        if (rt_rid == (col_root)) {                                                      \
            rt_rma(RMA_MODE_COL_BCAST, RMA_OPCODE_PUT, dst, src, len, &__rt_remote_rply, \
                   &__rt_local_rply, 1 << rt_cid, rt_tid);                               \
        } else {                                                                         \
            while (__rt_remote_rply == 0)                                                \
                ;                                                                        \
        }                                                                                \
        rt_synchronized_col();                                                           \
        0;                                                                               \
    })

// 行广播的调 阻塞 自己不收
#define rt_rma_row_bcast_other(dst, src, len, r_rply)                                    \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_rma(RMA_MODE_ROW_BCAST, RMA_OPCODE_PUT, dst, src, len, r_rply,                \
               &__rt_local_rply, 1 << rt_rid, rt_tid);                                   \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

// 行广播的调 非阻塞 自己不
#define rt_rma_row_ibcast_other(dst, src, len, l_rply, r_rply)                           \
    ({                                                                                   \
        rt_rma(RMA_MODE_ROW_BCAST, RMA_OPCODE_PUT, dst, src, len, r_rply, l_rply,        \
               1 << rt_rid, rt_tid);                                                     \
        0;                                                                               \
    })

// 行广播的都 阻塞 自己不
#define rt_rma_row_bcast_coll_other(dst, src, len, row_root)                             \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        __rt_remote_rply = 0;                                                            \
        rt_synchronized_row();                                                           \
        if (rt_cid == (row_root)) {                                                      \
            rt_rma(RMA_MODE_ROW_BCAST, RMA_OPCODE_PUT, dst, src, len, &__rt_remote_rply, \
                   &__rt_local_rply, 1 << rt_rid, rt_tid);                               \
        } else {                                                                         \
            while (__rt_remote_rply == 0)                                                \
                ;                                                                        \
        }                                                                                \
        rt_synchronized_row();                                                           \
        0;                                                                               \
    })

/* rma muticast */
// 列多 阻塞
#define rt_rma_col_mcast(dst, src, len, r_cols_mask, r_rply)                             \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_rma(RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA, RMA_OPCODE_PUT, dst, src, len,     \
               r_rply, &__rt_local_rply, r_cols_mask, rt_tid);                           \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

// 列多 非阻塞
#define rt_rma_col_imcast(dst, src, len, l_rply, r_cols_mask, r_rply)                    \
    ({                                                                                   \
        rt_rma(RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA, RMA_OPCODE_PUT, dst, src, len,     \
               r_rply, l_rply, r_cols_mask, rt_tid);                                     \
        0;                                                                               \
    })

// 行多 阻塞
#define rt_rma_row_mcast(dst, src, len, r_rows_mask, r_rply)                             \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_rma(RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA, RMA_OPCODE_PUT, dst, src, len,     \
               r_rply, &__rt_local_rply, r_rows_mask, rt_tid);                           \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

// 行多 非阻塞
#define rt_rma_row_imcast(dst, src, len, l_rply, r_rows_mask, r_rply)                    \
    ({                                                                                   \
        rt_rma(RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA, RMA_OPCODE_PUT, dst, src, len,     \
               r_rply, l_rply, r_rows_mask, rt_tid);                                     \
        0;                                                                               \
    })

// 行多 阻塞 自己不收
#define rt_rma_row_mcast_other(dst, src, len, r_rows_mask, r_rply)                       \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_rma(RMA_MODE_ROW_BCAST, RMA_OPCODE_PUT, dst, src, len, r_rply,                \
               &__rt_local_rply, r_rows_mask, rt_tid);                                   \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

// 列多 阻塞 自己不收
#define rt_rma_col_mcast_other(dst, src, len, r_cols_mask, r_rply)                       \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_rma(RMA_MODE_COL_BCAST, RMA_OPCODE_PUT, dst, src, len, r_rply,                \
               &__rt_local_rply, r_cols_mask, rt_tid);                                   \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

// 行多 非阻塞 自己不收
#define rt_rma_row_imcast_other(dst, src, len, l_rply, r_rows_mask, r_rply)              \
    ({                                                                                   \
        rt_rma(RMA_MODE_ROW_BCAST, RMA_OPCODE_PUT, dst, src, len, r_rply, l_rply,        \
               r_rows_mask, rt_tid);                                                     \
        0;                                                                               \
    })

// 列多 非阻塞 自己不收
#define rt_rma_col_imcast_other(dst, src, len, l_rply, r_cols_mask, r_rply)              \
    ({                                                                                   \
        rt_rma(RMA_MODE_COL_BCAST, RMA_OPCODE_PUT, dst, src, len, r_rply, l_rply,        \
               r_cols_mask, rt_tid);                                                     \
        0;                                                                               \
    })

// rma回答字等待
#define rt_rma_wait_value(reply, value)                                                  \
    ({                                                                                   \
        while (*(volatile int *)(reply) < value)                                         \
            ;                                                                            \
        0;                                                                               \
    })

/* dma */
#define DMA_OP_PUT 0ULL
#define DMA_OP_GET 1ULL
#define DMA_PUT_PHYSICAL 2ULL
#define DMA_GET_PHYSICAL 3ULL
#define DMA_OP_BARRIER 5ULL
#define DMA_OP_ALL_BARRIER 6ULL
#define DMA_OP_GET_TRANSPOSE 9ULL

#define DMA_MODE_SINGLE_CORE 0
#define DMA_MODE_ROW_BCAST 1
#define DMA_MODE_COL_BCAST 2

// 阻塞
#ifdef ADD_TECOCOV
#define rt_dma_get(dest, src, len)                                                       \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        asm volatile("memb"); \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_GET, (src), (dest), (len),                   \
               (void *)&__rt_local_rply, 0, 0, 0);                                       \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })
#else
#define rt_dma_get(dest, src, len)                                                       \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_GET, (src), (dest), (len),                   \
               (void *)&__rt_local_rply, 0, 0, 0);                                       \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })
#endif

// 阻塞 带跨步
#ifdef ADD_TECOCOV
#define rt_dma_get_stride(dest, src, len, bsize, stride)                                 \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        asm volatile("memb"); \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_GET, (src), (dest), (len),                   \
               (void *)&__rt_local_rply, 0, stride, bsize);                              \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })
#else
#define rt_dma_get_stride(dest, src, len, bsize, stride)                                 \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_GET, (src), (dest), (len),                   \
               (void *)&__rt_local_rply, 0, stride, bsize);                              \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })
#endif

// 非阻塞
#define rt_dma_iget(dest, src, len, reply)                                               \
    ({                                                                                   \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_GET, src, dest, len, (void *)(reply), 0, 0,  \
               0);                                                                       \
        0;                                                                               \
    })

// 非阻塞 带跨步
#define rt_dma_iget_stride(dest, src, len, bsize, stride, reply)                         \
    ({                                                                                   \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_GET, src, dest, len, (void *)(reply), 0,     \
               stride, bsize);                                                           \
        0;                                                                               \
    })

// 非阻塞
#define rt_dma_iput(dest, src, len, reply)                                               \
    ({                                                                                   \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_PUT, dest, src, len, (void *)(reply), 0, 0,  \
               0);                                                                       \
        0;                                                                               \
    })

// 非阻塞 跨步
#define rt_dma_iput_stride(dest, src, len, bsize, stride, reply)                         \
    ({                                                                                   \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_PUT, dest, src, len, (void *)(reply), 0,     \
               stride, bsize);                                                           \
        0;                                                                               \
    })

// 阻塞
#ifdef ADD_TECOCOV
#define rt_dma_put(dest, src, len)                                                       \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        asm volatile("memb"); \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_PUT, (dest), (src), (len),                   \
               (void *)&__rt_local_rply, 0, 0, 0);                                       \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })
#else
#define rt_dma_put(dest, src, len)                                                       \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_PUT, (dest), (src), (len),                   \
               (void *)&__rt_local_rply, 0, 0, 0);                                       \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })
#endif

// 阻塞 带跨步
#ifdef ADD_TECOCOV
#define rt_dma_put_stride(dest, src, len, bsize, stride)                                 \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        asm volatile("memb"); \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_PUT, dest, src, len,                         \
               (void *)&__rt_local_rply, 0, stride, bsize);                              \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })
#else
#define rt_dma_put_stride(dest, src, len, bsize, stride)                                 \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_PUT, dest, src, len,                         \
               (void *)&__rt_local_rply, 0, stride, bsize);                              \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })
#endif

/* DMA functions using physical address */

#define rt_dma_phy_get(dest, src, len)                                                   \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_GET_PHYSICAL, src, dest, len,                   \
               (void *)&__rt_local_rply, 0, 0, 0);                                       \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

#define rt_dma_phy_get_stride(dest, src, len, bsize, stride)                             \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_GET_PHYSICAL, (src), (dest), (len),             \
               (void *)&__rt_local_rply, 0, stride, bsize);                              \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

#define rt_dma_phy_iget(dest, src, len, reply)                                           \
    ({                                                                                   \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_GET_PHYSICAL, src, dest, len, (void *)(reply),  \
               0, 0, 0);                                                                 \
        0;                                                                               \
    })

#define rt_dma_phy_iget_stride(dest, src, len, bsize, stride, reply)                     \
    ({                                                                                   \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_GET_PHYSICAL, src, dest, len, (void *)(reply),  \
               0, stride, bsize);                                                        \
        0;                                                                               \
    })

#define rt_dma_phy_iput(dest, src, len, reply)                                           \
    ({                                                                                   \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_PUT_PHYSICAL, dest, src, len, (void *)(reply),  \
               0, 0, 0);                                                                 \
        0;                                                                               \
    })

#define rt_dma_phy_iput_stride(dest, src, len, bsize, stride, reply)                     \
    ({                                                                                   \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_PUT_PHYSICAL, dest, src, len, (void *)(reply),  \
               0, stride, bsize);                                                        \
        0;                                                                               \
    })

#define rt_dma_phy_put(dest, src, len)                                                   \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_PUT_PHYSICAL, (dest), (src), (len),             \
               (void *)&__rt_local_rply, 0, 0, 0);                                       \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

#define rt_dma_phy_put_stride(dest, src, len, bsize, stride)                             \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_PUT_PHYSICAL, dest, src, len,                   \
               (void *)&__rt_local_rply, 0, stride, bsize);                              \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

#define rt_s_dma_trans_get(dest, src, stride)                                            \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_GET_TRANSPOSE, src, dest, 1024,              \
               (void *)&__rt_local_rply, 0, stride, 64);                                 \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

#define rt_h_dma_trans_get(dest, src, stride)                                            \
    ({                                                                                   \
        __rt_local_rply = 0;                                                             \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_GET_TRANSPOSE, src, dest, 2048,              \
               (void *)&__rt_local_rply, 0, stride, 64);                                 \
        while (__rt_local_rply == 0)                                                     \
            ;                                                                            \
        0;                                                                               \
    })

#define rt_s_dma_trans_iget(dest, src, stride, reply)                                    \
    ({                                                                                   \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_GET_TRANSPOSE, src, dest, 1024,              \
               (void *)(reply), 0, stride, 64);                                          \
        0;                                                                               \
    })

#define rt_h_dma_trans_iget(dest, src, stride, reply)                                    \
    ({                                                                                   \
        rt_dma(DMA_MODE_SINGLE_CORE, DMA_OP_GET_TRANSPOSE, src, dest, 2048,              \
               (void *)(reply), 0, stride, 64);                                          \
        0;                                                                               \
    })

#define rt_dma_ibcast(dest, src, len, reply)                                             \
    ({                                                                                   \
        rt_dma(DMA_MODE_COL_BCAST, DMA_OP_GET, src, dest, len, (void *)(reply), 0xff, 0, \
               0);                                                                       \
        0;                                                                               \
    })

#define rt_s_dma_trans_ibcast(dest, src, stride, reply)                                  \
    ({                                                                                   \
        rt_dma(DMA_MODE_COL_BCAST, DMA_OP_GET_TRANSPOSE, src, dest, 1024,                \
               (void *)(reply), 0xff, stride, 64);                                       \
        0;                                                                               \
    })

#define rt_h_dma_trans_ibcast(dest, src, stride, reply)                                  \
    ({                                                                                   \
        rt_dma(DMA_MODE_COL_BCAST, DMA_OP_GET_TRANSPOSE, src, dest, 2048,                \
               (void *)(reply), 0xff, stride, 64);                                       \
        0;                                                                               \
    })

#define rt_dma_ibcast_stride(dest, src, len, bsize, stride, reply)                       \
    ({                                                                                   \
        rt_dma(DMA_MODE_COL_BCAST, DMA_OP_GET, src, dest, len, (void *)(reply), 0xff,    \
               stride, bsize);                                                           \
        0;                                                                               \
    })

#define rt_dma_col_ibcast(dest, src, len, reply)                                         \
    ({                                                                                   \
        rt_dma(DMA_MODE_COL_BCAST, DMA_OP_GET, src, dest, len, (void *)reply,            \
               1 << rt_cid, 0, 0);                                                       \
        0;                                                                               \
    })

#define rt_s_dma_trans_col_ibcast(dest, src, stride, reply)                              \
    ({                                                                                   \
        rt_dma(DMA_MODE_COL_BCAST, DMA_OP_GET_TRANSPOSE, src, dest, 1024, (void *)reply, \
               1 << rt_cid, stride, 64);                                                 \
        0;                                                                               \
    })

#define rt_h_dma_trans_col_ibcast(dest, src, stride, reply)                              \
    ({                                                                                   \
        rt_dma(DMA_MODE_COL_BCAST, DMA_OP_GET_TRANSPOSE, src, dest, 2048, (void *)reply, \
               1 << rt_cid, stride, 64);                                                 \
        0;                                                                               \
    })

#define rt_dma_col_ibcast_stride(dest, src, len, bsize, stride, reply)                   \
    ({                                                                                   \
        rt_dma(DMA_MODE_COL_BCAST, DMA_OP_GET, src, dest, len, (void *)reply,            \
               1 << rt_cid, stride, bsize);                                              \
        0;                                                                               \
    })

#define rt_dma_row_ibcast(dest, src, len, reply)                                         \
    ({                                                                                   \
        rt_dma(DMA_MODE_ROW_BCAST, DMA_OP_GET, src, dest, len, (void *)reply,            \
               1 << rt_rid, 0, 0);                                                       \
        0;                                                                               \
    })

#define rt_s_dma_trans_row_ibcast(dest, src, stride, reply)                              \
    ({                                                                                   \
        rt_dma(DMA_MODE_ROW_BCAST, DMA_OP_GET_TRANSPOSE, src, dest, 1024, (void *)reply, \
               1 << rt_rid, stride, 64);                                                 \
        0;                                                                               \
    })

#define rt_h_dma_trans_row_ibcast(dest, src, stride, reply)                              \
    ({                                                                                   \
        rt_dma(DMA_MODE_ROW_BCAST, DMA_OP_GET_TRANSPOSE, src, dest, 2048, (void *)reply, \
               1 << rt_rid, stride, 64);                                                 \
        0;                                                                               \
    })

#define rt_dma_row_ibcast_stride(dest, src, len, bsize, stride, reply)                   \
    ({                                                                                   \
        rt_dma(DMA_MODE_ROW_BCAST, DMA_OP_GET, src, dest, len, (void *)reply,            \
               1 << rt_rid, stride, bsize);                                              \
        0;                                                                               \
    })

#define rt_dma_wait_value(reply, value)                                                  \
    ({                                                                                   \
        while (*(volatile int *)(reply) < value)                                         \
            ;                                                                            \
        0;                                                                               \
    })
/****************************************************************
 *  macros for ldm allocation
 ***************************************************************/
#ifdef LDM_MALLOC_MEMSET
#include <stdlib.h>
#define __safe_ldm_malloc(size, section, _filename, _lineno)                             \
    ({                                                                                   \
        void *ptr = (void *)malloc(size, AddressLowToHigh);                                   \
        if (ptr == NULL || (int64_t)ptr < 0) {                                           \
            ai_printf("[%s:%d] ldm malloc failed, size %u\n", _filename, _lineno, size); \
            return;                                                                      \
        }                                                                                \
        memset(ptr, 0, size);                                                            \
        ptr;                                                                             \
    })
#elif defined(CHECK_LDM_OVERFLOW) // CHECK LDM OVERFLOW
#define __safe_ldm_malloc(size, section, _filename, _lineno)                             \
    ({                                                                                   \
        void *ptr = (void *)malloc((size) + 128, section);                     \
        void *real_ptr = NULL;                                                           \
        if (ptr == NULL || (int64_t)ptr < 0) {                                           \
            ai_printf("[%s:%d] ldm malloc failed, size %u\n", _filename, _lineno, size); \
            return;                                                                      \
        } else {                                                                         \
            for (int __ik = 0; __ik < 64; __ik++) {                                      \
                ((unsigned char *)ptr)[__ik] = 0xFF;                                     \
                ((unsigned char *)ptr)[(size) + 64 + __ik] = 0xFF;                       \
            }                                                                            \
            ((size_t *)ptr)[0] = size;                                                   \
            real_ptr = (void *)((unsigned long)ptr + 64);                                \
        }                                                                                \
        real_ptr;                                                                        \
    })
#else // LDM_MALLOC_MEMSET
#define __safe_ldm_malloc(size, section, _filename, _lineno)                             \
    ({                                                                                   \
        void *ptr = (void *)malloc(size, section);                             \
        if (ptr == NULL || (int64_t)ptr < 0) {                                           \
            ai_printf("[%s:%d] ldm malloc failed, size %u\n", _filename, _lineno, size); \
            return;                                                                      \
        }                                                                                \
        ptr;                                                                             \
    })
#endif // LDM_MALLOC_MEMSET
#define rt_ldm_malloc_try_left(size) __safe_ldm_malloc(size, AddressLowToHigh, __FILE__, __LINE__)
#define rt_ldm_malloc_try_right(size) __safe_ldm_malloc(size, AddressHighToLow, __FILE__, __LINE__)
#define rt_ldm_malloc(size) rt_ldm_malloc_try_left(size)

#if defined(CHECK_LDM_OVERFLOW)
#define __safe_ldm_free(ptr, _filename, _lineno)                                         \
    ({                                                                                   \
        if (ptr != NULL) {                                                               \
            int __sum_pre = 0, __sum_suf = 0;                                            \
            constexpr int pre_count = 64 - sizeof(size_t);                               \
            constexpr int suf_count = 64;                                                \
            size_t size = ((size_t *)ptr)[-(64 / sizeof(size_t))];                       \
            for (int __ik = -pre_count; __ik < 0; __ik++) {                              \
                __sum_pre += ((unsigned char *)ptr)[__ik];                               \
            }                                                                            \
            for (int __ik = 0; __ik < suf_count; __ik++) {                               \
                __sum_suf += ((unsigned char *)ptr)[size + __ik];                        \
            }                                                                            \
            if (__sum_pre != ((unsigned char)0xFF) * pre_count                           \
                || __sum_suf != ((unsigned char)0xFF) * suf_count) {                     \
                ai_printf(                                                               \
                    "[%s:%d]: ERROR!!!!!!! ldm overflow detected ptr %p size %lu\n",     \
                    _filename, _lineno, ptr, size);                                      \
                unsigned int tmp;                                                        \
                RAISE_EXCEPTION;                                                         \
            }                                                                            \
            free(((unsigned char *)ptr) - 64);                                 \
        }                                                                                \
    })
#else // CHECK LDM OVERFLOW
#define __safe_ldm_free(ptr, _filename, _lineno)                                         \
    ({                                                                                   \
        if (ptr != NULL) {                                                               \
            free((void *)ptr);                                                 \
        }                                                                                \
    })
#endif
#define rt_ldm_free(ptr) __safe_ldm_free(ptr, __FILE__, __LINE__);

/****************************************************************
 *  collective rma
 ***************************************************************/
static __device__ void rt_rma_allreduce(void *data, void *buf, int len, RED_TYPE type);
static __device__ void rt_rma_reduce(void *data, void *buf, int len, RED_TYPE type);
static __device__ void rt_rma_allreduce_to(int core_id, void *data, void *buf, int len,
                                           RED_TYPE type);
static __device__ void rt_rma_row_reduce(void *data, void *buf, int len, RED_TYPE type);
static __device__ void rt_rma_row_reduce_to(int core_id, void *data, void *buf, int len,
                                            RED_TYPE type);
static __device__ void rt_rma_col_reduce(void *data, void *buf, int len, RED_TYPE type);
static __device__ void rt_rma_col_reduce_to(int core_id, void *data, void *buf, int len,
                                            RED_TYPE type);
static __device__ void rt_unaligned_dma_get(void *dest, void *src, int len);
static __device__ void rt_unaligned_dma_put(void *dest, void *src, int len);
static __device__ void rt_unaligned_dma_get_stride(void *dest, void *src, int len,
                                                   int bsize, int stride);
static __device__ void rt_unaligned_dma_put_stride(void *dest, void *src, int len,
                                                   int bsize, int stride);
static __device__ void rt_memcpy(void *dest, void *src, int len);
static __device__ void rt_memmove(void *dest, void *src, int len);

#include "./rt.hpp"

// #endif  // __sw_slave__

#ifdef __cplusplus
}
#endif

#endif  // TECOOPS_UAL_COM_RT_H_