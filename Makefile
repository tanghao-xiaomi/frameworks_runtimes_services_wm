#
# Copyright (C) 2023 Xiaomi Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

include $(APPDIR)/Make.defs

CXXEXT   := .cpp

MODULE    = $(CONFIG_SYSTEM_WINDOW_SERVICE)
PRIORITY  = $(CONFIG_SYSTEM_WINDOW_SERVICE_PRIORITY)
STACKSIZE = $(CONFIG_SYSTEM_WINDOW_SERVICE_STACKSIZE)

ifneq ($(CONFIG_SYSTEM_WINDOW_SERVICE),)
AIDLSRCS += $(shell find aidl -name *.aidl)
AIDLFLAGS = --lang=cpp -Iaidl -haidl -oaidl

CXXSRCS  += $(shell find common server app -name *.cpp)
CXXSRCS  += $(patsubst %$(AIDLEXT),%$(CXXEXT),$(AIDLSRCS))
endif

ASRCS := $(wildcard $(ASRCS))
CSRCS := $(wildcard $(CSRCS))
CXXSRCS := $(wildcard $(CXXSRCS))
MAINSRC := $(wildcard $(MAINSRC))
NOEXPORTSRCS = $(ASRCS)$(CSRCS)$(CXXSRCS)$(MAINSRC)

ifneq ($(NOEXPORTSRCS),)
BIN := $(APPDIR)/staging/libframework.a
endif

CSRCS += $(wildcard server/lvgl/*.c)

ifneq ($(CONFIG_SYSTEM_WINDOW_SERVICE_TEST),)
CSRCS += $(wildcard test/*.c)
endif

EXPORT_FILES := include

include $(APPDIR)/Application.mk
