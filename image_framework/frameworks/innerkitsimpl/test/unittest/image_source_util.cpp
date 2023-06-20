/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include <fstream>
#include <string>
#include "directory_ex.h"
#include "hilog/log.h"
#include "image_packer.h"
#include "image_type.h"
#include "image_utils.h"
#include "log_tags.h"
#include "media_errors.h"
#include "pixel_map.h"
#include "image_source_util.h"

using namespace OHOS::Media;
using namespace OHOS::HiviewDFX;
using namespace OHOS::ImageSourceUtil;

static constexpr OHOS::HiviewDFX::HiLogLabel LABEL_TEST = { LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "ImageSourceUtil" };
namespace OHOS {
namespace ImageSourceUtil {
constexpr uint32_t NUM_1 = 1;
constexpr uint32_t NUM_100 = 100;
constexpr int64_t BUFFER_SIZE = 2 * 1024 * 1024;

int64_t PackImage(const std::string &filePath, std::unique_ptr<PixelMap> pixelMap)
{
    ImagePacker imagePacker;
    PackOption option;
    option.format = "image/jpeg";
    option.quality = NUM_100;
    option.numberHint = NUM_1;
    std::set<std::string> formats;
    if (pixelMap == nullptr) {
        HiLog::Error(LABEL_TEST, "pixelMap is nullptr");
        return 0;
    }
    uint32_t ret = imagePacker.GetSupportedFormats(formats);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL_TEST, "image packer get supported format failed, ret=%{public}u.", ret);
        return 0;
    }
    imagePacker.StartPacking(filePath, option);
    imagePacker.AddImage(*pixelMap);
    int64_t packedSize = 0;
    imagePacker.FinalizePacking(packedSize);
    return static_cast<int64_t>(packedSize);
}

int64_t PackImage(std::unique_ptr<ImageSource> imageSource)
{
    ImagePacker imagePacker;
    PackOption option;
    option.format = "image/jpeg";
    option.quality = NUM_100;
    option.numberHint = 1;
    std::set<std::string> formats;
    if (imageSource == nullptr) {
        HiLog::Error(LABEL_TEST, "imageSource is nullptr");
        return 0;
    }
    uint32_t ret = imagePacker.GetSupportedFormats(formats);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL_TEST, "image packer get supported format failed, ret=%{public}u.", ret);
        return 0;
    }
    int64_t bufferSize = BUFFER_SIZE;
    uint8_t *resultBuffer = reinterpret_cast<uint8_t *>(malloc(bufferSize));
    if (resultBuffer == nullptr) {
        HiLog::Error(LABEL_TEST, "image packer malloc buffer failed.");
        return 0;
    }
    imagePacker.StartPacking(resultBuffer, bufferSize, option);
    imagePacker.AddImage(*imageSource);
    int64_t packedSize = 0;
    imagePacker.FinalizePacking(packedSize);
    return static_cast<int64_t>(packedSize);
}

bool ReadFileToBuffer(const std::string &filePath, uint8_t *buffer, size_t bufferSize)
{
    std::string realPath;
    if (!OHOS::PathToRealPath(filePath, realPath)) {
        HiLog::Error(LABEL_TEST, "file path to real path failed, file path=%{public}s.", filePath.c_str());
        return false;
    }

    if (buffer == nullptr) {
        HiLog::Error(LABEL_TEST, "buffer is nullptr");
        return false;
    }

    FILE *fp = fopen(realPath.c_str(), "rb");
    if (fp == nullptr) {
        HiLog::Error(LABEL_TEST, "open file failed, real path=%{public}s.", realPath.c_str());
        return false;
    }
    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (bufferSize < fileSize) {
        HiLog::Error(LABEL_TEST, "buffer size:(%{public}zu) is smaller than file size:(%{public}zu).", bufferSize,
                     fileSize);
        fclose(fp);
        return false;
    }
    size_t retSize = fread(buffer, 1, fileSize, fp);
    if (retSize != fileSize) {
        HiLog::Error(LABEL_TEST, "read file result size = %{public}zu, size = %{public}zu.", retSize, fileSize);
        fclose(fp);
        return false;
    }
    int ret = fclose(fp);
    if (ret != 0) {
        return true;
    }
    return true;
}
} // namespace ImageSourceUtil
} // namespace OHOS
