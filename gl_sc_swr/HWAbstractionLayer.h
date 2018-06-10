#pragma once

#include <cstdint>
#include "LiteMath.h"

#include "TriRaster.h"        

#include "PureCpp.h"
#include "SSE1Impl.h"


//@select current implementation here via typedef assigment :)
//typedef HWImplementationPureCpp HWImpl;
typedef HWImpl_SSE1 HWImpl;


using Triangle = HWImpl::TriangleType;