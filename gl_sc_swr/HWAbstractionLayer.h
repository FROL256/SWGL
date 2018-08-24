#pragma once

#include <cstdint>
#include "LiteMath.h"

#include "TriRaster.h"        

#include "PureCpp.h"
#include "SSE1Impl.h"

#include "PureCppBlock2x2.h"
#include "PureCppBlock4x4.h"

//@select current implementation here via typedef assigment :)
//typedef HWImplementationPureCpp HWImpl;
//typedef HWImpl_SSE1 HWImpl;
typedef HWImplBlock2x2 HWImpl;
//typedef HWImplBlock4x4 HWImpl;



using Triangle = HWImpl::TriangleType;
