#include "config.h"
#include "lcarsng.h"
#include "imageloader.h"
#include <math.h>
#include <string>
#include <dirent.h>
#include <iostream>

using namespace Magick;

cImageLoader::cImageLoader() : cImageMagickWrapper() {
}

cImageLoader::~cImageLoader() {
}

cImage cImageLoader::GetImage() {
    return CreateImageCopy();
}

bool cImageLoader::LoadPoster(const char *poster, int width, int height, bool scale) {
    if ((width == 0) || (height==0))
        return false;
    if (LoadImage(poster)) {
        if (scale)
            buffer.sample(Geometry(width, height));
        return true;
    }
    return false;
}
