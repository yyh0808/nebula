/* Copyright (c) 2019 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include "meta/processors/configMan/RegConfigProcessor.h"

namespace nebula {
namespace meta {

void RegConfigProcessor::process(const cpp2::RegConfigReq& req) {
    std::vector<kvstore::KV> data;
    {
        folly::SharedMutex::WriteHolder wHolder(LockUtils::configLock());
        for (const auto& item : req.get_items()) {
            auto module = item.get_module();
            auto name = item.get_name();
            auto mode = item.get_mode();
            auto value = item.get_value();
            VLOG(1) << "Config name: " << name
                    << ", mode: " << meta::cpp2::_ConfigMode_VALUES_TO_NAMES.at(mode)
                    << ", module: " << meta::cpp2::_ConfigModule_VALUES_TO_NAMES.at(module)
                    << ", value: " << value;

            std::string configKey = MetaServiceUtils::configKey(module, name);
            // ignore config which has been registered before
            if (doGet(configKey).ok()) {
                continue;
            }
            std::string configValue = MetaServiceUtils::configValue(mode, value);
            data.emplace_back(std::move(configKey), std::move(configValue));
        }

        if (!data.empty()) {
            doSyncPutAndUpdate(std::move(data));
            return;
        }
    }
    handleErrorCode(cpp2::ErrorCode::SUCCEEDED);
    onFinished();
}

}  // namespace meta
}  // namespace nebula
