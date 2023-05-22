/*
 * Copyright (C) 2023 Xiaomi Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <binder/Parcel.h>
#include <binder/Parcelable.h>
#include <binder/Status.h>
#include <utils/RefBase.h>

namespace os {
namespace wm {

using namespace android;
using namespace android::base;
using namespace android::binder;
using namespace std;

class InputChannel : public Parcelable {
public:
    InputChannel();
    ~InputChannel();

    status_t writeToParcel(Parcel* out) const override;
    status_t readFromParcel(const Parcel* in) override;

private:
    String8 mqCliName;
    String8 mqSrvName;
};

} // namespace wm
} // namespace os
