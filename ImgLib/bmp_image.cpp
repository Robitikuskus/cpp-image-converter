#include "bmp_image.h"
#include "pack_defines.h"

#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    uint16_t bfType = 0x4D42;
    uint32_t bfSize;
    uint16_t bfReserved1 = 0;
    uint16_t bfReserved2 = 0;
    uint32_t bfOffBits = 54;
}
PACKED_STRUCT_END;

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t biSize = 40;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes = 1;
    uint16_t biBitCount = 24;
    uint32_t biCompression = 0;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter = 11811;
    int32_t biYPelsPerMeter = 11811;
    uint32_t biClrUsed = 0;
    uint32_t biClrImportant = 0x1000000;
}
PACKED_STRUCT_END;

static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

bool SaveBMP(const Path& file, const Image& image) {
    int width = image.GetWidth();
    int height = image.GetHeight();
    int stride = GetBMPStride(width);

    BitmapFileHeader fileHeader;
    BitmapInfoHeader infoHeader;

    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biSizeImage = stride * height;

    fileHeader.bfSize = fileHeader.bfOffBits + infoHeader.biSizeImage;

    ofstream out(file, ios::binary);
    if (!out) {
        return false;
    }

    out.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    out.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

    vector<char> padding(stride - width * 3, 0);

    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            auto pixel = image.GetPixel(x, y);
            char bgr[3] = {static_cast<char>(pixel.b), static_cast<char>(pixel.g), static_cast<char>(pixel.r)};
            out.write(bgr, 3);
        }
        out.write(padding.data(), padding.size());
    }

    return out.good();
}

Image LoadBMP(const Path& file) {
    ifstream in(file, ios::binary);
    if (!in) {
        return {};
    }

    BitmapFileHeader fileHeader;
    BitmapInfoHeader infoHeader;

    in.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    in.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

    if (fileHeader.bfType != 0x4D42 || infoHeader.biBitCount != 24 || infoHeader.biCompression != 0) {
        return {};
    }

    int width = infoHeader.biWidth;
    int height = abs(infoHeader.biHeight);
    int stride = GetBMPStride(width);

    Image image(width, height, Color::Black());
    vector<char> padding(stride - width * 3);

    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            char bgr[3];
            in.read(bgr, 3);
            image.GetPixel(x, y) = {
                std::byte(static_cast<uint8_t>(bgr[2])),
                std::byte(static_cast<uint8_t>(bgr[1])),
                std::byte(static_cast<uint8_t>(bgr[0])),
                std::byte{255}
            };
        }
        in.read(padding.data(), padding.size());
    }

    if (image.GetWidth() != width || image.GetHeight() != height) {
        return {};
    }

    return in.good() ? image : Image();
}

} // namespace img_lib
