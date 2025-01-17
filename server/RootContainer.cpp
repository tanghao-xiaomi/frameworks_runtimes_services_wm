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

#define LOG_TAG "WMS:Root"

#include "RootContainer.h"

#include <lvgl/lvgl.h>

#include "../common/WindowUtils.h"
#include "WindowManagerService.h"

namespace os {
namespace wm {
#ifdef CONFIG_SYSTEM_WINDOW_USE_VSYNC_EVENT
static void vsyncEventReceived(lv_event_t* e);
#endif

RootContainer::RootContainer(DeviceEventListener* listener, uv_loop_t* loop)
      : mListener(listener),
        mDisp(nullptr),
        mVsyncEnabled(false),
#ifndef CONFIG_SYSTEM_WINDOW_USE_VSYNC_EVENT
        mVsyncTimer(nullptr),
#endif
        mUvData(nullptr),
        mUvLoop(loop),
        mTraceFrame(false) {
    mReady = init();
    if (mReady) {
        // set bg color to black for lvgl
        lv_obj_set_style_bg_color(lv_display_get_screen_active(mDisp), lv_color_black(), 0);
    }
}

RootContainer::~RootContainer() {
    LV_GLOBAL_DEFAULT()->user_data = nullptr;

#ifdef CONFIG_SYSTEM_WINDOW_USE_VSYNC_EVENT
    if (mVsyncEnabled && mDisp) lv_display_unregister_vsync_event(mDisp, vsyncEventReceived, this);
#else
    if (mVsyncTimer) lv_timer_del(mVsyncTimer);
#endif

    lv_anim_del_all();

    lv_nuttx_uv_deinit(&mUvData);
    mUvData = nullptr;
    mUvLoop = nullptr;

    if (mDisp) {
        lv_disp_remove(mDisp);
        mDisp = nullptr;
    }

    mListener = nullptr;

    lv_nuttx_deinit(&mResult);

    lv_deinit();
}

lv_disp_t* RootContainer::getRoot() {
    return mDisp;
}

lv_obj_t* RootContainer::getDefLayer() {
    return lv_disp_get_scr_act(mDisp);
}

lv_obj_t* RootContainer::getSysLayer() {
    return lv_disp_get_layer_sys(mDisp);
}

lv_obj_t* RootContainer::getTopLayer() {
    return lv_disp_get_layer_top(mDisp);
}

#ifdef CONFIG_SYSTEM_WINDOW_USE_VSYNC_EVENT
static void vsyncEventReceived(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VSYNC) {
        WM_PROFILER_BEGIN();

        RootContainer* container = reinterpret_cast<RootContainer*>(lv_event_get_user_data(e));
        if (container && container->vsyncEnabled()) {
            container->processVsyncEvent();
        }

        WM_PROFILER_END();
    }
}

static void asyncEnableVsync(lv_timer_t* tmr) {
    RootContainer* container = static_cast<RootContainer*>(lv_timer_get_user_data(tmr));
    if (!container) return;

    if (container->vsyncEnabled()) {
        FLOGD("register");
        lv_display_register_vsync_event(container->getRoot(), vsyncEventReceived, container);
    } else {
        FLOGD("unregister");
        lv_display_unregister_vsync_event(container->getRoot(), vsyncEventReceived, container);
    }
}

#else
static void vsyncCallback(lv_timer_t* tmr) {
    RootContainer* container = static_cast<RootContainer*>(lv_timer_get_user_data(tmr));
    if (container) {
        container->processVsyncEvent();
    }
}
#endif

void RootContainer::enableVsync(bool enable) {
    WM_PROFILER_BEGIN();

    if (mVsyncEnabled == enable) {
        return;
    }

    FLOGI("%s fb vsync event", enable ? "enable" : "disable");
    mVsyncEnabled = enable;
#ifdef CONFIG_SYSTEM_WINDOW_USE_VSYNC_EVENT
#if 0
    lv_timer_t* timer = lv_timer_create(asyncEnableVsync, 0, this);
    lv_timer_set_repeat_count(timer, 1);
#else
    if (vsyncEnabled()) {
        FLOGD("register vsync event");
        lv_display_register_vsync_event(container->getRoot(), vsyncEventReceived, container);
    } else {
        FLOGD("unregister vsync event");
        lv_display_unregister_vsync_event(container->getRoot(), vsyncEventReceived, container);
    }
#endif
#else
    if (mVsyncTimer) {
        if (enable && mVsyncTimer->paused) {
            FLOGD("enable fb vsync timer");
            lv_timer_resume(mVsyncTimer);
        } else if (!enable && !mVsyncTimer->paused) {
            FLOGD("disable fb vsync timer");
            lv_timer_pause(mVsyncTimer);
        }
    }
#endif
    WM_PROFILER_END();
}

void RootContainer::processVsyncEvent() {
    WM_PROFILER_BEGIN();
    if (mListener) {
        mListener->responseVsync();
    }
    WM_PROFILER_END();
}

static bool monitor_indev_read(lv_indev_t* indev, lv_indev_data_t* data) {
    if (!data) return false;

    RootContainer* container = reinterpret_cast<RootContainer*>(LV_GLOBAL_DEFAULT()->user_data);
    if (container) return container->readInput(indev, data);
    return false;
}

bool RootContainer::readInput(lv_indev_t* indev, lv_indev_data_t* data) {
    if (!mListener) return false;

    int type = lv_indev_get_type(indev);
    InputMessage msg;

    switch (type) {
        case LV_INDEV_TYPE_POINTER:
            msg.pointer.x = msg.pointer.raw_x = data->point.x;
            msg.pointer.y = msg.pointer.raw_y = data->point.y;
            msg.pointer.gesture_state = 0;
            break;
        case LV_INDEV_TYPE_KEYPAD:
            msg.keypad.key_code = data->key;
            break;
        default:
            return false;
    }

    msg.type = (InputMessageType)type;
    msg.state = (InputMessageState)data->state;
    return mListener->responseInput(&msg);
}

FrameMetaInfo* RootContainer::frameInfo() {
    return mTraceFrame ? &mFrameInfo : nullptr;
}

#define CONTAINER_FROM_EVENT(e)                                             \
    RootContainer* container = (RootContainer*)(lv_event_get_user_data(e)); \
    if (container == nullptr) {                                             \
        return;                                                             \
    }

static void processDispEvent(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);

    switch (code) {
        case LV_EVENT_REFR_START: {
            CONTAINER_FROM_EVENT(e);
            container->onFrameStart();
            break;
        }
        case LV_EVENT_RENDER_START: {
            CONTAINER_FROM_EVENT(e);
            container->onRenderStart();
            break;
        }

        case LV_EVENT_REFR_READY: {
            CONTAINER_FROM_EVENT(e);
            container->onFrameFinished();
            break;
        }
        default:
            break;
    }
}

void RootContainer::onFrameStart() {
    auto info = frameInfo();
    if (info) {
        info->setVsync(FrameMetaInfo::getCurSysTime(), 0, LV_DEF_REFR_PERIOD);
        info->markLayoutStart();
    }

#ifndef CONFIG_SYSTEM_WINDOW_USE_VSYNC_EVENT
    if (!mVsyncTimer->paused) {
        lv_timer_reset(mVsyncTimer);
        if (mVsyncTimer->timer_cb) mVsyncTimer->timer_cb(mVsyncTimer);
    }
#endif
}

void RootContainer::onRenderStart() {
    if (!mTraceFrame) return;
    mFrameInfo.markRenderStart();
}

void RootContainer::onFrameFinished() {
    if (!mTraceFrame) return;

    mFrameInfo.markRenderEnd();
    mFrameInfo.markFrameFinished();
    mFrameTimeInfo.time(&mFrameInfo);

    FLOGI("SingleFrameLog{seq=%" PRId64 ", totalMs=%" PRId64 ", renderMs=%" PRId64
          ", layoutMs=%" PRId64 "}",
          mFrameInfo.getVsyncId(), mFrameInfo.totalDrawnDuration(),
          mFrameInfo.totalRenderDuration(), mFrameInfo.totalLayoutDuration());
}

bool RootContainer::init() {
    lv_init();
    lv_image_cache_resize(0, false);

#if LV_USE_NUTTX
    lv_nuttx_dsc_t info;

    lv_nuttx_dsc_init(&info);
    info.fb_path = CONFIG_SYSTEM_WINDOW_FBDEV_DEVICEPATH;
    info.input_path = CONFIG_SYSTEM_WINDOW_TOUCHPAD_DEVICEPATH;

    lv_nuttx_init(&info, &mResult);
    if (mResult.disp == nullptr) {
        FLOGE("Failed to open fb device:%s. Please check device node and its access "
              "permissions.",
              info.fb_path);
        return false;
    }

    mDisp = mResult.disp;
    lv_nuttx_uv_t uv_info = {
            .loop = mUvLoop,
            .disp = mResult.disp,
            .indev = mResult.indev,
            .uindev = mResult.utouch_indev,
    };
    mUvData = lv_nuttx_uv_init(&uv_info);

#ifndef CONFIG_SYSTEM_WINDOW_USE_VSYNC_EVENT
    mVsyncTimer = lv_timer_create(vsyncCallback, LV_DEF_REFR_PERIOD, this);
#endif
    lv_display_add_event_cb(mDisp, processDispEvent, LV_EVENT_ALL, this);

    if (mListener) {
        LV_GLOBAL_DEFAULT()->user_data = this;
        lv_indev_set_read_preprocess_cb(mResult.indev, monitor_indev_read);
        if (mResult.utouch_indev) {
            lv_indev_set_read_preprocess_cb(mResult.utouch_indev, monitor_indev_read);
        }
    }
#endif

    return mDisp ? true : false;
}

bool RootContainer::getDisplayInfo(DisplayInfo* info) {
    if (info) {
        info->width = lv_disp_get_hor_res(mDisp);
        info->height = lv_disp_get_ver_res(mDisp);
        return true;
    }

    return false;
}

static lv_anim_t* toast_fade_in(lv_obj_t* obj, uint32_t time, uint32_t delay);
static lv_anim_t* toast_fade_out(lv_obj_t* obj, uint32_t time, uint32_t delay);

void RootContainer::showToast(const char* text, uint32_t duration) {
    lv_obj_t* label = lv_label_create(lv_disp_get_layer_sys(mDisp));
    lv_label_set_text(label, text);
    lv_obj_set_style_bg_opa(label, LV_OPA_80, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_pad_all(label, 5, 0);
    lv_obj_set_style_radius(label, 10, 0);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_set_user_data(label, (void*)(uintptr_t)duration);

    toast_fade_in(label, 500, 100);
}

static void toast_fade_anim_cb(void* obj, int32_t v) {
    lv_obj_set_style_opa((lv_obj_t*)obj, v, 0);
}

static void toast_fade_in_anim_ready(lv_anim_t* a) {
    lv_obj_remove_local_style_prop((lv_obj_t*)(a->var), LV_STYLE_OPA, 0);
    lv_obj_t* target = (lv_obj_t*)(a->var);
    uint32_t duration = (uintptr_t)lv_obj_get_user_data(target);
    toast_fade_out(target, lv_anim_get_time(a), duration);
}

static void fade_out_anim_ready(lv_anim_t* a) {
    lv_obj_del((lv_obj_t*)(a->var));
}

static lv_anim_t* toast_fade_in(lv_obj_t* obj, uint32_t time, uint32_t delay) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, 0, LV_OPA_COVER);
    lv_anim_set_exec_cb(&a, toast_fade_anim_cb);
    lv_anim_set_ready_cb(&a, toast_fade_in_anim_ready);
    lv_anim_set_time(&a, time);
    lv_anim_set_delay(&a, delay);
    return lv_anim_start(&a);
}

static lv_anim_t* toast_fade_out(lv_obj_t* obj, uint32_t time, uint32_t delay) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, lv_obj_get_style_opa(obj, 0), LV_OPA_TRANSP);
    lv_anim_set_exec_cb(&a, toast_fade_anim_cb);
    lv_anim_set_ready_cb(&a, fade_out_anim_ready);
    lv_anim_set_time(&a, time);
    lv_anim_set_delay(&a, delay);
    return lv_anim_start(&a);
}

void RootContainer::traceFrame(bool enable) {
    if (mTraceFrame == enable) return;

    /* dump last frame information and disable trace */
    mTraceFrame = enable;
    if (!enable) {
        mFrameTimeInfo.time(nullptr);
    }
}

} // namespace wm
} // namespace os
