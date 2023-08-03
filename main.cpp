/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <AppMain.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>

#include "SwitchHandler.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace std::chrono_literals;

namespace {
DeviceLayer::NetworkCommissioning::LinuxWiFiDriver sLinuxWiFiDriver;
Clusters::NetworkCommissioning::Instance sWiFiNetworkCommissioningInstance(
    0, &sLinuxWiFiDriver);
}  // namespace

void MatterPostAttributeChangeCallback(
    const chip::app::ConcreteAttributePath& attributePath, uint8_t type,
    uint16_t size, uint8_t* value) {
    /*
     *if (attributePath.mClusterId == OnOff::Id && attributePath.mAttributeId ==
     *OnOff::Attributes::OnOff::Id)
     *{
     *    LightingMgr().InitiateAction(*value ? LightingManager::ON_ACTION :
     *LightingManager::OFF_ACTION);
     *}
     */
}

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 * TODO Issue #3841
 * emberAfOnOffClusterInitCallback happens before the stack initialize the
 * cluster attributes to the default value. The logic here expects something
 * similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint) {
    // TODO: implement any additional Cluster Server init actions
}

void ApplicationInit() {
    sWiFiNetworkCommissioningInstance.Init();

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    ChipLogError(NotSpecified, "......... Thread is ENABLED");
#elif
    ChipLogError(NotSpecified, "......... Thread s NOT enabled!!!");
#endif
}

struct DataReader {
    // init TODO Mqtt client

    void Output(bool state) {
        std::cout << "Got switch state !\n";
        // TODO - create mqtt publish topic with upated state.
        std::cout << "State:  " << (state ? "On" : "Off") << "\n";
    }

    void Run() {
        auto instance = SwitchHandler::GetInstance();
        instance->Init();
        instance->SetCallback(
            std::bind(&DataReader::Output, this, std::placeholders::_1));

        bool isOnSubscription = false;
        std::this_thread::sleep_for(10s);
        while (true) {
            std::this_thread::sleep_for(5s);
            //
            if (!isOnSubscription) {
                if (instance->Subscribe()) {
                    isOnSubscription = true;
                }
            } else {
                instance->InvokeToggleCommand(false);
            }
        }  // while
    }
};

int main(int argc, char* argv[]) {
    if (ChipLinuxAppInit(argc, argv) != 0) {
        return -1;
    }

    DataReader reader;
    std::thread read_thread(&DataReader::Run, reader);

    ChipLinuxAppMainLoop();

    read_thread.join();
    return 0;
}
