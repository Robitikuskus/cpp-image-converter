#include <img_lib.h>
#include <jpeg_image.h>
#include <ppm_image.h>
#include <bmp_image.h>

#include <filesystem>
#include <string>
#include <iostream>
#include <algorithm>

using namespace std;

namespace FormatInterfaces {

class ImageFormatInterface {
public:
    virtual ~ImageFormatInterface() = default;
    virtual img_lib::Image Load(const img_lib::Path& path) const = 0;
    virtual bool Save(const img_lib::Path& path, const img_lib::Image& image) const = 0;
};

class PPM : public ImageFormatInterface {
public:
    img_lib::Image Load(const img_lib::Path& path) const override {
        return img_lib::LoadPPM(path);
    }
    bool Save(const img_lib::Path& path, const img_lib::Image& image) const override {
        return img_lib::SavePPM(path, image);
    }
};

class JPEG : public ImageFormatInterface {
public:
    img_lib::Image Load(const img_lib::Path& path) const override {
        return img_lib::LoadJPEG(path);
    }
    bool Save(const img_lib::Path& path, const img_lib::Image& image) const override {
        return img_lib::SaveJPEG(path, image);
    }
};

class BMP : public ImageFormatInterface {
public:
    img_lib::Image Load(const img_lib::Path& path) const override {
        return img_lib::LoadBMP(path);
    }
    bool Save(const img_lib::Path& path, const img_lib::Image& image) const override {
        return img_lib::SaveBMP(path, image);
    }
};

}  // namespace FormatInterfaces

string GetFormatByExtension(const img_lib::Path& path) {
    string ext = path.extension().string();
    if (!ext.empty() && ext.front() == '.') {
        ext.erase(0, 1);
    }
    transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

const FormatInterfaces::ImageFormatInterface* GetFormatInterface(const img_lib::Path& path) {
    static const FormatInterfaces::PPM ppmInterface;
    static const FormatInterfaces::JPEG jpegInterface;
    static const FormatInterfaces::BMP bmpInterface;

    string format = GetFormatByExtension(path);
    
    if (format == "ppm") return &ppmInterface;
    if (format == "jpeg" || format == "jpg") return &jpegInterface;
    if (format == "bmp") return &bmpInterface;
    
    return nullptr;
}

int main(int argc, const char** argv) {
    if (argc != 3) {
        cerr << "Usage: "sv << argv[0] << " <in_file> <out_file>"sv << endl;
        return 1;
    }

    img_lib::Path in_path = argv[1];
    img_lib::Path out_path = argv[2];

    if (!filesystem::exists(in_path)) {
        cerr << "Input file does not exist: "sv << in_path << endl;
        return 6;
    }

    const FormatInterfaces::ImageFormatInterface* in_format = GetFormatInterface(in_path);
    if (!in_format) {
        cerr << "Unknown format of the input file"sv << endl;
        return 2;
    }

    const FormatInterfaces::ImageFormatInterface* out_format = GetFormatInterface(out_path);
    if (!out_format) {
        cerr << "Unknown format of the output file"sv << endl;
        return 3;
    }

    img_lib::Image image = in_format->Load(in_path);
    if (image.GetWidth() == 0 || image.GetHeight() == 0) {
        cerr << "Loading failed: image is empty or corrupted"sv << endl;
        return 4;
    }

    if (!out_format->Save(out_path, image)) {
        cerr << "Saving failed"sv << endl;
        return 5;
    }

    cout << "Successfully converted"sv << endl;
    return 0;
}
