//
// Created by frol on 29.10.18.
//

#ifndef TEST_GL_TOP_HW_ROP_SIMDPP_H
#define TEST_GL_TOP_HW_ROP_SIMDPP_H

#define  	SIMDPP_ARCH_X86_AVX
#include "../simdpp/simd.h"

inline static unsigned int RealColorToUint32_BGRA_SIMD(const simdpp::float32<4>& real_color)
{
  static const simdpp::float32<4> const_255 = simdpp::make_float(255.0f);
  static const simdpp::uint32<4>  shiftmask = simdpp::make_int(16,8,0,24);
  const  simdpp::uint32<4>        rgbai     = simdpp::to_int32(real_color*const_255);
  return simdpp::reduce_or(simdpp::shift_l(rgbai, shiftmask)); // return blue | (green << 8) | (red << 16) | (alpha << 24);
}


#endif //TEST_GL_TOP_HW_ROP_SIMDPP_H
