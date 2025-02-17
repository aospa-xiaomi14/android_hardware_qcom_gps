/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Not a Contribution
 */
/*
 * Copyright (C) 2016 The Android Open Source Project
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
/*
Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#define LOG_TAG "LocSvc_AGnssInterface"

#include <log_util.h>
#include "Gnss.h"
#include "AGnss.h"

namespace android {
namespace hardware {
namespace gnss {
namespace V2_1 {
namespace implementation {

static AGnss* spAGnss = nullptr;

AGnss::AGnss(Gnss* gnss) : mGnss(gnss), mType(LOC_AGPS_TYPE_INVALID) {
    spAGnss = this;

    LocationControlCallbacks locCtrlCbs;
    memset(&locCtrlCbs, 0, sizeof(locCtrlCbs));
    locCtrlCbs.size = sizeof(LocationControlCallbacks);

    locCtrlCbs.agpsStatusIpV4Cb = [this](AGnssExtStatusIpV4 status) {
            statusCb(status.type, status.status);
    };

    mGnss->getLocationControlApi()->updateCallbacks(locCtrlCbs);
}

AGnss::~AGnss() {
    spAGnss = nullptr;
}

void AGnss::statusCb(AGpsExtType type, AGpsStatusValue status) {

    V2_0::IAGnssCallback::AGnssType  aType;
    IAGnssCallback::AGnssStatusValue aStatus;

    // cache the AGps Type
    if (type > LOC_AGPS_TYPE_INVALID && type <= LOC_AGPS_TYPE_SUPL_ES) {
        mMutex.lock();
        mType = type;
        mMutex.unlock();
    }

    switch (type) {
    case AGPS_TYPE_SUPL:
        aType = IAGnssCallback::AGnssType::SUPL;
        break;
    case AGPS_TYPE_SUPL_ES:
        aType = IAGnssCallback::AGnssType::SUPL_EIMS;
        break;
    default:
        LOC_LOGE("invalid type: %d", type);
        return;
    }

    switch (status) {
    case AGPS_REQUEST_AGPS_DATA_CONN:
        aStatus = IAGnssCallback::AGnssStatusValue::REQUEST_AGNSS_DATA_CONN;
        break;
    case AGPS_RELEASE_AGPS_DATA_CONN:
        aStatus = IAGnssCallback::AGnssStatusValue::RELEASE_AGNSS_DATA_CONN;
        break;
    case AGPS_DATA_CONNECTED:
        aStatus = IAGnssCallback::AGnssStatusValue::AGNSS_DATA_CONNECTED;
        break;
    case AGPS_DATA_CONN_DONE:
        aStatus = IAGnssCallback::AGnssStatusValue::AGNSS_DATA_CONN_DONE;
        break;
    case AGPS_DATA_CONN_FAILED:
        aStatus = IAGnssCallback::AGnssStatusValue::AGNSS_DATA_CONN_FAILED;
        break;
    default:
        LOC_LOGE("invalid status: %d", status);
        return;
    }

    mMutex.lock();
    auto aGnssCbIface = mAGnssCbIface;
    mMutex.unlock();
    if (aGnssCbIface != nullptr) {
        auto r = aGnssCbIface->agnssStatusCb(aType, aStatus);
        if (!r.isOk()) {
            LOC_LOGw("Error invoking AGNSS status cb %s", r.description().c_str());
        }
    }
    else {
        LOC_LOGw("setCallback has not been called yet");
    }
}

Return<void> AGnss::setCallback(const sp<V2_0::IAGnssCallback>& callback) {

    if(mGnss == nullptr || mGnss->getLocationControlApi() == nullptr){
        LOC_LOGE("Null GNSS interface or Control interface");
        return Void();
    }

    // Save the interface
    mMutex.lock();
    mAGnssCbIface = callback;
    mMutex.unlock();

    return Void();
}

Return<bool> AGnss::dataConnClosed() {

    if(mGnss == nullptr || mGnss->getLocationControlApi() == nullptr){
        LOC_LOGE("Null GNSS interface");
        return false;
    }

    mGnss->getLocationControlApi()->agpsDataConnClosed(AGPS_TYPE_SUPL);
    return true;
}

Return<bool> AGnss::dataConnFailed() {

    if(mGnss == nullptr || mGnss->getLocationControlApi() == nullptr){
        LOC_LOGE("Null GNSS interface");
        return false;
    }

    mGnss->getLocationControlApi()->agpsDataConnFailed(AGPS_TYPE_SUPL);
    return true;
}

Return<bool> AGnss::dataConnOpen(uint64_t /*networkHandle*/, const hidl_string& apn,
        V2_0::IAGnss::ApnIpType apnIpType) {

    if(mGnss == nullptr || mGnss->getLocationControlApi() == nullptr){
        LOC_LOGE("Null GNSS interface");
        return false;
    }

    std::string apnString(apn.c_str());
    mMutex.lock();
    auto agpsType = mType;
    mMutex.unlock();
    // During Emergency SUPL, an apn name of "sos" means that no
    // apn was found, like in the simless case, so apn is cleared
    if (LOC_AGPS_TYPE_SUPL_ES == agpsType && "sos" == apnString) {
        LOC_LOGD("dataConnOpen APN name = [sos] cleared");
        apnString.clear();
    }

    LOC_LOGD("dataConnOpen APN name = [%s]", apnString.c_str());

    AGpsBearerType bearerType;
    switch (apnIpType) {
    case IAGnss::ApnIpType::IPV4:
        bearerType = AGPS_APN_BEARER_IPV4;
        break;
    case IAGnss::ApnIpType::IPV6:
        bearerType = AGPS_APN_BEARER_IPV6;
        break;
    case IAGnss::ApnIpType::IPV4V6:
        bearerType = AGPS_APN_BEARER_IPV4V6;
        break;
    default:
        bearerType = AGPS_APN_BEARER_IPV4;
        break;
    }

    mGnss->getLocationControlApi()->agpsDataConnOpen(
       AGPS_TYPE_SUPL, apn.c_str(), apn.size(), (int)bearerType);
    return true;
}

Return<bool> AGnss::setServer(V2_0::IAGnssCallback::AGnssType type,
                              const hidl_string& hostname,
                              int32_t port) {
    if (mGnss == nullptr) {
        LOC_LOGE("%s]: mGnss is nullptr", __FUNCTION__);
        return false;
    }

    GnssConfig config;
    memset(&config, 0, sizeof(GnssConfig));
    config.size = sizeof(GnssConfig);
    config.flags = GNSS_CONFIG_FLAGS_SET_ASSISTANCE_DATA_VALID_BIT;
    config.assistanceServer.size = sizeof(GnssConfigSetAssistanceServer);
    if (type == IAGnssCallback::AGnssType::SUPL) {
        config.assistanceServer.type = GNSS_ASSISTANCE_TYPE_SUPL;
    } else if (type == IAGnssCallback::AGnssType::C2K) {
        config.assistanceServer.type = GNSS_ASSISTANCE_TYPE_C2K;
    } else if (type == IAGnssCallback::AGnssType::SUPL_EIMS) {
        config.assistanceServer.type = GNSS_ASSISTANCE_TYPE_SUPL_EIMS;
    } else if (type == IAGnssCallback::AGnssType::SUPL_IMS) {
        config.assistanceServer.type = GNSS_ASSISTANCE_TYPE_SUPL_IMS;
    } else {
        LOC_LOGE("%s]: invalid AGnssType: %d", __FUNCTION__, static_cast<uint8_t>(type));
        return false;
    }
    config.assistanceServer.hostName = strdup(hostname.c_str());
    config.assistanceServer.port = port;
    return mGnss->updateConfiguration(config);
}

}  // namespace implementation
}  // namespace V2_1
}  // namespace gnss
}  // namespace hardware
}  // namespace android
