#pragma once

#include <cstdint>
#include "LiteMath.h"
#include "TriRaster.h"        

#include "HWPureCpp.h"
#include "HWBlock.h"

//@select current implementation here via typedef assigment :)

//typedef HWImplementationPureCpp HWImpl;
//typedef HWImpl_SSE1 HWImpl;
//typedef HWImplBlockLine4x4_CVEX HWImpl;
//typedef HWImplBlockLine8x2_CVEX HWImpl; // does not works with binned FB currently due to we removed FB_BILLET
typedef HWImplBlock16x1_CVEX HWImpl;

//typedef HWImplBlockLine4x4Fixp_CVEX HWImpl;

using Triangle = HWImpl::TriangleType;
