/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "webp_encoder.h"
#include "webp/mux.h"
#include "hilog/log.h"
#include "log_tags.h"
#include "media_errors.h"
#include "pixel_convert_adapter.h"
namespace OHOS {
namespace ImagePlugin {
using namespace OHOS::HiviewDFX;
using namespace MultimediaPlugin;
using namespace Media;
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "WebpEncoder" };
constexpr uint32_t WEBP_IMAGE_NUM = 1;
constexpr uint32_t COMPONENT_NUM_3 = 3;
constexpr uint32_t COMPONENT_NUM_4 = 4;
} // namespace

static int StreamWriter(const uint8_t* data, size_t data_size, const WebPPicture* const picture)
{
    HiLog::Debug(LABEL, "StreamWriter data_size=%{public}zu", data_size);

    auto webpEncoder = static_cast<WebpEncoder*>(picture->custom_ptr);
    return webpEncoder->Write(data, data_size) ? 1 : 0;
}

WebpEncoder::WebpEncoder()
{
    HiLog::Debug(LABEL, "create IN");

    HiLog::Debug(LABEL, "create OUT");
}

WebpEncoder::~WebpEncoder()
{
    HiLog::Debug(LABEL, "release IN");

    pixelMaps_.clear();

    HiLog::Debug(LABEL, "release OUT");
}

uint32_t WebpEncoder::StartEncode(OutputDataStream &outputStream, PlEncodeOptions &option)
{
    HiLog::Debug(LABEL, "StartEncode IN, quality=%{public}u, numberHint=%{public}u",
        option.quality, option.numberHint);

    pixelMaps_.clear();

    outputStream_ = &outputStream;
    encodeOpts_ = option;

    HiLog::Debug(LABEL, "StartEncode OUT");
    return SUCCESS;
}

uint32_t WebpEncoder::AddImage(Media::PixelMap &pixelMap)
{
    HiLog::Debug(LABEL, "AddImage IN");

    if (pixelMaps_.size() >= WEBP_IMAGE_NUM) {
        HiLog::Error(LABEL, "AddImage, add pixel map out of range=%{public}u.", WEBP_IMAGE_NUM);
        return ERR_IMAGE_ADD_PIXEL_MAP_FAILED;
    }

    pixelMaps_.push_back(&pixelMap);

    HiLog::Debug(LABEL, "AddImage OUT");
    return SUCCESS;
}

uint32_t WebpEncoder::FinalizeEncode()
{
    HiLog::Debug(LABEL, "FinalizeEncode IN");

    if (pixelMaps_.empty()) {
        HiLog::Error(LABEL, "FinalizeEncode, no pixel map input.");
        return ERR_IMAGE_INVALID_PARAMETER;
    }

    HiLog::Debug(LABEL, "FinalizeEncode, quality=%{public}u, numberHint=%{public}u",
        encodeOpts_.quality, encodeOpts_.numberHint);

    uint32_t errorCode = ERROR;

    Media::PixelMap &pixelMap = *(pixelMaps_[0]);
    WebPConfig webpConfig;
    WebPPicture webpPicture;
    WebPPictureInit(&webpPicture);

    errorCode = SetEncodeConfig(pixelMap, webpConfig, webpPicture);
    HiLog::Debug(LABEL, "FinalizeEncode, config, %{public}u.", errorCode);

    if (errorCode != SUCCESS) {
        HiLog::Error(LABEL, "FinalizeEncode, config failed=%{public}u.", errorCode);
        WebPPictureFree(&webpPicture);
        return errorCode;
    }

    errorCode = DoEncode(pixelMap, webpConfig, webpPicture);
    HiLog::Debug(LABEL, "FinalizeEncode, encode,%{public}u.", errorCode);
    WebPPictureFree(&webpPicture);

    if (errorCode != SUCCESS) {
        HiLog::Error(LABEL, "FinalizeEncode, encode failed=%{public}u.", errorCode);
    }

    HiLog::Debug(LABEL, "FinalizeEncode OUT");
    return errorCode;
}

bool WebpEncoder::Write(const uint8_t* data, size_t data_size)
{
    HiLog::Debug(LABEL, "Write data_size=%{public}zu, iccValid=%{public}d", data_size, iccValid_);

    if (iccValid_) {
        return memoryStream_.write(data, data_size);
    }

    return outputStream_->Write(data, data_size);
}

bool WebpEncoder::CheckEncodeFormat(Media::PixelMap &pixelMap)
{
    PixelFormat pixelFormat = GetPixelFormat(pixelMap);
    HiLog::Debug(LABEL, "CheckEncodeFormat, pixelFormat=%{public}u", pixelFormat);

    switch (pixelFormat) {
        case PixelFormat::RGBA_8888: {
            HiLog::Debug(LABEL, "CheckEncodeFormat, RGBA_8888");
            return true;
        }
        case PixelFormat::BGRA_8888: {
            HiLog::Debug(LABEL, "CheckEncodeFormat, BGRA_8888");
            return true;
        }
        case PixelFormat::RGBA_F16: {
            HiLog::Debug(LABEL, "CheckEncodeFormat, RGBA_F16");
            return true;
        }
        case PixelFormat::ARGB_8888: {
            HiLog::Debug(LABEL, "CheckEncodeFormat, ARGB_8888");
            return true;
        }
        case PixelFormat::RGB_888: {
            HiLog::Debug(LABEL, "CheckEncodeFormat, RGB_888");
            return true;
        }
        case PixelFormat::RGB_565: {
            bool isOpaque = IsOpaque(pixelMap);
            HiLog::Debug(LABEL, "CheckEncodeFormat, RGB_565, isOpaque=%{public}d", isOpaque);
            return isOpaque;
        }
        case PixelFormat::ALPHA_8: {
            HiLog::Debug(LABEL, "CheckEncodeFormat, ALPHA_8");
            return true;
        }
        default: {
            HiLog::Error(LABEL, "CheckEncodeFormat, pixelFormat=%{public}u", pixelFormat);
            return false;
        }
    }
}

bool WebpEncoder::DoTransform(Media::PixelMap &pixelMap, char* dst, int componentsNum)
{
    HiLog::Debug(LABEL, "DoTransform IN");

    PixelFormat pixelFormat = GetPixelFormat(pixelMap);
    AlphaType alphaType = GetAlphaType(pixelMap);
    HiLog::Debug(LABEL, "DoTransform, pixelFormat=%{public}u, alphaType=%{public}d, componentsNum=%{public}d",
        pixelFormat, alphaType, componentsNum);

    if ((pixelFormat == PixelFormat::RGBA_8888) && (alphaType == AlphaType::IMAGE_ALPHA_TYPE_OPAQUE)) {
        HiLog::Debug(LABEL, "DoTransform, RGBA_8888, OPAQUE");
        return DoTransformRGBX(pixelMap, dst, componentsNum);
    } else if ((pixelFormat == PixelFormat::RGBA_8888) && (alphaType == AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL)) {
        HiLog::Debug(LABEL, "DoTransform, RGBA_8888, UNPREMUL");
        return DoTransformMemcpy(pixelMap, dst, componentsNum);
    } else if ((pixelFormat == PixelFormat::RGBA_8888) && (alphaType == AlphaType::IMAGE_ALPHA_TYPE_PREMUL)) {
        HiLog::Debug(LABEL, "DoTransform, RGBA_8888, PREMUL");
        return DoTransformRgbA(pixelMap, dst, componentsNum);
    } else if ((pixelFormat == PixelFormat::BGRA_8888) && (alphaType == AlphaType::IMAGE_ALPHA_TYPE_OPAQUE)) {
        HiLog::Debug(LABEL, "DoTransform, BGRA_8888, OPAQUE");
        return DoTransformBGRX(pixelMap, dst, componentsNum);
    } else if ((pixelFormat == PixelFormat::BGRA_8888) && (alphaType == AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL)) {
        HiLog::Debug(LABEL, "DoTransform, BGRA_8888, UNPREMUL");
        return DoTransformBGRA(pixelMap, dst, componentsNum);
    } else if ((pixelFormat == PixelFormat::BGRA_8888) && (alphaType == AlphaType::IMAGE_ALPHA_TYPE_PREMUL)) {
        HiLog::Debug(LABEL, "DoTransform, BGRA_8888, PREMUL");
        return DoTransformBgrA(pixelMap, dst, componentsNum);
    } else if ((pixelFormat == PixelFormat::RGBA_F16) && (alphaType == AlphaType::IMAGE_ALPHA_TYPE_OPAQUE)) {
        HiLog::Debug(LABEL, "DoTransform, RGBA_F16, OPAQUE");
        return DoTransformF16To8888(pixelMap, dst, componentsNum);
    } else if ((pixelFormat == PixelFormat::RGBA_F16) && (alphaType == AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL)) {
        HiLog::Debug(LABEL, "DoTransform, RGBA_F16, UNPREMUL");
        return DoTransformF16To8888(pixelMap, dst, componentsNum);
    } else if ((pixelFormat == PixelFormat::RGBA_F16) && (alphaType == AlphaType::IMAGE_ALPHA_TYPE_PREMUL)) {
        HiLog::Debug(LABEL, "DoTransform, RGBA_F16, PREMUL");
        return DoTransformF16pTo8888(pixelMap, dst, componentsNum);
    } else if ((pixelFormat == PixelFormat::ARGB_8888) && (alphaType == AlphaType::IMAGE_ALPHA_TYPE_OPAQUE)) {
        return DoTransformArgbToRgb(pixelMap, dst, componentsNum);
    } else if ((pixelFormat == PixelFormat::ARGB_8888) && (alphaType == AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL)) {
        return DoTransformArgbToRgba(pixelMap, dst, componentsNum);
    } else if ((pixelFormat == PixelFormat::ARGB_8888) && (alphaType == AlphaType::IMAGE_ALPHA_TYPE_PREMUL)) {
        return DoTransformArgbToRgba(pixelMap, dst, componentsNum);
    } else if (pixelFormat == PixelFormat::RGB_888) {
        return DoTransformMemcpy(pixelMap, dst, componentsNum);
    } else if ((pixelFormat == PixelFormat::RGB_565) && IsOpaque(pixelMap)) {
        HiLog::Debug(LABEL, "DoTransform, RGB_565, Opaque");
        return DoTransformRGB565(pixelMap, dst, componentsNum);
    } else if (pixelFormat == PixelFormat::ALPHA_8) {
        HiLog::Debug(LABEL, "DoTransform, ALPHA_8");
        return DoTransformGray(pixelMap, dst, componentsNum);
    }

    HiLog::Debug(LABEL, "DoTransform OUT");
    return false;
}

uint32_t WebpEncoder::SetEncodeConfig(Media::PixelMap &pixelMap, WebPConfig &webpConfig, WebPPicture &webpPicture)
{
    HiLog::Debug(LABEL, "SetEncodeConfig IN");

    if (pixelMap.GetPixels() == nullptr) {
        HiLog::Error(LABEL, "SetEncodeConfig, pixels invalid.");
        return ERROR;
    }

    if (!CheckEncodeFormat(pixelMap)) {
        HiLog::Error(LABEL, "SetEncodeConfig, check invalid.");
        return ERR_IMAGE_UNKNOWN_FORMAT;
    }

    if (GetPixelFormat(pixelMap) == PixelFormat::RGBA_F16) {
        componentsNum_ = COMPONENT_NUM_4;
    } else {
        componentsNum_ = IsOpaque(pixelMap) ? COMPONENT_NUM_3 : COMPONENT_NUM_4;
    }
    HiLog::Debug(LABEL, "SetEncodeConfig, componentsNum=%{public}u", componentsNum_);

    if (!WebPConfigPreset(&webpConfig, WEBP_PRESET_DEFAULT, encodeOpts_.quality)) {
        HiLog::Error(LABEL, "SetEncodeConfig, config preset issue.");
        return ERROR;
    }

    GetIcc(pixelMap);

    webpConfig.lossless = 1; // Lossless encoding (0=lossy(default), 1=lossless).
    webpConfig.method = 0; // quality/speed trade-off (0=fast, 6=slower-better)
    webpPicture.use_argb = 1; // Main flag for encoder selecting between ARGB or YUV input.

    webpPicture.width = pixelMap.GetWidth(); // dimensions (less or equal to WEBP_MAX_DIMENSION)
    webpPicture.height = pixelMap.GetHeight(); // dimensions (less or equal to WEBP_MAX_DIMENSION)
    webpPicture.writer = StreamWriter;
    webpPicture.custom_ptr = static_cast<void*>(this);

    auto colorSpace = GetColorSpace(pixelMap);
    HiLog::Debug(LABEL, "SetEncodeConfig, "
        "width=%{public}u, height=%{public}u, colorspace=%{public}d, componentsNum=%{public}d.",
        webpPicture.width, webpPicture.height, colorSpace, componentsNum_);

    HiLog::Debug(LABEL, "SetEncodeConfig OUT");
    return SUCCESS;
}

uint32_t WebpEncoder::DoEncode(Media::PixelMap &pixelMap, WebPConfig &webpConfig, WebPPicture &webpPicture)
{
    HiLog::Debug(LABEL, "DoEncode IN");

    const int width = pixelMap.GetWidth();
    const int height = webpPicture.height;
    const int rgbStride = width * componentsNum_;
    const int rgbSize = rgbStride * height;
    HiLog::Debug(LABEL, "DoEncode, width=%{public}d, height=%{public}d, componentsNum=%{public}d,"
        " rgbStride=%{public}d, rgbSize=%{public}d", width, height, componentsNum_, rgbStride, rgbSize);

    std::unique_ptr<uint8_t[]> rgb = std::make_unique<uint8_t[]>(rgbSize);
    if (!DoTransform(pixelMap, reinterpret_cast<char*>(&rgb[0]), componentsNum_)) {
        HiLog::Error(LABEL, "DoEncode, transform issue.");
        return ERROR;
    }

    auto importProc = WebPPictureImportRGB;
    if (componentsNum_ != COMPONENT_NUM_3) {
        importProc = (IsOpaque(pixelMap)) ? WebPPictureImportRGBX : WebPPictureImportRGBA;
    }

    HiLog::Debug(LABEL, "DoEncode, importProc");
    if (!importProc(&webpPicture, &rgb[0], rgbStride)) {
        HiLog::Error(LABEL, "DoEncode, import issue.");
        return ERROR;
    }

    HiLog::Debug(LABEL, "DoEncode, WebPEncode");
    if (!WebPEncode(&webpConfig, &webpPicture)) {
        HiLog::Error(LABEL, "DoEncode, encode issue.");
        return ERROR;
    }

    HiLog::Debug(LABEL, "DoEncode, iccValid=%{public}d", iccValid_);
    if (iccValid_) {
        auto res = DoEncodeForICC(pixelMap);
        if (res != SUCCESS) {
            HiLog::Error(LABEL, "DoEncode, encode for icc issue.");
            return res;
        }
    }

    HiLog::Debug(LABEL, "DoEncode OUT");
    return SUCCESS;
}

uint32_t WebpEncoder::DoEncodeForICC(Media::PixelMap &pixelMap)
{
    HiLog::Debug(LABEL, "DoEncodeForICC IN");

    auto encodedData = memoryStream_.detachAsData();
    WebPData webpEncode = { encodedData->bytes(), encodedData->size() };
    WebPData webpIcc = { iccBytes_, iccSize_ };

    auto mux = WebPMuxNew();
    if (WebPMuxSetImage(mux, &webpEncode, 0) != WEBP_MUX_OK) {
        HiLog::Error(LABEL, "DoEncodeForICC, image issue.");
        WebPMuxDelete(mux);
        return ERROR;
    }

    if (WebPMuxSetChunk(mux, "ICCP", &webpIcc, 0) != WEBP_MUX_OK) {
        HiLog::Error(LABEL, "DoEncodeForICC, icc issue.");
        WebPMuxDelete(mux);
        return ERROR;
    }

    WebPData webpAssembled;
    if (WebPMuxAssemble(mux, &webpAssembled) != WEBP_MUX_OK) {
        HiLog::Error(LABEL, "DoEncodeForICC, assemble issue.");
        WebPMuxDelete(mux);
        return ERROR;
    }

    outputStream_->Write(webpAssembled.bytes, webpAssembled.size);
    WebPDataClear(&webpAssembled);
    WebPMuxDelete(mux);

    HiLog::Debug(LABEL, "DoEncodeForICC OUT");
    return SUCCESS;
}

ColorSpace WebpEncoder::GetColorSpace(Media::PixelMap &pixelMap)
{
    return pixelMap.GetColorSpace();
}

PixelFormat WebpEncoder::GetPixelFormat(Media::PixelMap &pixelMap)
{
    return pixelMap.GetPixelFormat();
}

AlphaType WebpEncoder::GetAlphaType(Media::PixelMap &pixelMap)
{
    return pixelMap.GetAlphaType();
}

bool WebpEncoder::GetIcc(Media::PixelMap &pixelMap)
{
    return iccValid_;
}

bool WebpEncoder::IsOpaque(Media::PixelMap &pixelMap)
{
    return (GetAlphaType(pixelMap) == AlphaType::IMAGE_ALPHA_TYPE_OPAQUE);
}

bool WebpEncoder::DoTransformMemcpy(Media::PixelMap &pixelMap, char* dst, int componentsNum)
{
    HiLog::Debug(LABEL, "DoTransformMemcpy IN");

    auto src = pixelMap.GetPixels();
    if ((src == nullptr) || (dst == nullptr)) {
        HiLog::Error(LABEL, "DoTransformMemcpy, address issue.");
        return false;
    }

    const int32_t width = pixelMap.GetWidth();
    const int32_t height = pixelMap.GetHeight();
    const uint32_t rowBytes = pixelMap.GetRowBytes();
    const int stride = pixelMap.GetWidth() * componentsNum;

    HiLog::Debug(LABEL,
        "width=%{public}u, height=%{public}u, rowBytes=%{public}u, stride=%{public}d, componentsNum=%{public}d",
        width, height, rowBytes, stride, componentsNum);

    for (int32_t h = 0; h < height; h++) {
        transform_scanline_memcpy(reinterpret_cast<char*>(&dst[h * stride]),
            reinterpret_cast<const char*>(&src[h * rowBytes]),
            width, componentsNum);
    }

    HiLog::Debug(LABEL, "DoTransformMemcpy OUT");
    return true;
}

bool WebpEncoder::DoTransformRGBX(Media::PixelMap &pixelMap, char* dst, int componentsNum)
{
    HiLog::Debug(LABEL, "DoTransformRGBX IN");

    const void *srcPixels = pixelMap.GetPixels();
    uint32_t srcRowBytes = pixelMap.GetRowBytes();
    const ImageInfo srcInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGBA_8888, AlphaType::IMAGE_ALPHA_TYPE_OPAQUE);

    void *dstPixels = dst;
    uint32_t dstRowBytes = pixelMap.GetWidth() * componentsNum;
    const ImageInfo dstInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGB_888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);

    ShowTransformParam(srcInfo, srcRowBytes, dstInfo, dstRowBytes, componentsNum);

    if ((srcPixels == nullptr) || (dstPixels == nullptr)) {
        HiLog::Error(LABEL, "DoTransformRGBX, address issue.");
        return false;
    }

    const Position dstPos;
    if (!PixelConvertAdapter::WritePixelsConvert(srcPixels, srcRowBytes, srcInfo,
        dstPixels, dstPos, dstRowBytes, dstInfo)) {
        HiLog::Error(LABEL, "DoTransformRGBX, pixel convert in adapter failed.");
        return false;
    }

    HiLog::Debug(LABEL, "DoTransformRGBX OUT");
    return true;
}

bool WebpEncoder::DoTransformRgbA(Media::PixelMap &pixelMap, char* dst, int componentsNum)
{
    HiLog::Debug(LABEL, "DoTransformRgbA IN");

    const void *srcPixels = pixelMap.GetPixels();
    uint32_t srcRowBytes = pixelMap.GetRowBytes();
    const ImageInfo srcInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGBA_8888, AlphaType::IMAGE_ALPHA_TYPE_PREMUL);

    void *dstPixels = dst;
    uint32_t dstRowBytes = pixelMap.GetWidth() * componentsNum;
    const ImageInfo dstInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGBA_8888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);

    ShowTransformParam(srcInfo, srcRowBytes, dstInfo, dstRowBytes, componentsNum);

    if ((srcPixels == nullptr) || (dstPixels == nullptr)) {
        HiLog::Error(LABEL, "DoTransformRgbA, address issue.");
        return false;
    }

    const Position dstPos;
    if (!PixelConvertAdapter::WritePixelsConvert(srcPixels, srcRowBytes, srcInfo,
        dstPixels, dstPos, dstRowBytes, dstInfo)) {
        HiLog::Error(LABEL, "DoTransformRgbA, pixel convert in adapter failed.");
        return false;
    }

    HiLog::Debug(LABEL, "DoTransformRgbA OUT");
    return true;
}

bool WebpEncoder::DoTransformBGRX(Media::PixelMap &pixelMap, char* dst, int componentsNum)
{
    HiLog::Debug(LABEL, "DoTransformBGRX IN");

    const void *srcPixels = pixelMap.GetPixels();
    uint32_t srcRowBytes = pixelMap.GetRowBytes();
    const ImageInfo srcInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::BGRA_8888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);

    void *dstPixels = dst;
    uint32_t dstRowBytes = pixelMap.GetWidth() * componentsNum;
    const ImageInfo dstInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGB_888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);

    ShowTransformParam(srcInfo, srcRowBytes, dstInfo, dstRowBytes, componentsNum);

    if ((srcPixels == nullptr) || (dstPixels == nullptr)) {
        HiLog::Error(LABEL, "DoTransformBGRX, address issue.");
        return false;
    }

    const Position dstPos;
    if (!PixelConvertAdapter::WritePixelsConvert(srcPixels, srcRowBytes, srcInfo,
        dstPixels, dstPos, dstRowBytes, dstInfo)) {
        HiLog::Error(LABEL, "DoTransformBGRX, pixel convert in adapter failed.");
        return false;
    }

    HiLog::Debug(LABEL, "DoTransformBGRX OUT");
    return true;
}

bool WebpEncoder::DoTransformBGRA(Media::PixelMap &pixelMap, char* dst, int componentsNum)
{
    HiLog::Debug(LABEL, "DoTransformBGRA IN");

    const void *srcPixels = pixelMap.GetPixels();
    uint32_t srcRowBytes = pixelMap.GetRowBytes();
    const ImageInfo srcInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::BGRA_8888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);

    void *dstPixels = dst;
    uint32_t dstRowBytes = pixelMap.GetWidth() * componentsNum;
    const ImageInfo dstInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGBA_8888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);

    ShowTransformParam(srcInfo, srcRowBytes, dstInfo, dstRowBytes, componentsNum);

    if ((srcPixels == nullptr) || (dstPixels == nullptr)) {
        HiLog::Error(LABEL, "DoTransformBGRA, address issue.");
        return false;
    }

    const Position dstPos;
    if (!PixelConvertAdapter::WritePixelsConvert(srcPixels, srcRowBytes, srcInfo,
        dstPixels, dstPos, dstRowBytes, dstInfo)) {
        HiLog::Error(LABEL, "DoTransformBGRA, pixel convert in adapter failed.");
        return false;
    }

    HiLog::Debug(LABEL, "DoTransformBGRA OUT");
    return true;
}

bool WebpEncoder::DoTransformBgrA(Media::PixelMap &pixelMap, char* dst, int componentsNum)
{
    HiLog::Debug(LABEL, "DoTransformBgrA IN");

    const void *srcPixels = pixelMap.GetPixels();
    uint32_t srcRowBytes = pixelMap.GetRowBytes();
    const ImageInfo srcInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::BGRA_8888, AlphaType::IMAGE_ALPHA_TYPE_PREMUL);

    void *dstPixels = dst;
    uint32_t dstRowBytes = pixelMap.GetWidth() * componentsNum;
    const ImageInfo dstInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGBA_8888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);

    ShowTransformParam(srcInfo, srcRowBytes, dstInfo, dstRowBytes, componentsNum);

    if ((srcPixels == nullptr) || (dstPixels == nullptr)) {
        HiLog::Error(LABEL, "DoTransformBgrA, address issue.");
        return false;
    }

    const Position dstPos;
    if (!PixelConvertAdapter::WritePixelsConvert(srcPixels, srcRowBytes, srcInfo,
        dstPixels, dstPos, dstRowBytes, dstInfo)) {
        HiLog::Error(LABEL, "DoTransformBgrA, pixel convert in adapter failed.");
        return false;
    }

    HiLog::Debug(LABEL, "DoTransformBgrA OUT");
    return true;
}

bool WebpEncoder::DoTransformF16To8888(Media::PixelMap &pixelMap, char* dst, int componentsNum)
{
    HiLog::Debug(LABEL, "DoTransformF16To8888 IN");

    const void *srcPixels = pixelMap.GetPixels();
    uint32_t srcRowBytes = pixelMap.GetRowBytes();
    const ImageInfo srcInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGBA_F16, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);

    void *dstPixels = dst;
    uint32_t dstRowBytes = pixelMap.GetWidth() * componentsNum;
    const ImageInfo dstInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGBA_8888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);

    ShowTransformParam(srcInfo, srcRowBytes, dstInfo, dstRowBytes, componentsNum);

    if ((srcPixels == nullptr) || (dstPixels == nullptr)) {
        HiLog::Error(LABEL, "DoTransformF16To8888, address issue.");
        return false;
    }

    const Position dstPos;
    if (!PixelConvertAdapter::WritePixelsConvert(srcPixels, srcRowBytes, srcInfo,
        dstPixels, dstPos, dstRowBytes, dstInfo)) {
        HiLog::Error(LABEL, "DoTransformF16To8888, pixel convert in adapter failed.");
        return false;
    }

    HiLog::Debug(LABEL, "DoTransformF16To8888 OUT");
    return true;
}

bool WebpEncoder::DoTransformF16pTo8888(Media::PixelMap &pixelMap, char* dst, int componentsNum)
{
    HiLog::Debug(LABEL, "DoTransformF16pTo8888 IN");

    const void *srcPixels = pixelMap.GetPixels();
    uint32_t srcRowBytes = pixelMap.GetRowBytes();
    const ImageInfo srcInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGBA_F16, AlphaType::IMAGE_ALPHA_TYPE_PREMUL);

    void *dstPixels = dst;
    uint32_t dstRowBytes = pixelMap.GetWidth() * componentsNum;
    const ImageInfo dstInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGBA_8888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);

    ShowTransformParam(srcInfo, srcRowBytes, dstInfo, dstRowBytes, componentsNum);

    if ((srcPixels == nullptr) || (dstPixels == nullptr)) {
        HiLog::Error(LABEL, "DoTransformF16pTo8888, address issue.");
        return false;
    }

    const Position dstPos;
    if (!PixelConvertAdapter::WritePixelsConvert(srcPixels, srcRowBytes, srcInfo,
        dstPixels, dstPos, dstRowBytes, dstInfo)) {
        HiLog::Error(LABEL, "DoTransformF16pTo8888, pixel convert in adapter failed.");
        return false;
    }

    HiLog::Debug(LABEL, "DoTransformF16pTo8888 OUT");
    return true;
}

bool WebpEncoder::DoTransformArgbToRgb(Media::PixelMap &pixelMap, char* dst, int componentsNum)
{
    HiLog::Debug(LABEL, "DoTransformArgbToRgb IN");

    const void *srcPixels = pixelMap.GetPixels();
    uint32_t srcRowBytes = pixelMap.GetRowBytes();
    const ImageInfo srcInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::ARGB_8888, AlphaType::IMAGE_ALPHA_TYPE_OPAQUE);

    void *dstPixels = dst;
    uint32_t dstRowBytes = pixelMap.GetWidth() * componentsNum;
    const ImageInfo dstInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGB_888, AlphaType::IMAGE_ALPHA_TYPE_OPAQUE);

    ShowTransformParam(srcInfo, srcRowBytes, dstInfo, dstRowBytes, componentsNum);

    if ((srcPixels == nullptr) || (dstPixels == nullptr)) {
        HiLog::Error(LABEL, "DoTransformArgbToRgb, address issue.");
        return false;
    }

    const Position dstPos;
    if (!PixelConvertAdapter::WritePixelsConvert(srcPixels, srcRowBytes, srcInfo,
        dstPixels, dstPos, dstRowBytes, dstInfo)) {
        HiLog::Error(LABEL, "DoTransformArgbToRgb, pixel convert in adapter failed.");
        return false;
    }

    HiLog::Debug(LABEL, "DoTransformArgbToRgb OUT");
    return true;
}

bool WebpEncoder::DoTransformArgbToRgba(Media::PixelMap &pixelMap, char* dst, int componentsNum)
{
    HiLog::Debug(LABEL, "DoTransformArgbToRgba IN");

    const void *srcPixels = pixelMap.GetPixels();
    uint32_t srcRowBytes = pixelMap.GetRowBytes();
    const ImageInfo srcInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::ARGB_8888, pixelMap.GetAlphaType());

    void *dstPixels = dst;
    uint32_t dstRowBytes = pixelMap.GetWidth() * componentsNum;
    const ImageInfo dstInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGBA_8888, pixelMap.GetAlphaType());

    ShowTransformParam(srcInfo, srcRowBytes, dstInfo, dstRowBytes, componentsNum);

    if ((srcPixels == nullptr) || (dstPixels == nullptr)) {
        HiLog::Error(LABEL, "DoTransformArgbToRgba, address issue.");
        return false;
    }

    const Position dstPos;
    if (!PixelConvertAdapter::WritePixelsConvert(srcPixels, srcRowBytes, srcInfo,
        dstPixels, dstPos, dstRowBytes, dstInfo)) {
        HiLog::Error(LABEL, "DoTransformArgbToRgba, pixel convert in adapter failed.");
        return false;
    }

    HiLog::Debug(LABEL, "DoTransformArgbToRgba OUT");
    return true;
}

bool WebpEncoder::DoTransformRGB565(Media::PixelMap &pixelMap, char* dst, int componentsNum)
{
    HiLog::Debug(LABEL, "DoTransformRGB565 IN");

    const void *srcPixels = pixelMap.GetPixels();
    uint32_t srcRowBytes = pixelMap.GetRowBytes();
    const ImageInfo srcInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGB_565, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);

    void *dstPixels = dst;
    uint32_t dstRowBytes = pixelMap.GetWidth() * componentsNum;
    const ImageInfo dstInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGB_888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);

    ShowTransformParam(srcInfo, srcRowBytes, dstInfo, dstRowBytes, componentsNum);

    if ((srcPixels == nullptr) || (dstPixels == nullptr)) {
        HiLog::Error(LABEL, "DoTransformRGB565, address issue.");
        return false;
    }

    const Position dstPos;
    if (!PixelConvertAdapter::WritePixelsConvert(srcPixels, srcRowBytes, srcInfo,
        dstPixels, dstPos, dstRowBytes, dstInfo)) {
        HiLog::Error(LABEL, "DoTransformRGB565, pixel convert in adapter failed.");
        return false;
    }

    HiLog::Debug(LABEL, "DoTransformRGB565 OUT");
    return true;
}

bool WebpEncoder::DoTransformGray(Media::PixelMap &pixelMap, char* dst, int componentsNum)
{
    HiLog::Debug(LABEL, "DoTransformGray IN");

    const void *srcPixels = pixelMap.GetPixels();
    uint32_t srcRowBytes = pixelMap.GetRowBytes();
    const ImageInfo srcInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::ALPHA_8, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);

    void *dstPixels = dst;
    uint32_t dstRowBytes = pixelMap.GetWidth() * componentsNum;
    const ImageInfo dstInfo = MakeImageInfo(pixelMap.GetWidth(), pixelMap.GetHeight(),
        PixelFormat::RGBA_8888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);

    ShowTransformParam(srcInfo, srcRowBytes, dstInfo, dstRowBytes, componentsNum);

    if ((srcPixels == nullptr) || (dstPixels == nullptr)) {
        HiLog::Error(LABEL, "DoTransformGray, address issue.");
        return false;
    }

    const Position dstPos;
    if (!PixelConvertAdapter::WritePixelsConvert(srcPixels, srcRowBytes, srcInfo,
        dstPixels, dstPos, dstRowBytes, dstInfo)) {
        HiLog::Error(LABEL, "DoTransformGray, pixel convert in adapter failed.");
        return false;
    }

    HiLog::Debug(LABEL, "DoTransformGray OUT");
    return true;
}

ImageInfo WebpEncoder::MakeImageInfo(int width, int height, PixelFormat pf, AlphaType at, ColorSpace cs)
{
    ImageInfo info = {
        .size = {
            .width = width,
            .height = height
        },
        .pixelFormat = pf,
        .colorSpace = cs,
        .alphaType = at
    };

    return info;
}

void WebpEncoder::ShowTransformParam(const ImageInfo &srcInfo, const uint32_t &srcRowBytes,
    const ImageInfo &dstInfo, const uint32_t &dstRowBytes, const int &componentsNum)
{
    HiLog::Debug(LABEL,
        "src(width=%{public}u, height=%{public}u, rowBytes=%{public}u,"
        " pixelFormat=%{public}u, colorspace=%{public}d, alphaType=%{public}d, baseDensity=%{public}d), "
        "dst(width=%{public}u, height=%{public}u, rowBytes=%{public}u,"
        " pixelFormat=%{public}u, colorspace=%{public}d, alphaType=%{public}d, baseDensity=%{public}d), "
        "componentsNum=%{public}d",
        srcInfo.size.width, srcInfo.size.height, srcRowBytes,
        srcInfo.pixelFormat, srcInfo.colorSpace, srcInfo.alphaType, srcInfo.baseDensity,
        dstInfo.size.width, dstInfo.size.height, dstRowBytes,
        dstInfo.pixelFormat, dstInfo.colorSpace, dstInfo.alphaType, dstInfo.baseDensity,
        componentsNum);
}
} // namespace ImagePlugin
} // namespace OHOS
