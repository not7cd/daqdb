/**
 *  Copyright (c) 2019 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

#include "spdk/bdev.h"

#include "Rqst.h"
#include "SpdkConf.h"
#include "SpdkDevice.h"
#include <Logger.h>
#include <RTree.h>

namespace DaqDB {

class SpdkRAID0Bdev : public SpdkDevice {
  public:
    SpdkRAID0Bdev(void);
    ~SpdkRAID0Bdev() = default;

    /**
     * Initialize RAID0 devices.
     *
     * @return  if this RAID0 devices successfully configured and opened, false
     * otherwise
     */
    virtual bool init(const SpdkConf &conf);
    virtual void deinit();

    /*
     * SpdkDevice virtual interface
     */
    virtual int read(DeviceTask *task);
    virtual int write(DeviceTask *task);
    virtual int reschedule(DeviceTask *task);

    virtual void enableStats(bool en);
    inline virtual size_t getOptimalSize(size_t size) { return 0; }
    inline virtual size_t getAlignedSize(size_t size) { return 0; }
    inline virtual uint32_t getSizeInBlk(size_t &size) { return 0; }
    void virtual setReady() {}
    virtual bool isOffloadEnabled() { return true; }
    virtual bool isBdevFound() { return true; }
    virtual void IOQuiesce() {}
    virtual bool isIOQuiescent() { return true; }
    virtual void IOAbort() {}

    static SpdkDeviceClass bdev_class;
};

} // namespace DaqDB
