
#include "tinygltf_config.h"
#include <tiny_gltf.h>

namespace tinygltf {

    bool LoadImageData(
        Image* image, int image_idx,
        std::string* err, std::string* warn,
        int req_width, int req_height,
        const unsigned char* bytes, int size, void* user_data)
    {
        // We do NOT want TinyGLTF to load images.
        // Just mark image empty.
        image->image.clear();
        image->width = 0;
        image->height = 0;
        image->component = 0;
        return true;
    }

    bool WriteImageData(
        const std::string* filename,
        const std::string* basedir,
        const Image* image,
        bool embedImages,
        const FsCallbacks* fs,
        const URICallbacks* uri,
        std::string* out_uri,
        void* user_data)
    {
        // Not needed — return true or false depending on your needs.
        return false;
    }

} // namespace tinygltf
