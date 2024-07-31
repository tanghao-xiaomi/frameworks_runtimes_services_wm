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

#include "os/wm/VsyncRequest.h"

namespace os {
namespace wm {

static inline VsyncRequest nextVsyncState(VsyncRequest req) {
    switch (req) {
        case VsyncRequest::VSYNC_REQ_NONE:
            return VsyncRequest::VSYNC_REQ_NONE;

        case VsyncRequest::VSYNC_REQ_SINGLE:
            return VsyncRequest::VSYNC_REQ_NONE;

        case VsyncRequest::VSYNC_REQ_SINGLESUPPRESS:
            return VsyncRequest::VSYNC_REQ_SINGLE;

        case VsyncRequest::VSYNC_REQ_PERIODIC:
            return VsyncRequest::VSYNC_REQ_PERIODIC;

        default:
            break;
    }

    return VsyncRequest::VSYNC_REQ_NONE;
}

static inline const char* VsyncRequestToString(VsyncRequest req) {
    switch (req) {
        case VsyncRequest::VSYNC_REQ_NONE:
            return "none";

        case VsyncRequest::VSYNC_REQ_SINGLE:
            return "single";

        case VsyncRequest::VSYNC_REQ_SINGLESUPPRESS:
            return "singlesuppress";

        case VsyncRequest::VSYNC_REQ_PERIODIC:
            return "periodic";

        default:
            break;
    }

    return "unknown";
}

} // namespace wm
} // namespace os
