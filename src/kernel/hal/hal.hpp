#pragma once
#include <boot/bootparams.h>
#include <core/arch/i686/PagingManager.hpp>

// Hardware Abstraction Layer Initialization function
PagingManager HAL_Initialize(BootParams* bootparams);