#include "SwitchHandler.h"

#include <vector>
#include <app/server/Server.h>
#include <credentials/FabricTable.h>
#include <controller/ReadInteraction.h>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;
using OnOffType = app::Clusters::OnOff::Attributes::OnOff::TypeInfo::Type;

const int CLUSTER_ONOFF = 6;
const int MIN_TIME = 1;
const int MAX_TIME = 10;


void SwitchHandler::Init() 
{
    DeviceLayer::PlatformMgr().ScheduleWork(InitInternal);
}

void SwitchHandler::InitInternal(intptr_t aArg)
{
    ChipLogProgress(AppServer, "Initialize Switch Handler");
    auto & server = Server::GetInstance();

    BindingManager::GetInstance().Init({ 
            &server.GetFabricTable(), 
            server.GetCASESessionManager(), 
            &server.GetPersistentStorage() 
    });
    
    BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(SubscribeDataHandler);
    //BindingManager::GetInstance().RegisterBoundDeviceContextReleaseHandler(SubscribeDataContextReleaseHandler);
    SwitchHandler::GetInstance()->PrintBindingTable();
}

void SwitchHandler::SubscribeDataHandler(const EmberBindingTableEntry & binding, OperationalDeviceProxy * aDevice, void * aContext)
{
    VerifyOrReturn(aContext != nullptr, ChipLogError(AppServer, "Invalid context for Light switch handler"););
    
    std::cout << "SubscribeDataHandler\n";
    SwitchHandler::Data * data = reinterpret_cast<SwitchHandler::Data *>(aContext);

    auto instance = SwitchHandler::GetInstance();
    VerifyOrDie(aDevice);

    auto onSuccess = [instance](const app::ConcreteDataAttributePath & aPath, const OnOffType & aData) {
        ChipLogProgress(AppServer, "++++ ReadAttribute onoff value: %d ", aData);
        // Callback is called 
        instance->Update(aData);
    };

    auto onFailure = [](const app::ConcreteDataAttributePath * aPath, CHIP_ERROR aError) {
        std::cerr << "SubscribeAttribute .... error!" << std::endl;
    };
    
    if (data->ClusterId == CLUSTER_ONOFF) {  
        
        std::cout << "...................... OnOff .......................\n";
        CHIP_ERROR ret = chip::Controller::SubscribeAttribute<OnOffType>(
                aDevice->GetExchangeManager(),
                aDevice->GetSecureSession().Value(), 
                data->EndpointId,   
                data->ClusterId,    
                data->AttributeId, 
                onSuccess, 
                onFailure,
                MIN_TIME, 
                MAX_TIME
                );

        if (CHIP_NO_ERROR != ret) {
            ChipLogProgress(AppServer, "Request ERROR, %s ", ErrorStr(ret));
        }
    }
}

void SwitchHandler::SubscribeDataContextReleaseHandler(void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(AppServer, "Invalid context for Light switch context release handler"););
    std::cout << "Sensors Data Release\n";
    Platform::Delete(static_cast<Data *>(context));
}

void SwitchHandler::PrintBindingTable()
{
    BindingTable & bindingTable = BindingTable::GetInstance();

    ChipLogError(AppServer, "Binding Table size: [%d]:", bindingTable.Size());
    uint8_t i = 0;
    for (auto & entry : bindingTable)
    {
        switch (entry.type)
        {
        case EMBER_UNICAST_BINDING:
            ChipLogError(AppServer, "[%d] UNICAST:", i++);
            ChipLogError(AppServer, "\t\t+ Fabric: %d\n \
            \t+ LocalEndpoint %d \n \
            \t+ ClusterId %d \n \
            \t+ RemoteEndpointId %d \n \
            \t+ NodeId %d",
                    (int) entry.fabricIndex, (int) entry.local, (int) entry.clusterId.Value(), (int) entry.remote,
                    (int) entry.nodeId);
            break;
        case EMBER_MULTICAST_BINDING:
            ChipLogError(AppServer, "[%d] GROUP:", i++);
            ChipLogError(AppServer, "\t\t+ Fabric: %d\n \
            \t+ LocalEndpoint %d \n \
            \t+ RemoteEndpointId %d \n \
            \t+ GroupId %d",
                    (int) entry.fabricIndex, (int) entry.local, (int) entry.remote, (int) entry.groupId);
            break;
        case EMBER_UNUSED_BINDING:
            ChipLogError(AppServer, "[%d] UNUSED", i++);
            break;
        default:
            break;
        }
    }
}

void SwitchHandler::SwitchWorkerHandler(intptr_t aContext)
{
    VerifyOrReturn(aContext != 0, ChipLogError(AppServer, "Invalid sensors data"));
    
    SwitchHandler::Data* data = reinterpret_cast<SwitchHandler::Data*>(aContext);
    std::cout << "Notify ....... cluster ID: " << data->ClusterId << "\n";
    BindingManager::GetInstance().NotifyBoundClusterChanged(data->EndpointId, data->ClusterId, static_cast<void *>(data));
}

bool SwitchHandler::Subscribe() {
    
    using namespace std::chrono_literals;
    std::cout << "Subscribe for attribute change ............ ";
    auto bTable = BindingTable::GetInstance();

    std::vector<std::shared_ptr<SwitchHandler::Data>> data;

    std::transform(bTable.begin(), bTable.end(), std::back_inserter(data),  [](auto & entry) { 
            return std::make_shared<SwitchHandler::Data>(SwitchHandler::Data{
                    .EndpointId = entry.remote,
                    .ClusterId = entry.clusterId.Value(),
                    .AttributeId = 0,
            });
    });
    
    if (data.empty()) {
        std::cout << "No data - return\n";
        return false;
    }
    
    std::cout << "There are some data! :)\n";
    for (auto & d : data) {
        DeviceLayer::PlatformMgr().ScheduleWork(SwitchHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(d.get()));
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return true;
}

void SwitchHandler::Update(bool state) 
{
    if (mDataCallback) {
        mDataCallback(state);
    }
}

void SwitchHandler::SetCallback(std::function<void(bool)> callback) 
{
    mDataCallback = callback;
}


std::ostream& operator<<(std::ostream& os, const SwitchHandler::Data& data)
{
    os << data.EndpointId << '/' << data.ClusterId << '/' << data.AttributeId;
    return os;
}
