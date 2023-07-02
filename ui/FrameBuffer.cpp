#include "FrameBuffer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "3rd/stb_image_write.h"

void FrameBuffer::saveToFile(const std::string& fileName)
{
    stbi_write_png(fileName.c_str(), width, height, channals, ubuffer, width * channals);
}
