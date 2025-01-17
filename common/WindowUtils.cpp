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

#include "WindowUtils.h"

#include <lvgl/lvgl.h>

#include "wm/LayoutParams.h"

uint32_t getLvColorFormatType(uint32_t format) {
    uint32_t value;
    switch (format) {
        case os::wm::LayoutParams::FORMAT_RGB_565:
            value = LV_COLOR_FORMAT_RGB565;
            break;

        case os::wm::LayoutParams::FORMAT_RGB_565A8:
            value = LV_COLOR_FORMAT_RGB565A8;
            break;
        case os::wm::LayoutParams::FORMAT_RGB_888:
            value = LV_COLOR_FORMAT_RGB888;
            break;
        case os::wm::LayoutParams::FORMAT_XRGB_8888:
            value = LV_COLOR_FORMAT_XRGB8888;
            break;

        case os::wm::LayoutParams::FORMAT_ARGB_8888:
        default:
            value = LV_COLOR_FORMAT_ARGB8888;
            break;
    }
    return value;
}

uint64_t curSysTimeMs(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

uint64_t curSysTimeUs(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

uint64_t curSysTimeNs(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}
