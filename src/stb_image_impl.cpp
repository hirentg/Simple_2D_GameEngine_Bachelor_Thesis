// Dedicated compilation unit for stb_image implementation.
// This ensures STB_IMAGE_IMPLEMENTATION is defined exactly once across the
// entire project, preventing duplicate symbol linker errors.

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
