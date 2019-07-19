#pragma once

#include <cstdint>
#include "LiteMath.h"
#include "TriRaster.h"        

#include "HWPureCpp.h"
#include "HWSSE1Impl.h"
#include "HWBlockLine4x4.h"

//@select current implementation here via typedef assigment :)
//typedef HWImplementationPureCpp HWImpl;
typedef HWImpl_SSE1 HWImpl;
//typedef HWImplBlockLine4x4_CVEX HWImpl;
//typedef HWImplBlockLine8x8_CVEX HWImpl;

//typedef HWImplBlockLine4x4Fixp_CVEX HWImpl;

using Triangle = HWImpl::TriangleType;
