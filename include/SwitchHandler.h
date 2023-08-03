#pragma once

#include <app/CASESessionManager.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <memory>
#include <functional>
#include <ostream>

using namespace chip;

class SwitchHandler
{
public:

    SwitchHandler(SwitchHandler const &) = delete;
    void operator=(SwitchHandler const &)  = delete;

    static std::shared_ptr<SwitchHandler> GetInstance() { 
        static std::shared_ptr<SwitchHandler> sSwitchHandler{new SwitchHandler};
        return sSwitchHandler; 
    }

    struct Data
    {
        chip::EndpointId EndpointId;
        chip::ClusterId ClusterId;
        chip::AttributeId AttributeId;
    };
    
    friend std::ostream& operator<<(std::ostream& os, const Data& data);

    void SetCallback(std::function<void(bool)> callback);

    /**
     * method is called when connection is established successfully and data is going to send to WebSeocket server.
     * Update WebSocket server state.
     */ 
    void Init();
    void Update(bool state);
    void PrintBindingTable();
    bool Subscribe();

    void InvokeToggleCommand(bool on);
    
    static void SwitchWorkerHandler(intptr_t);
 
    void SetPeerDevice(OperationalDeviceProxy* aDevice) {
        //OperationalDeviceProxy device(aDevice->GetExchangeManager(), aDevice->GetSecureSession().Value());
        mPeerDevice = *aDevice;
    }
   

private:
    SwitchHandler() {}
    
    static BindingManagerInitParams mInitParams;

    static void SubscribeDataHandler(const EmberBindingTableEntry &, chip::OperationalDeviceProxy *, void *);
    static void InvokeCommandHandler(const EmberBindingTableEntry & binding, OperationalDeviceProxy * aDevice, void * aContext);

    static void SubscribeDataContextReleaseHandler(void * context);

    static void InitInternal(intptr_t);
    std::function<void (bool)> mDataCallback;

    OperationalDeviceProxy mPeerDevice;
};

std::ostream& operator<<(std::ostream& os, const SwitchHandler::Data& data);




