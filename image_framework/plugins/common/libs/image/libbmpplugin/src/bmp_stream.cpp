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

#include "bmp_stream.h"

namespace OHOS {
namespace ImagePlugin {
using namespace OHOS::HiviewDFX;
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "BmpStream" };
BmpStream::BmpStream(InputDataStream *stream) : inputStream_(stream) {}

size_t BmpStream::read(void *buffer, size_t size)
{
    if (inputStream_ == nullptr) {
        HiLog::Error(LABEL, "read failed, inputStream_ is null");
        return 0;
    }
    if (buffer == nullptr) {
        size_t curPosition = static_cast<size_t>(inputStream_->Tell());
        if (!inputStream_->Seek(curPosition + size)) {
            HiLog::Error(LABEL, "read failed, curpositon=%{public}zu, skip size=%{public}zu", curPosition, size);
            return 0;
        }
        return size;
    }
    uint32_t desireSize = static_cast<uint32_t>(size);
    uint32_t bufferSize = desireSize;
    uint32_t readSize = desireSize;
    if (!inputStream_->Read(desireSize, static_cast<uint8_t *>(buffer), bufferSize, readSize)) {
        HiLog::Error(LABEL, "read failed, desire read size=%{public}u", desireSize);
        return 0;
    }
    return readSize;
}

size_t BmpStream::peek(void *buffer, size_t size) const
{
    if (inputStream_ == nullptr) {
        HiLog::Error(LABEL, "peek failed, inputStream_ is null");
        return 0;
    }
    if (buffer == nullptr) {
        HiLog::Error(LABEL, "peek failed, output buffer is null");
        return 0;
    }
    uint32_t desireSize = static_cast<uint32_t>(size);
    uint32_t bufferSize = desireSize;
    uint32_t readSize = desireSize;
    if (!inputStream_->Peek(desireSize, static_cast<uint8_t *>(buffer), bufferSize, readSize)) {
        HiLog::Error(LABEL, "peek failed, desire peek size=%{public}u", desireSize);
        return 0;
    }
    return readSize;
}

bool BmpStream::isAtEnd() const
{
    if (inputStream_ == nullptr) {
        HiLog::Error(LABEL, "get stream status failed, inputStream_ is null.");
        return false;
    }
    size_t size = inputStream_->GetStreamSize();
    return (inputStream_->Tell() == size);
}
} // namespace ImagePlugin
} // namespace OHOS
