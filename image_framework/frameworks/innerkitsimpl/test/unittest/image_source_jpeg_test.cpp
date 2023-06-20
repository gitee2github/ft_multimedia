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

#include <gtest/gtest.h>
#include <fstream>
#include <fcntl.h>
#include "directory_ex.h"
#include "hilog/log.h"
#include "image_packer.h"
#include "image_source.h"
#include "image_type.h"
#include "image_utils.h"
#include "incremental_pixel_map.h"
#include "log_tags.h"
#include "media_errors.h"
#include "pixel_map.h"
#include "image_receiver.h"
#include "image_source_util.h"
#include "graphic_common.h"
#include "image_receiver_manager.h"

using namespace testing::ext;
using namespace OHOS::Media;
using namespace OHOS::HiviewDFX;
using namespace OHOS::ImageSourceUtil;

namespace OHOS {
namespace Multimedia {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL_TEST = {
    LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "ImageSourceJpegTest"
};
static constexpr uint32_t DEFAULT_DELAY_UTIME = 10000;  // 10 ms.
static const std::string IMAGE_INPUT_JPEG_PATH = "/data/local/tmp/image/test.jpg";
static const std::string IMAGE_INPUT_HW_JPEG_PATH = "/data/local/tmp/image/test_hw.jpg";
static const std::string IMAGE_INPUT_EXIF_JPEG_PATH = "/data/local/tmp/image/test_exif.jpg";
static const std::string IMAGE_OUTPUT_JPEG_FILE_PATH = "/data/test/test_file.jpg";
static const std::string IMAGE_OUTPUT_JPEG_BUFFER_PATH = "/data/test/test_buffer.jpg";
static const std::string IMAGE_OUTPUT_JPEG_ISTREAM_PATH = "/data/test/test_istream.jpg";
static const std::string IMAGE_OUTPUT_JPEG_INC_PATH = "/data/test/test_inc.jpg";
static const std::string IMAGE_OUTPUT_HW_JPEG_FILE_PATH = "/data/test/test_hw_file.jpg";
static const std::string IMAGE_OUTPUT_JPEG_MULTI_FILE1_PATH = "/data/test/test_file1.jpg";
static const std::string IMAGE_OUTPUT_JPEG_MULTI_FILE2_PATH = "/data/test/test_file2.jpg";
static const std::string IMAGE_OUTPUT_JPEG_MULTI_INC1_PATH = "/data/test/test_inc1.jpg";
static const std::string IMAGE_OUTPUT_JPEG_MULTI_ONETIME1_PATH = "/data/test/test_onetime1.jpg";
static const std::string IMAGE_OUTPUT_JPEG_MULTI_INC2_PATH = "/data/test/test_inc2.jpg";
static const std::string IMAGE_OUTPUT_JPEG_MULTI_ONETIME2_PATH = "/data/test/test_onetime2.jpg";

const std::string ORIENTATION = "Orientation";
const std::string IMAGE_HEIGHT = "ImageHeight";
const std::string IMAGE_WIDTH = "ImageWidth";
const std::string GPS_LATITUDE = "GPSLatitude";
const std::string GPS_LONGITUDE = "GPSLongitude";
const std::string GPS_LATITUDE_REF = "GPSLatitudeRef";
const std::string GPS_LONGITUDE_REF = "GPSLongitudeRef";

class ImageSourceJpegTest : public testing::Test {
public:
    ImageSourceJpegTest() {}
    ~ImageSourceJpegTest() {}
};

/**
 * @tc.name: TC028
 * @tc.desc: Create ImageSource(stream)
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, TC028, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct jpeg stream and jpeg format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    opts.formatHint = "image/jpeg";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(std::move(fs), opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
}

/**
 * @tc.name: TC029
 * @tc.desc: Create ImageSource(path)
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, TC029, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct jpeg file path and jpeg format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/jpeg";
    std::unique_ptr<ImageSource> imageSource =
        ImageSource::CreateImageSource(IMAGE_INPUT_HW_JPEG_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
}

/**
 * @tc.name: TC030
 * @tc.desc: Create ImageSource(data)
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, TC030, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct jpeg data and jpeg format hit.
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool ret = ImageUtils::GetFileSize(IMAGE_INPUT_JPEG_PATH, bufferSize);
    ASSERT_EQ(ret, true);
    uint8_t *buffer = static_cast<uint8_t *>(malloc(bufferSize));
    ASSERT_NE(buffer, nullptr);
    ret = OHOS::ImageSourceUtil::ReadFileToBuffer(IMAGE_INPUT_JPEG_PATH, buffer, bufferSize);
    ASSERT_EQ(ret, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(buffer, bufferSize, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
}

/**
 * @tc.name: TC032
 * @tc.desc: Test GetImageInfo
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, TC032, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct jpeg data and jpeg format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_JPEG_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);

    ImageInfo imageInfo;
    uint32_t index = 0;
    uint32_t ret = imageSource->GetImageInfo(index, imageInfo);
    ASSERT_EQ(ret, SUCCESS);
    ret = imageSource->GetImageInfo(imageInfo);
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: TC033
 * @tc.desc: Test GetImagePropertyInt(index, key, value)
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, TC033, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct jpeg data and jpeg format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_JPEG_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);

    uint32_t index = 0;
    int32_t value = 0;
    std::string key;
    imageSource->GetImagePropertyInt(index, key, value);
}

/**
 * @tc.name: TC035
 * @tc.desc: Test CreatePixelMap
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, TC035, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct jpeg data and jpeg format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_JPEG_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);

    uint32_t index = 0;
    DecodeOptions optsPixel;
    errorCode = 0;

    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(index, optsPixel, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    ASSERT_EQ(pixelMap->GetAlphaType(), AlphaType::IMAGE_ALPHA_TYPE_OPAQUE);
}

/**
 * @tc.name: TC036
 * @tc.desc: Test Area decoding,configure area
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, TC036, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create jpg image source by istream source stream and default format hit
     * @tc.expected: step1. create jpg image source success.
     */
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    fs->open("/data/local/tmp/image/test.jpg", std::fstream::binary | std::fstream::in);
    bool isOpen = fs->is_open();
    ASSERT_EQ(isOpen, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(std::move(fs), opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. crop jpg image source to pixel map crop options
     * @tc.expected: step2. crop jpg image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    decodeOpts.CropRect.top = 3;
    decodeOpts.CropRect.width = 100;
    decodeOpts.CropRect.left = 3;
    decodeOpts.CropRect.height = 200;
    decodeOpts.desiredSize.width = 200;
    decodeOpts.desiredSize.height = 400;
    decodeOpts.rotateDegrees = 90;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    HiLog::Debug(LABEL_TEST, "create pixel map error code=%{public}u.", errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    EXPECT_EQ(200, pixelMap->GetWidth());
    EXPECT_EQ(400, pixelMap->GetHeight());
}

/**
 * @tc.name: TC037
 * @tc.desc: Test CreatePixelMap
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, TC037, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct jpeg data and jpeg format hit.
     * @tc.expected: step1. create image source success.
     */
    IncrementalSourceOptions opts;
    uint32_t errorCode = 0;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateIncrementalImageSource(opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
}

/**
 * @tc.name: TC038
 * @tc.desc: Test jpeg decode
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, TC038, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct jpeg file path and jpeg format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/jpeg";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_JPEG_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);

    /**
     * @tc.steps: step2. decode image source to pixel map by default decode options.
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    ASSERT_EQ(pixelMap->GetAlphaType(), AlphaType::IMAGE_ALPHA_TYPE_OPAQUE);
}

/**
 * @tc.name: TC055
 * @tc.desc: Test StartPacking
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, TC055, TestSize.Level3)
{
GTEST_LOG_(INFO) << "ImageSourceJpegTest: TC055 start";
/**
     * @tc.steps: step1. create image source by buffer source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool ret = ImageUtils::GetFileSize(IMAGE_INPUT_JPEG_PATH, bufferSize);
    ASSERT_EQ(ret, true);
    uint8_t *buffer = static_cast<uint8_t *>(malloc(bufferSize));
    ASSERT_NE(buffer, nullptr);
    ret = OHOS::ImageSourceUtil::ReadFileToBuffer(IMAGE_INPUT_JPEG_PATH, buffer, bufferSize);
    ASSERT_EQ(ret, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(buffer, bufferSize, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map by default decode options
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap, nullptr);
    ASSERT_NE(pixelMap.get(), nullptr);

    ImageInfo imageInfo;
    pixelMap->GetImageInfo(imageInfo);
    decodeOpts.CropRect = { imageInfo.size.width - 1, imageInfo.size.height - 1, 1, 1 };
    std::unique_ptr<PixelMap> cropPixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_NE(cropPixelMap, nullptr);
    ASSERT_NE(cropPixelMap.get(), nullptr);
    cropPixelMap->GetImageInfo(imageInfo);
    ASSERT_EQ(imageInfo.size.width, 1);
    ASSERT_EQ(imageInfo.size.height, 1);
    /**
     * @tc.steps: step3. compress the pixel map to jpeg file.
     * @tc.expected: step3. pack pixel map success and the jpeg compress file size equals to JPEG_PACK_SIZE.
     */
    ImagePacker imagePacker;
    int64_t packSize = OHOS::ImageSourceUtil::PackImage(IMAGE_OUTPUT_JPEG_BUFFER_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);
    free(buffer);

    GTEST_LOG_(INFO) << "ImageSourceJpegTest: TC055 end";
}

/**
 * @tc.name: TC056
 * @tc.desc: Test StartPacking
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, TC056, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "ImageSourceJpegTest: TC056 start";
/**
     * @tc.steps: step1. create image source by correct jpeg file path and jpeg format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/jpeg";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_JPEG_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. get support decode image format.
     * @tc.expected: step2. get support format info success.
     */
    std::set<string> formats;
    uint32_t ret = imageSource->GetSupportedFormats(formats);
    ASSERT_EQ(ret, SUCCESS);
    /**
     * @tc.steps: step3. decode image source to pixel map by default decode options.
     * @tc.expected: step3. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap, nullptr);
    ASSERT_NE(pixelMap.get(), nullptr);
    ASSERT_EQ(pixelMap->GetAlphaType(), AlphaType::IMAGE_ALPHA_TYPE_OPAQUE);
    /**
     * @tc.steps: step4. get image source information.
     * @tc.expected: step4. get image source information success and source state is parsed.
     */
    SourceInfo sourceInfo = imageSource->GetSourceInfo(errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_EQ(sourceInfo.state, SourceInfoState::FILE_INFO_PARSED);
    /**
     * @tc.steps: step5. compress the pixel map to jpeg file.
     * @tc.expected: step5. pack pixel map success and the jpeg compress file size equals to JPEG_PACK_SIZE.
     */
    int64_t packSize = OHOS::ImageSourceUtil::PackImage(IMAGE_OUTPUT_JPEG_FILE_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);
    GTEST_LOG_(INFO) << "ImageSourceJpegTest: TC056 end";
}

/**
 * @tc.name: TC057
 * @tc.desc: Test StartPacking
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, TC057, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "ImageSourceJpegTest: TC057 start";
   /**
     * @tc.steps: step1. create image source by istream source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    fs->open(IMAGE_INPUT_JPEG_PATH, std::fstream::binary | std::fstream::in);
    bool isOpen = fs->is_open();
    ASSERT_EQ(isOpen, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(std::move(fs), opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map by default decode options
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap, nullptr);
    ASSERT_NE(pixelMap.get(), nullptr);
    /**
     * @tc.steps: step3. compress the pixel map to jpeg file.
     * @tc.expected: step3. pack pixel map success and the jpeg compress file size equals to JPEG_PACK_SIZE.
     */
    int64_t packSize = OHOS::ImageSourceUtil::PackImage(IMAGE_OUTPUT_JPEG_ISTREAM_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);

    GTEST_LOG_(INFO) << "ImageSourceJpegTest: TC057 end";
}

/**
 * @tc.name: TC059
 * @tc.desc: Test AddImage ImageSource
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, TC059, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "ImageSourceJpegTest: TC059 start";
    /**
     * @tc.steps: step1. create iamgesource
     * @tc.expected: step1. create iamgesource success
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/jpeg";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_JPEG_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);

    uint32_t index = 0;
    DecodeOptions optsPixel;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(index, optsPixel, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ImagePacker imagePacker;
    imagePacker.AddImage(*imageSource);
    ASSERT_NE(pixelMap.get(), nullptr);
    GTEST_LOG_(INFO) << "ImageSourceJpegTest: TC059 end";
}

/**
 * @tc.name: TC061
 * @tc.desc: Test GetSupportedFormats
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, TC061, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "ImageSourceJpegTest: TC061 start";
    /**
     * @tc.steps: step1.GetSupportedFormats(formats)
     * @tc.expected: step1. GetSupportedFormats(formats) success
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/jpeg";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_JPEG_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    std::set<std::string> formats;
    uint32_t ret = imageSource->GetSupportedFormats(formats);
    ASSERT_EQ(ret, SUCCESS);
    GTEST_LOG_(INFO) << "ImageSourceJpegTest: TC061 end";
}

/**
 * @tc.name: JpegImageDecode001
 * @tc.desc: Decode jpeg image from file source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, JpegImageDecode001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct jpeg file path and jpeg format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/jpeg";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_JPEG_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. get support decode image format.
     * @tc.expected: step2. get support format info success.
     */
    std::set<string> formats;
    uint32_t ret = imageSource->GetSupportedFormats(formats);
    ASSERT_EQ(ret, SUCCESS);
    /**
     * @tc.steps: step3. decode image source to pixel map by default decode options.
     * @tc.expected: step3. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap, nullptr);
    ASSERT_NE(pixelMap.get(), nullptr);
    ASSERT_EQ(pixelMap->GetAlphaType(), AlphaType::IMAGE_ALPHA_TYPE_OPAQUE);
    /**
     * @tc.steps: step4. get image source information.
     * @tc.expected: step4. get image source information success and source state is parsed.
     */
    SourceInfo sourceInfo = imageSource->GetSourceInfo(errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_EQ(sourceInfo.state, SourceInfoState::FILE_INFO_PARSED);
    /**
     * @tc.steps: step5. compress the pixel map to jpeg file.
     * @tc.expected: step5. pack pixel map success and the jpeg compress file size equals to JPEG_PACK_SIZE.
     */
    int64_t packSize = OHOS::ImageSourceUtil::PackImage(IMAGE_OUTPUT_JPEG_FILE_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);
}

/**
 * @tc.name: JpegImageDecode002
 * @tc.desc: Create image source by correct jpeg file path and wrong format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, JpegImageDecode002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct jpeg file path and default format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_JPEG_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. get image info from input image.
     * @tc.expected: step2. get image info success.
     */
    ImageInfo imageInfo;
    uint32_t ret = imageSource->GetImageInfo(0, imageInfo);
    ASSERT_EQ(ret, SUCCESS);
    ret = imageSource->GetImageInfo(imageInfo);
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: JpegImageDecode003
 * @tc.desc: Create image source by correct jpeg file path and wrong format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, JpegImageDecode003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct jpeg file path and wrong format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/png";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_JPEG_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
}

/**
 * @tc.name: JpegImageDecode004
 * @tc.desc: Create image source by wrong jpeg file path and default format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, JpegImageDecode004, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by wrong jpeg file path and default format hit.
     * @tc.expected: step1. create image source error.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource("/data/jpeg/test.jpg", opts, errorCode);
    ASSERT_EQ(errorCode, ERR_IMAGE_SOURCE_DATA);
    ASSERT_EQ(imageSource.get(), nullptr);
}

/**
 * @tc.name: JpegImageDecode005
 * @tc.desc: Decode jpeg image from buffer source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, JpegImageDecode005, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by buffer source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool ret = ImageUtils::GetFileSize(IMAGE_INPUT_JPEG_PATH, bufferSize);
    ASSERT_EQ(ret, true);
    uint8_t *buffer = static_cast<uint8_t *>(malloc(bufferSize));
    ASSERT_NE(buffer, nullptr);
    ret = OHOS::ImageSourceUtil::ReadFileToBuffer(IMAGE_INPUT_JPEG_PATH, buffer, bufferSize);
    ASSERT_EQ(ret, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(buffer, bufferSize, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map by default decode options
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);

    ImageInfo imageInfo;
    pixelMap->GetImageInfo(imageInfo);
    decodeOpts.CropRect = { imageInfo.size.width - 1, imageInfo.size.height - 1, 1, 1 };
    std::unique_ptr<PixelMap> cropPixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_NE(pixelMap, nullptr);
    ASSERT_NE(pixelMap.get(), nullptr);
    cropPixelMap->GetImageInfo(imageInfo);
    ASSERT_EQ(imageInfo.size.width, 1);
    ASSERT_EQ(imageInfo.size.height, 1);
    /**
     * @tc.steps: step3. compress the pixel map to jpeg file.
     * @tc.expected: step3. pack pixel map success and the jpeg compress file size equals to JPEG_PACK_SIZE.
     */
    ImagePacker imagePacker;
    int64_t packSize = OHOS::ImageSourceUtil::PackImage(IMAGE_OUTPUT_JPEG_BUFFER_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);
    free(buffer);
}

/**
 * @tc.name: JpegImageDecode006
 * @tc.desc: Decode jpeg image from istream source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, JpegImageDecode006, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by istream source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    fs->open(IMAGE_INPUT_JPEG_PATH, std::fstream::binary | std::fstream::in);
    bool isOpen = fs->is_open();
    ASSERT_EQ(isOpen, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(std::move(fs), opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map by default decode options
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap, nullptr);
    ASSERT_NE(pixelMap.get(), nullptr);
    /**
     * @tc.steps: step3. compress the pixel map to jpeg file.
     * @tc.expected: step3. pack pixel map success and the jpeg compress file size equals to JPEG_PACK_SIZE.
     */
    int64_t packSize = OHOS::ImageSourceUtil::PackImage(IMAGE_OUTPUT_JPEG_ISTREAM_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);
}

/**
 * @tc.name: JpegImageDecode007
 * @tc.desc: Decode jpeg image from incremental source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, JpegImageDecode007, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by incremental source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool fileRet = ImageUtils::GetFileSize(IMAGE_INPUT_JPEG_PATH, bufferSize);
    ASSERT_EQ(fileRet, true);
    uint8_t *buffer = static_cast<uint8_t *>(malloc(bufferSize));
    ASSERT_NE(buffer, nullptr);
    fileRet = OHOS::ImageSourceUtil::ReadFileToBuffer(IMAGE_INPUT_JPEG_PATH, buffer, bufferSize);
    ASSERT_EQ(fileRet, true);
    uint32_t errorCode = 0;
    IncrementalSourceOptions incOpts;
    incOpts.incrementalMode = IncrementalMode::INCREMENTAL_DATA;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateIncrementalImageSource(incOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. update incremental stream every 10 ms with random data size and promote decode
     * image to pixel map by default decode options
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<IncrementalPixelMap> incPixelMap = imageSource->CreateIncrementalPixelMap(0, decodeOpts,
        errorCode);
    ASSERT_NE(incPixelMap, nullptr);
    uint32_t updateSize = 0;
    srand(time(nullptr));
    bool isCompleted = false;
    while (updateSize < bufferSize) {
        uint32_t updateOnceSize = rand() % 1024;
        if (updateSize + updateOnceSize > bufferSize) {
            updateOnceSize = bufferSize - updateSize;
            isCompleted = true;
        }
        uint32_t ret = imageSource->UpdateData(buffer + updateSize, updateOnceSize, isCompleted);
        ASSERT_EQ(ret, SUCCESS);
        uint8_t decodeProgress = 0;
        incPixelMap->PromoteDecoding(decodeProgress);
        updateSize += updateOnceSize;
        usleep(DEFAULT_DELAY_UTIME);
    }
    incPixelMap->DetachFromDecoding();
    IncrementalDecodingStatus status = incPixelMap->GetDecodingStatus();
    ASSERT_EQ(status.decodingProgress, 100);
    /**
     * @tc.steps: step3. compress the pixel map to jpeg file.
     * @tc.expected: step3. pack pixel map success and the jpeg compress file size equals to JPEG_PACK_SIZE.
     */
    int64_t packSize = OHOS::ImageSourceUtil::PackImage(IMAGE_OUTPUT_JPEG_INC_PATH, std::move(incPixelMap));
    ASSERT_NE(packSize, 0);
    free(buffer);
}

/**
 * @tc.name: JpegImageDecode008
 * @tc.desc: Decode jpeg image multiple times from one ImageSource
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, JpegImageDecode008, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by jpeg file path.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_JPEG_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map by default decode options.
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap1 = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap1, nullptr);
    ASSERT_NE(pixelMap1.get(), nullptr);
    /**
     * @tc.steps: step3. decode image source to pixel map by default decode options again.
     * @tc.expected: step3. decode image source to pixel map success.
     */
    std::unique_ptr<PixelMap> pixelMap2 = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap2, nullptr);
    ASSERT_NE(pixelMap2.get(), nullptr);
    /**
     * @tc.steps: step4. compress the pixel map to jpeg file.
     * @tc.expected: step4. pack pixel map success and the jpeg compress file size equals to JPEG_PACK_SIZE.
     */
    int64_t packSize = OHOS::ImageSourceUtil::PackImage(IMAGE_OUTPUT_JPEG_MULTI_FILE1_PATH, std::move(pixelMap1));
    ASSERT_NE(packSize, 0);
    packSize = OHOS::ImageSourceUtil::PackImage(IMAGE_OUTPUT_JPEG_MULTI_FILE2_PATH, std::move(pixelMap2));
    ASSERT_NE(packSize, 0);
}

/**
 * @tc.name: JpegImageDecode009
 * @tc.desc: Decode jpeg image by incremental mode and then decode again by one-time mode.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, JpegImageDecode009, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by incremental source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool fileRet = ImageUtils::GetFileSize(IMAGE_INPUT_JPEG_PATH, bufferSize);
    ASSERT_EQ(fileRet, true);
    uint8_t *buffer = static_cast<uint8_t *>(malloc(bufferSize));
    ASSERT_NE(buffer, nullptr);
    fileRet = OHOS::ImageSourceUtil::ReadFileToBuffer(IMAGE_INPUT_JPEG_PATH, buffer, bufferSize);
    ASSERT_EQ(fileRet, true);
    uint32_t errorCode = 0;
    IncrementalSourceOptions incOpts;
    incOpts.incrementalMode = IncrementalMode::INCREMENTAL_DATA;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateIncrementalImageSource(incOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. update incremental stream every 10 ms with random data size and promote decode
     * image to pixel map by default decode options
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<IncrementalPixelMap> incPixelMap = imageSource->CreateIncrementalPixelMap(0, decodeOpts,
        errorCode);
    ASSERT_NE(incPixelMap, nullptr);
    uint32_t updateSize = 0;
    srand(time(nullptr));
    bool isCompleted = false;
    while (updateSize < bufferSize) {
        uint32_t updateOnceSize = rand() % 1024;
        if (updateSize + updateOnceSize > bufferSize) {
            updateOnceSize = bufferSize - updateSize;
            isCompleted = true;
        }
        uint32_t ret = imageSource->UpdateData(buffer + updateSize, updateOnceSize, isCompleted);
        ASSERT_EQ(ret, SUCCESS);
        uint8_t decodeProgress = 0;
        incPixelMap->PromoteDecoding(decodeProgress);
        updateSize += updateOnceSize;
        usleep(DEFAULT_DELAY_UTIME);
    }
    incPixelMap->DetachFromDecoding();
    IncrementalDecodingStatus status = incPixelMap->GetDecodingStatus();
    ASSERT_EQ(status.decodingProgress, 100);
    /**
     * @tc.steps: step3. decode image source to pixel map by default decode options again.
     * @tc.expected: step3. decode image source to pixel map success.
     */
    std::unique_ptr<PixelMap> pixelMap1 = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap1, nullptr);
    ASSERT_NE(pixelMap1.get(), nullptr);
    /**
     * @tc.steps: step4. compress the pixel map to jpeg file.
     * @tc.expected: step4. pack pixel map success and the jpeg compress file size equals to JPEG_PACK_SIZE.
     */
    int64_t packSize = OHOS::ImageSourceUtil::PackImage(IMAGE_OUTPUT_JPEG_MULTI_INC1_PATH, std::move(incPixelMap));
    ASSERT_NE(packSize, 0);
    packSize = OHOS::ImageSourceUtil::PackImage(IMAGE_OUTPUT_JPEG_MULTI_ONETIME1_PATH, std::move(pixelMap1));
    ASSERT_NE(packSize, 0);
    free(buffer);
}

/**
 * @tc.name: JpegImageDecode010
 * @tc.desc: Decode jpeg image by one-time mode and then decode again by incremental mode.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, JpegImageDecode010, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by incremental source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool fileRet = ImageUtils::GetFileSize(IMAGE_INPUT_JPEG_PATH, bufferSize);
    ASSERT_EQ(fileRet, true);
    uint8_t *buffer = static_cast<uint8_t *>(malloc(bufferSize));
    ASSERT_NE(buffer, nullptr);
    fileRet = OHOS::ImageSourceUtil::ReadFileToBuffer(IMAGE_INPUT_JPEG_PATH, buffer, bufferSize);
    ASSERT_EQ(fileRet, true);
    uint32_t errorCode = 0;
    IncrementalSourceOptions incOpts;
    incOpts.incrementalMode = IncrementalMode::INCREMENTAL_DATA;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateIncrementalImageSource(incOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. update incremental stream every 10 ms with random data size.
     * @tc.expected: step2. update success.
     */
    uint32_t updateSize = 0;
    srand(time(nullptr));
    bool isCompleted = false;
    while (updateSize < bufferSize) {
        uint32_t updateOnceSize = rand() % 1024;
        if (updateSize + updateOnceSize > bufferSize) {
            updateOnceSize = bufferSize - updateSize;
            isCompleted = true;
        }
        uint32_t ret = imageSource->UpdateData(buffer + updateSize, updateOnceSize, isCompleted);
        ASSERT_EQ(ret, SUCCESS);
        updateSize += updateOnceSize;
        usleep(DEFAULT_DELAY_UTIME);
    }

    /**
     * @tc.steps: step3. decode image source to pixel map by default decode options.
     * @tc.expected: step3. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap1 = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap1, nullptr);
    ASSERT_NE(pixelMap1.get(), nullptr);

    /**
     * @tc.steps: step4. decode image source to pixel map by incremental mode again.
     * @tc.expected: step4. decode image source to pixel map success.
     */
    std::unique_ptr<IncrementalPixelMap> incPixelMap = imageSource->CreateIncrementalPixelMap(0, decodeOpts,
        errorCode);
    ASSERT_NE(incPixelMap, nullptr);
    uint8_t decodeProgress = 0;
    incPixelMap->PromoteDecoding(decodeProgress);
    incPixelMap->DetachFromDecoding();
    IncrementalDecodingStatus status = incPixelMap->GetDecodingStatus();
    ASSERT_EQ(status.decodingProgress, 100);
    /**
     * @tc.steps: step4. compress the pixel map to jpeg file.
     * @tc.expected: step4. pack pixel map success and the jpeg compress file size equals to JPEG_PACK_SIZE.
     */
    int64_t packSize = OHOS::ImageSourceUtil::PackImage(IMAGE_OUTPUT_JPEG_MULTI_INC2_PATH, std::move(incPixelMap));
    ASSERT_NE(packSize, 0);
    packSize = OHOS::ImageSourceUtil::PackImage(IMAGE_OUTPUT_JPEG_MULTI_ONETIME2_PATH, std::move(pixelMap1));
    ASSERT_NE(packSize, 0);
    free(buffer);
}

/**
 * @tc.name: JpgImageCrop001
 * @tc.desc: Crop jpg image from istream source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, JpgImageCrop001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create jpg image source by istream source stream and default format hit
     * @tc.expected: step1. create jpg image source success.
     */
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    fs->open("/data/local/tmp/image/test.jpg", std::fstream::binary | std::fstream::in);
    bool isOpen = fs->is_open();
    ASSERT_EQ(isOpen, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(std::move(fs), opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. crop jpg image source to pixel map crop options
     * @tc.expected: step2. crop jpg image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    decodeOpts.CropRect.top = 3;
    decodeOpts.CropRect.width = 100;
    decodeOpts.CropRect.left = 3;
    decodeOpts.CropRect.height = 200;
    decodeOpts.desiredSize.width = 200;
    decodeOpts.desiredSize.height = 400;
    decodeOpts.rotateDegrees = 90;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    HiLog::Debug(LABEL_TEST, "create pixel map error code=%{public}u.", errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    EXPECT_EQ(200, pixelMap->GetWidth());
    EXPECT_EQ(400, pixelMap->GetHeight());
}

/**
 * @tc.name: JpegImageHwDecode001
 * @tc.desc: Hardware decode jpeg image from file source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceJpegTest, JpegImageHwDecode001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct jpeg file path and jpeg format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/jpeg";
    std::unique_ptr<ImageSource> imageSource =
        ImageSource::CreateImageSource(IMAGE_INPUT_HW_JPEG_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map by default decode options
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    HiLog::Debug(LABEL_TEST, "create pixel map ret=%{public}u.", errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    /**
     * @tc.steps: step3. compress the pixel map to jpeg file.
     * @tc.expected: step3. pack pixel map success and the jpeg compress file size equals to JPEG_PACK_SIZE.
     */
    int64_t packSize = OHOS::ImageSourceUtil::PackImage(IMAGE_OUTPUT_HW_JPEG_FILE_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);
}
} // namespace Multimedia
} // namespace OHOS