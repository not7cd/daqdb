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

#include <iostream>

#include "spdk/stdinc.h"
#include "spdk/cpuset.h"
#include "spdk/queue.h"
#include "spdk/log.h"
#include "spdk/thread.h"
#include "spdk/event.h"
#include "spdk/ftl.h"
#include "spdk/conf.h"
#include "spdk/env.h"

#include "Rqst.h"
#include "SpdkBdev.h"
#include <Logger.h>

namespace DaqDB {

const std::string DEFAULT_SPDK_CONF_FILE = "spdk.conf";

SpdkBdev::SpdkBdev() : state(SpdkBdevState::SPDK_BDEV_INIT) {
    spBdevCtx.reset(new SpdkBdevCtx());
}

int SpdkBdev::read(OffloadRqst *rqst) { return 0; }

int SpdkBdev::write(OffloadRqst *rqst) { return 0; }

void SpdkBdev::deinit() {
  spdk_put_io_channel(spBdevCtx->io_channel);
  spdk_bdev_close(spBdevCtx->bdev_desc);
}

bool SpdkBdev::init(const char *bdev_name) {
    spBdevCtx->bdev_name = bdev_name;
    spBdevCtx->bdev = 0;
    spBdevCtx->bdev_desc = 0;
    spdk_bdev_opts bdev_opts;

    spBdevCtx->bdev = spdk_bdev_first();
    if (!spBdevCtx->bdev) {
        DAQ_CRITICAL(std::string("No NVMe devices detected for name[") + spBdevCtx->bdev_name + "]");
        spBdevCtx->state = SPDK_BDEV_NOT_FOUND;
        return false;
    }
    DAQ_DEBUG("NVMe devices detected for name[" + spBdevCtx->bdev_name + "]");

    int rc = spdk_bdev_open(spBdevCtx->bdev, true, NULL, NULL, &spBdevCtx->bdev_desc);
    if (rc) {
        DAQ_CRITICAL("Open BDEV failed with error code[" + std::to_string(rc) + "]");
        spBdevCtx->state = SPDK_BDEV_ERROR;
        return false;
    }

    spdk_bdev_get_opts(&bdev_opts);
    bdev_opts.bdev_io_cache_size = bdev_opts.bdev_io_pool_size >> 1;
    spdk_bdev_set_opts(&bdev_opts);
    spdk_bdev_get_opts(&bdev_opts);
    DAQ_DEBUG("bdev.bdev_io_pool_size[" + bdev_opts.bdev_io_pool_size + "]" +
              " bdev.bdev.io_cache_size[" + bdev_opts.bdev_io_cache_size + "]");

    spBdevCtx->io_pool_size = bdev_opts.bdev_io_pool_size;
    spBdevCtx->io_cache_size = bdev_opts.bdev_io_cache_size;

    spBdevCtx->io_channel = spdk_bdev_get_io_channel(spBdevCtx->bdev_desc);
    if (!spBdevCtx->io_channel) {
        DAQ_CRITICAL(std::string("Get io_channel failed bdev[") + spBdevCtx->bdev_name + "]");
        spBdevCtx->state = SPDK_BDEV_ERROR;
        return false;
    }

    spBdevCtx->blk_size = spdk_bdev_get_block_size(spBdevCtx->bdev);
    DAQ_DEBUG("BDEV block size[" + std::to_string(spBdevCtx->blk_size) + "]");

    spBdevCtx->data_blk_size = spdk_bdev_get_data_block_size(spBdevCtx->bdev);
    DAQ_DEBUG("BDEV data block size[" + spBdevCtx->data_blk_size + "]");

    spBdevCtx->buf_align = spdk_bdev_get_buf_align(spBdevCtx->bdev);
    DAQ_DEBUG("BDEV align[" + std::to_string(spBdevCtx->buf_align) + "]");

    spBdevCtx->blk_num = spdk_bdev_get_num_blocks(spBdevCtx->bdev);
    DAQ_DEBUG("BDEV number of blocks[" + std::to_string(spBdevCtx->blk_num) + "]");

    return true;
}

} // namespace DaqDB
