#pragma once

// Disable TinyGLTF internal stb image
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

// We want to manually override loading/writing
#define TINYGLTF_NO_EXTERNAL_IMAGE_LOADER
#define TINYGLTF_NO_EXTERNAL_IMAGE_WRITER

// Enable implementation in ONE translation unit only
#ifdef TINYGLTF_IMPLEMENTATION_FILE
#define TINYGLTF_IMPLEMENTATION
#endif
