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

namespace DaqDB {

const std::string SPDK_APP_ENV_NAME = "DaqDB";
const std::string DEFAULT_SPDK_CONF_FILE = "spdk.conf";

enum class SpdkState : std::uint8_t {
    SPDK_INIT = 0,
    SPDK_READY,
    SPDK_ERROR,
    SPDK_STOPPED
};

using namespace std;
namespace bf = boost::filesystem;

class SpdkCore {
  public:
    SpdkCore(OffloadOptions _offloadOptions);
    ~SpdkCore();

    /**
     * SPDK requires passing configuration through file.
     * This function creates temporary configuration file
     * in format supported by SPDK.
     *
     * @return false when cannot create the file or there are no NVMe devices in
     * _offloadOptions, true otherwise
     */
    bool createConfFile(void);
    void removeConfFile(void);
    bool isOffloadEnabled() {
        if (state == SpdkState::SPDK_READY)
            return spBdev->isOffloadEnabled();
        else
            return false;
    }
    bool spdkEnvInit(void);
    SpdkDevice *getBdev(void) { return spBdev; }
    bool isSpdkReady() {
        return state == SpdkState::SPDK_READY ? true : false;
    }
    bool isBdevFound() {
        return state == SpdkState::SPDK_READY && spBdev->isBdevFound() == true;
    }
    void restoreSignals();

    std::atomic<SpdkState> state;
    SpdkDevice *spBdev;
    OffloadOptions offloadOptions;
    Poller<OffloadRqst> *poller;

    const static char *spdkHugepageDirname;

    void signalReady();
    bool waitReady();
    void setPoller(Poller<OffloadRqst> *pol) { poller = pol; }
    static int spdkCoreMainLoop(SpdkCore *spdkCore);

    /*
     * Callback function called by SPDK spdk_app_start in the context of an SPDK
     * thread.
     */
    static void spdkStart(void *arg);
    void startSpdk();

  private:
    std::thread *_spdkThread;
    std::thread *_loopThread;
    bool _ready;

    size_t _cpuCore;
    SpdkConf _spdkConf;
    std::string _confFile;

    std::mutex _syncMutex;
    std::condition_variable _cv;

    inline bool isNvmeInOptions() {
        return offloadOptions._devs.size() ? true : false;
    }
    inline std::string getName() { return offloadOptions.name; }

    void _spdkThreadMain(void);
};

} // namespace DaqDB
