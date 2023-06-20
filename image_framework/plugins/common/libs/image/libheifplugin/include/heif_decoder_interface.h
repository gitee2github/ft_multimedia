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

#ifndef HEIF_DECODER_INTERFACE_H
#define HEIF_DECODER_INTERFACE_H

#include "abs_image_decoder.h"
#include "image_plugin_type.h"
#include "input_data_stream.h"

namespace OHOS {
namespace ImagePlugin {
class HeifDecoderInterface {
public:
    HeifDecoderInterface() = default;
    virtual ~HeifDecoderInterface() = default;
    virtual void GetHeifSize(PlSize &size) = 0;
    virtual void SetAllowPartial(const bool isAllowPartialImage) = 0;
    virtual bool ConversionSupported(const PlPixelFormat &plPixelFormat, int32_t &bytesPerPixel) = 0;
    virtual uint32_t OnGetPixels(const PlSize &dstSize, const uint32_t dstRowBytes, DecodeContext &context) = 0;
};
} // namespace ImagePlugin
} // namespace OHOS

#endif // HEIF_DECODER_INTERFACE_H
