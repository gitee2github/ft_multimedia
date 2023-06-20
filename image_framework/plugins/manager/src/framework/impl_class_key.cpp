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

#include "impl_class_key.h"
#include "hilog/log_c.h"
#include "hilog/log_cpp.h"
#include "impl_class.h"
#include "iosfwd"
#include "log_tags.h"
#include "string"

namespace OHOS {
namespace MultimediaPlugin {
using namespace OHOS::HiviewDFX;

static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "ImplClassKey" };

void ImplClassKey::OnObjectDestroy()
{
    HiLog::Debug(LABEL, "destroy object: className: %{public}s, packageName: %{public}s,",
                 implClass_.GetClassName().c_str(), implClass_.GetPackageName().c_str());

    implClass_.OnObjectDestroy();
}
} // namespace MultimediaPlugin
} // namespace OHOS
