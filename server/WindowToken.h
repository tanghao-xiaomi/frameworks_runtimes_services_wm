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

#include <vector>

namespace os {
namespace wm {

using android::IBinder;
using android::sp;

class WindowManagerService;
class WindowState;

class WindowToken {
public:
    WindowToken(WindowManagerService* service, const sp<IBinder>& token, int32_t type,
                int32_t displayId);
    ~WindowToken();

    void addWindow(WindowState* win);
    bool isClientVisible();
    void setClientVisible(bool clientVisible);

private:
    WindowManagerService* mService;
    sp<IBinder> mToken;
    int32_t mType;
    std::vector<WindowState*> mChildren;
    bool mClientVisible;
};

} // namespace wm
} // namespace os
