//
// Created by frol on 28.10.18.
//

#include "PureCpp.h"
#include "SimdBlockFixp.h"
#include "PureCppBlock4x4.h"

#ifdef WIN32
#undef min
#undef max
#endif

#include <algorithm>

using TriangleLocal = HWImplementationPureCpp::TriangleType;

void HWImplBlock4x4_Fixp::RasterizeTriangle(RasterOp a_ropT, BlendOp a_bopT, const TriangleType& tri, int tileMinX, int tileMinY,
                                            FrameBuffer* frameBuf)
{
  RasterizeTriHalfSpaceBlockFixp2D<TriangleType, 4>(tri, tileMinX, tileMinY,
                                                    frameBuf);
}