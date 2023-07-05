/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm-cl/vector.h>

using namespace cm;

namespace {

// Value are taken from LSC_OP enum
// Source/visa/include/visa_igc_common_header.h
enum class AtomicOp : char {
  Inc = 0x08,
  Dec = 0x09,
  Load = 0x0A,
  Xchg = 0x0B,
  Add = 0x0C,
  Sub = 0x0D,
  SMin = 0x0E,
  SMax = 0x0F,
  UMin = 0x10,
  UMax = 0x11,
  Cas = 0x12,
  Fadd = 0x13,
  Fsub = 0x14,
  Fmin = 0x15,
  Fmax = 0x16,
  Fcas = 0x17,
  And = 0x18,
  Or = 0x19,
  Xor = 0x1A,
};

template <int N>
CM_NODEBUG CM_INLINE vector<uint64_t, N>
__impl_atomic_local(mask<N> pred, AtomicOp op, char l1cachecontrol,
                    char l3cachecontrol, int base, vector<int, N> index,
                    short scale, int offset, vector<uint64_t, N> src1,
                    vector<uint64_t, N> src2, vector<uint64_t, N> passthru) {
  vector<int, N> addr = base + index * scale + offset;
  vector<uint64_t, N> laddr = addr;
  vector<uint64_t, N> orig =
      detail::__cm_cl_gather(3, laddr.cl_vector(), sizeof(uint64_t),
                             pred.cl_vector(), passthru.cl_vector());

  // Value should be equal to LSC_ADDR_SIZE_32b from
  // Source/visa/include/visa_igc_common_header.h
  constexpr char AddrSize = 2;

  // Value should be equal to LSC_DATA_SIZE_64b from
  // Source/visa/include/visa_igc_common_header.h
  constexpr char DataSize = 4;

  do {
    vector<uint64_t, N> newval = orig;
    switch (op) {
    case AtomicOp::Add:
      newval += src1;
      break;
    case AtomicOp::Sub:
      newval -= src1;
      break;
    case AtomicOp::And:
      newval &= src1;
      break;
    case AtomicOp::Or:
      newval |= src1;
      break;
    case AtomicOp::Xor:
      newval ^= src1;
      break;
    case AtomicOp::Xchg:
      newval = src1;
      break;
    case AtomicOp::SMin: {
      vector<int64_t, N> ssrc1 = src1.template format<int64_t>();
      vector<int64_t, N> snewval = newval.template format<int64_t>();
      newval.merge(src1, ssrc1 < snewval);
    } break;
    case AtomicOp::SMax: {
      vector<int64_t, N> ssrc1 = src1.template format<int64_t>();
      vector<int64_t, N> snewval = newval.template format<int64_t>();
      newval.merge(src1, ssrc1 > snewval);
    } break;
    case AtomicOp::UMin:
      newval.merge(src1, src1 < newval);
      break;
    case AtomicOp::UMax:
      newval.merge(src1, src1 > newval);
      break;
    case AtomicOp::Inc:
      newval = newval + 1;
      break;
    case AtomicOp::Dec:
      newval = newval - 1;
      break;
    case AtomicOp::Load:
      break;
    default:
      break;
    }

    vector<uint64_t, N> res = detail::__cm_cl_vector_atomic_slm(
        pred.cl_vector(), static_cast<char>(AtomicOp::Cas), AddrSize, DataSize,
        l1cachecontrol, l3cachecontrol, 0, addr.cl_vector(), 1, 0,
        orig.cl_vector(), newval.cl_vector(), orig.cl_vector());
    pred &= res != orig;
    orig = res;
  } while (pred.any());

  return orig;
}

template <int N>
CM_NODEBUG CM_INLINE vector<double, N>
__impl_atomic_global(mask<N> pred, AtomicOp op, char l1cachecontrol,
                     char l3cachecontrol, long base, vector<long, N> index,
                     short scale, int offset, vector<double, N> src1,
                     vector<double, N> src2, vector<double, N> passthru) {
  vector<long, N> addr = base + index * scale + offset;
  vector<uint64_t, N> laddr = addr;
  vector<double, N> orig =
      detail::__cm_cl_gather(1, laddr.cl_vector(), sizeof(double),
                             pred.cl_vector(), passthru.cl_vector());

  // Value should be equal to LSC_ADDR_SIZE_64b from
  // Source/visa/include/visa_igc_common_header.h
  constexpr char AddrSize = 3;

  // Value should be equal to LSC_DATA_SIZE_64b from
  // Source/visa/include/visa_igc_common_header.h
  constexpr char DataSize = 4;

  do {
    vector<double, N> newval = orig;
    switch (op) {
    case AtomicOp::Fadd:
      newval += src1;
      break;
    case AtomicOp::Fsub:
      newval -= src1;
      break;
    case AtomicOp::Fmin:
      newval.merge(src1, src1 < newval);
      break;
    case AtomicOp::Fmax:
      newval.merge(src1, src1 > newval);
      break;
    case AtomicOp::Fcas:
      newval.merge(src2, src1 == newval);
      break;
    default:
      break;
    }

    vector<uint64_t, N> iorig = orig.template format<uint64_t>();
    vector<uint64_t, N> inewval = newval.template format<uint64_t>();

    vector<uint64_t, N> res = detail::__cm_cl_vector_atomic_ugm(
        pred.cl_vector(), static_cast<char>(AtomicOp::Cas), AddrSize, DataSize,
        l1cachecontrol, l3cachecontrol, 0, addr.cl_vector(), 1, 0,
        iorig.cl_vector(), inewval.cl_vector(), iorig.cl_vector());
    vector<double, N> fres = res.template format<double>();
    pred &= fres != orig;
    orig = fres;
  } while (pred.any());

  return orig;
}

template <int N>
CM_NODEBUG CM_INLINE vector<uint32_t, N>
__impl_atomic_global(mask<N> pred, AtomicOp op, char l1cachecontrol,
                     char l3cachecontrol, long base, vector<long, N> index,
                     short scale, int offset, vector<uint32_t, N> src1,
                     vector<uint32_t, N> src2, vector<uint32_t, N> passthru) {
  vector<long, N> addr = base + index * scale + offset;
  vector<uint64_t, N> laddr = addr;

  vector<half, N> hpassthru =
      passthru.template format<half>().template select<N, 2>(0);

  vector<half, N> orig =
      detail::__cm_cl_gather(1, laddr.cl_vector(), sizeof(half),
                             pred.cl_vector(), hpassthru.cl_vector());

  vector<uint32_t, N> iorig;

  // Value should be equal to LSC_ADDR_SIZE_64b from
  // Source/visa/include/visa_igc_common_header.h
  constexpr char AddrSize = 3;

  // Value should be equal to LSC_DATA_SIZE_16c32b from
  // Source/visa/include/visa_igc_common_header.h
  constexpr char DataSize = 6;

  vector<half, N> hsrc = src1.template format<half>().template select<N, 2>(0);

  do {
    vector<half, N> newval = orig;
    switch (op) {
    case AtomicOp::Fadd:
      newval += hsrc;
      break;
    case AtomicOp::Fsub:
      newval -= hsrc;
      break;
    default:
      break;
    }

    iorig.template format<half>().template select<N, 2>(0) = orig;

    vector<uint32_t, N> inewval;
    inewval.template format<half>().template select<N, 2>(0) = newval;

    vector<uint32_t, N> res = detail::__cm_cl_vector_atomic_ugm(
        pred.cl_vector(), static_cast<char>(AtomicOp::Cas), AddrSize, DataSize,
        l1cachecontrol, l3cachecontrol, 0, addr.cl_vector(), 1, 0,
        iorig.cl_vector(), inewval.cl_vector(), iorig.cl_vector());

    vector<half, N> hres = res.template format<half>().template select<N, 2>(0);

    pred &= hres != orig;
    orig = hres;
  } while (pred.any());

  return iorig;
}

} // namespace

#define ATOMIC(WIDTH)                                                          \
  CM_NODEBUG CM_INLINE extern "C" cl_vector<uint64_t, WIDTH>                   \
      __vc_builtin_atomic_slm_v##WIDTH##i64(                                   \
          cl_vector<char, WIDTH> pred, AtomicOp op, char l1cachecontrol,       \
          char l3cachecontrol, int base, cl_vector<int, WIDTH> index,          \
          short scale, int offset, cl_vector<uint64_t, WIDTH> src1,            \
          cl_vector<uint64_t, WIDTH> src2,                                     \
          cl_vector<uint64_t, WIDTH> passthru) {                               \
    mask<WIDTH> vpred{pred};                                                   \
    vector<int, WIDTH> vindex{index};                                          \
    vector<uint64_t, WIDTH> vsrc1{src1};                                       \
    vector<uint64_t, WIDTH> vsrc2{src2};                                       \
    vector<uint64_t, WIDTH> vpassthru{passthru};                               \
    return __impl_atomic_local<WIDTH>(vpred, op, l1cachecontrol,               \
                                      l3cachecontrol, base, vindex, scale,     \
                                      offset, vsrc1, vsrc2, vpassthru)         \
        .cl_vector();                                                          \
  }                                                                            \
  CM_NODEBUG CM_INLINE extern "C" cl_vector<double, WIDTH>                     \
      __vc_builtin_atomic_ugm_v##WIDTH##f64(                                   \
          cl_vector<char, WIDTH> pred, AtomicOp op, char l1cachecontrol,       \
          char l3cachecontrol, long base, cl_vector<long, WIDTH> index,        \
          short scale, int offset, cl_vector<double, WIDTH> src1,              \
          cl_vector<double, WIDTH> src2, cl_vector<double, WIDTH> passthru) {  \
    mask<WIDTH> vpred{pred};                                                   \
    vector<long, WIDTH> vindex{index};                                         \
    vector<double, WIDTH> vsrc1{src1};                                         \
    vector<double, WIDTH> vsrc2{src2};                                         \
    vector<double, WIDTH> vpassthru{passthru};                                 \
    return __impl_atomic_global<WIDTH>(vpred, op, l1cachecontrol,              \
                                       l3cachecontrol, base, vindex, scale,    \
                                       offset, vsrc1, vsrc2, vpassthru)        \
        .cl_vector();                                                          \
  }                                                                            \
  CM_NODEBUG CM_INLINE extern "C" cl_vector<uint32_t, WIDTH>                   \
      __vc_builtin_atomic_ugm_v##WIDTH##i32(                                   \
          cl_vector<char, WIDTH> pred, AtomicOp op, char l1cachecontrol,       \
          char l3cachecontrol, long base, cl_vector<long, WIDTH> index,        \
          short scale, int offset, cl_vector<uint32_t, WIDTH> src1,            \
          cl_vector<uint32_t, WIDTH> src2,                                     \
          cl_vector<uint32_t, WIDTH> passthru) {                               \
    mask<WIDTH> vpred{pred};                                                   \
    vector<long, WIDTH> vindex{index};                                         \
    vector<uint32_t, WIDTH> vsrc1{src1};                                       \
    vector<uint32_t, WIDTH> vsrc2{src2};                                       \
    vector<uint32_t, WIDTH> vpassthru{passthru};                               \
    return __impl_atomic_global<WIDTH>(vpred, op, l1cachecontrol,              \
                                       l3cachecontrol, base, vindex, scale,    \
                                       offset, vsrc1, vsrc2, vpassthru)        \
        .cl_vector();                                                          \
  }

ATOMIC(1)
ATOMIC(2)
ATOMIC(4)
ATOMIC(8)
ATOMIC(16)
ATOMIC(32)