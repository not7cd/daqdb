/**
 * Copyright 2018 - 2019 Intel Corporation.
 *
 * This software and the related documents are Intel copyrighted materials,
 * and your use of them is governed by the express license under which they
 * were provided to you (Intel OBL Internal Use License).
 * Unless the License provides otherwise, you may not use, modify, copy,
 * publish, distribute, disclose or transmit this software or the related
 * documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no
 * express or implied warranties, other than those that are expressly
 * stated in the License.
 */

#include <config/Configuration.h>
#include <libconfig.h++>

using namespace libconfig;

namespace DaqDB {

bool readConfiguration(const std::string &configFile, DaqDB::Options &options) {
    std::stringstream ss;
    return readConfiguration(configFile, options, ss);
}

bool readConfiguration(const std::string &configFile, DaqDB::Options &options,
                       std::stringstream &ss) {
    Config cfg;
    try {
        cfg.readFile(configFile.c_str());
    } catch (const libconfig::FileIOException &fioex) {
        ss << "I/O error while reading file.\n";
        return false;
    } catch (const libconfig::ParseException &pex) {
        ss << "Parse error at " << pex.getFile() << ":" << pex.getLine()
           << " - " << pex.getError();
        return false;
    }

    cfg.lookupValue("pmem_path", options.pmem.poolPath);
    long long pmemSize;
    if (cfg.lookupValue("pmem_size", pmemSize))
        options.pmem.totalSize = pmemSize;
    int allocUnitSize;
    if (cfg.lookupValue("alloc_unit_size", allocUnitSize))
        options.pmem.allocUnitSize = allocUnitSize;

    // Configure key structure
    std::string primaryKey;
    cfg.lookupValue("primaryKey", primaryKey);
    std::vector<int> keysStructure;
    try {
        const libconfig::Setting &keys_settings = cfg.lookup("keys_structure");
        for (int n = 0; n < keys_settings.getLength(); ++n) {
            keysStructure.push_back(keys_settings[n]);
            // TODO extend functionality of primary key definition
            options.key.field(n, keysStructure[n], (n == 0) ? true : false);
        }
    } catch (SettingNotFoundException &e) {
        // no action needed
    }

    std::string db_mode;
    cfg.lookupValue("mode", db_mode);
    if (db_mode.compare("satellite") == 0) {
        options.mode = OperationalMode::SATELLITE;
    } else {
        // STORAGE as default mode
        options.mode = OperationalMode::STORAGE;
    }

    int offloadAllocUnitSize;
    if (cfg.lookupValue("offload_unit_alloc_size", offloadAllocUnitSize))
        options.offload.allocUnitSize = offloadAllocUnitSize;
    cfg.lookupValue("offload_nvme_addr", options.offload.nvmeAddr);
    cfg.lookupValue("offload_nvme_name", options.offload.nvmeName);
    int maskLength = 0;
    int maskOffset = 0;
    cfg.lookupValue("dht_key_mask_length", maskLength);
    cfg.lookupValue("dht_key_mask_offset", maskOffset);

    try {
        const libconfig::Setting &neighbors = cfg.lookup("neighbors");
        for (int n = 0; n < neighbors.getLength(); ++n) {
            auto dhtNeighbor = new DaqDB::DhtNeighbor;
            const libconfig::Setting &neighbor = neighbors[n];
            dhtNeighbor->ip = neighbor["ip"].c_str();
            dhtNeighbor->port = (unsigned int)(neighbor["port"]);
            dhtNeighbor->keyRange.maskLength = maskLength;
            dhtNeighbor->keyRange.maskOffset = maskOffset;
            try {
                dhtNeighbor->keyRange.start = neighbor["keys"]["start"].c_str();
                dhtNeighbor->keyRange.end = neighbor["keys"]["end"].c_str();
            } catch (SettingNotFoundException &e) {
                dhtNeighbor->keyRange.start = "";
                dhtNeighbor->keyRange.end = "";
            }
            try {
                dhtNeighbor->peerPort = (unsigned int)(neighbor["peerPort"]);
            } catch (SettingNotFoundException &e) {
                dhtNeighbor->peerPort = 0;
            }
            try {
                dhtNeighbor->local = ((unsigned int)(neighbor["local"]) > 0);
            } catch (SettingNotFoundException &e) {
                dhtNeighbor->local = false;
            }
            options.dht.neighbors.push_back(dhtNeighbor);
        }
    } catch (SettingNotFoundException &e) {
        // no action needed
    }
    return true;
}

} // namespace DaqDB
