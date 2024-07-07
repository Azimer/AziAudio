#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

#include "testmain.h"
#include "../Configuration.h"
#include "../common.h"

#include <Windows.h>
#include "../XAudio2SoundDriver.h"

#ifdef false


class TestConfiguration : Configuration
{
public:
    static void SetTestValues()
    {
        LoadDefaults();
        setSyncAudio(true);
        setForceSync(true);
        setAIEmulation(false);
        setVolume(50);
        setDriver(SND_DRIVER_XA2);
        setBufferLevel(6);
        setBufferFPS(48);
        setBackendFPS(98);
        setDisallowSleepDS8(true);
        setDisallowSleepXA2(false);
    }
};


//--------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------
struct AudioDevice
{
    std::wstring deviceId;
    std::wstring description;
};

HRESULT EnumerateAudio(_In_ IXAudio2* pXaudio2, _Inout_ std::vector<AudioDevice>& list);



int XAudioGetDevices(std::string& failureResult)
{


    return 0;
}

/*
Plan:

  * Loop through all active endpoints to find the endpoint set from the configuration.
  * Loop through all active endpoints to find the endpoint set as the default - Allow Windows to manage it
  * Do not set an endpoint and 'play' audio in silence
    
 */

 //--------------------------------------------------------------------------------------
 // Enumerate audio end-points
 //--------------------------------------------------------------------------------------
HRESULT EnumerateAudio(_In_ IXAudio2* pXaudio2, _Inout_ std::vector<AudioDevice>& list)
{
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)

    UNREFERENCED_PARAMETER(pXaudio2);

#if defined(__cplusplus_winrt )

    // Enumerating with WinRT using C++/CX
    using namespace concurrency;
    using Windows::Devices::Enumeration::DeviceClass;
    using Windows::Devices::Enumeration::DeviceInformation;
    using Windows::Devices::Enumeration::DeviceInformationCollection;

    auto operation = DeviceInformation::FindAllAsync(DeviceClass::AudioRender);

    auto task = create_task(operation);

    task.then([&list](DeviceInformationCollection^ devices)
        {
            for (unsigned i = 0; i < devices->Size; ++i)
            {
                using Windows::Devices::Enumeration::DeviceInformation;

                DeviceInformation^ d = devices->GetAt(i);

                AudioDevice device;
                device.deviceId = d->Id->Data();
                device.description = d->Name->Data();
                list.emplace_back(device);
            }
        });

    task.wait();

    if (list.empty())
        return S_FALSE;

#else

    // Enumerating with WinRT using WRL
    using namespace Microsoft::WRL;
    using namespace Microsoft::WRL::Wrappers;
    using namespace ABI::Windows::Foundation;
    using namespace ABI::Windows::Foundation::Collections;
    using namespace ABI::Windows::Devices::Enumeration;

    RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
    HRESULT hr = initialize;
    if (FAILED(hr))
        return hr;

    Microsoft::WRL::ComPtr<IDeviceInformationStatics> diFactory;
    hr = ABI::Windows::Foundation::GetActivationFactory(HStringReference(RuntimeClass_Windows_Devices_Enumeration_DeviceInformation).Get(), &diFactory);
    if (FAILED(hr))
        return hr;

    Event findCompleted(CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, WRITE_OWNER | EVENT_ALL_ACCESS));
    if (!findCompleted.IsValid())
        return HRESULT_FROM_WIN32(GetLastError());

    auto callback = Callback<IAsyncOperationCompletedHandler<DeviceInformationCollection*>>(
        [&findCompleted, list](IAsyncOperation<DeviceInformationCollection*>* aDevices, AsyncStatus status) -> HRESULT
        {
            SetEvent(findCompleted.Get());
            return S_OK;
        });

    ComPtr<IAsyncOperation<DeviceInformationCollection*>> operation;
    hr = diFactory->FindAllAsyncDeviceClass(DeviceClass_AudioRender, operation.GetAddressOf());
    if (FAILED(hr))
        return hr;

    operation->put_Completed(callback.Get());

    WaitForSingleObject(findCompleted.Get(), INFINITE);

    ComPtr<IVectorView<DeviceInformation*>> devices;
    operation->GetResults(devices.GetAddressOf());

    unsigned int count = 0;
    hr = devices->get_Size(&count);
    if (FAILED(hr))
        return hr;

    if (!count)
        return S_FALSE;

    for (unsigned int j = 0; j < count; ++j)
    {
        ComPtr<IDeviceInformation> deviceInfo;
        hr = devices->GetAt(j, deviceInfo.GetAddressOf());
        if (SUCCEEDED(hr))
        {
            HString id;
            deviceInfo->get_Id(id.GetAddressOf());

            HString name;
            deviceInfo->get_Name(name.GetAddressOf());

            AudioDevice device;
            device.deviceId = id.GetRawBuffer(nullptr);
            device.description = name.GetRawBuffer(nullptr);
            list.emplace_back(device);
        }
    }

    return S_OK;

#endif 

#elif defined(USING_XAUDIO2_REDIST)

    // Enumeration for XAudio 2.9 down-level on Windows 7
    ComPtr<IMMDeviceEnumerator> devEnum;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(devEnum.GetAddressOf()));
    if (FAILED(hr))
        return hr;

    ComPtr<IMMDeviceCollection> devices;
    hr = devEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &devices);
    if (FAILED(hr))
        return hr;

    UINT count = 0;
    hr = devices->GetCount(&count);
    if (FAILED(hr))
        return hr;

    if (!count)
        return S_FALSE;

    for (UINT j = 0; j < count; ++j)
    {
        ComPtr<IMMDevice> endpoint;
        hr = devices->Item(j, endpoint.GetAddressOf());
        if (FAILED(hr))
            return hr;

        LPWSTR id = nullptr;
        hr = endpoint->GetId(&id);
        if (FAILED(hr))
            return hr;

        AudioDevice device;
        device.deviceId = id;
        CoTaskMemFree(id);

        ComPtr<IPropertyStore> props;
        hr = endpoint->OpenPropertyStore(STGM_READ, props.GetAddressOf());
        if (FAILED(hr))
            return hr;

        PROPVARIANT var;
        PropVariantInit(&var);

        hr = props->GetValue(PKEY_Device_FriendlyName, &var);
        if (FAILED(hr) || var.vt != VT_LPWSTR)
            return hr;

        device.description = var.pwszVal;
        PropVariantClear(&var);

        list.emplace_back(device);
    }

#else // USING_XAUDIO2_7_DIRECTX

    // Enumerating with XAudio 2.7
    UINT32 count = 0;
    HRESULT hr = pXaudio2->GetDeviceCount(&count);
    if (FAILED(hr))
        return hr;

    if (!count)
        return S_FALSE;

    list.reserve(count);

    for (UINT32 j = 0; j < count; ++j)
    {
        XAUDIO2_DEVICE_DETAILS details;
        hr = pXaudio2->GetDeviceDetails(j, &details);
        if (SUCCEEDED(hr))
        {
            AudioDevice device;
            device.deviceId = details.DeviceID;
            device.description = details.DisplayName;
            list.emplace_back(device);
        }
    }

#endif

    return S_OK;
}

#endif