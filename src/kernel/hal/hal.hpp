#pragma once
#include <boot/bootparams.h>
#include <core/arch/i686/VGATextDevice.hpp>

// Hardware Abstraction Layer Initialization function
void HAL_Initialize(BootParams* bootparams, arch::i686::VGATextDevice* vga);