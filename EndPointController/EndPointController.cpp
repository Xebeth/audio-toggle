// EndPointController.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include "EndPointController.h"
#include "Functiondiscoverykeys_devpkey.h"

EndPointController::EndPointController() : m_pPolicyConfig(NULL)
{
	// initialize COM
	::CoInitialize(NULL);

	HRESULT hr = ::CoCreateInstance(__uuidof(CPolicyConfigVistaClient), NULL, CLSCTX_ALL, 
									__uuidof(IPolicyConfigVista), (LPVOID*)&m_pPolicyConfig);

	m_CurrentEndPoint = m_EndPoints.end();

	if (FAILED(hr))
		throw std::exception("Failed to instantiate the audio endpoint configuration policy.");
}

EndPointController::~EndPointController()
{
	if (m_pPolicyConfig != NULL)
	{
		m_pPolicyConfig->Release();
		m_pPolicyConfig = NULL;
	}

	// shutdown COM
	::CoUninitialize();
}

bool EndPointController::SetDefaultAudioPlaybackDevice(LPCWSTR devID)
{	
	ERole reserved = eConsole;
	HRESULT hr = 0xFFFFFFFF;

	if (m_pPolicyConfig != NULL)
		hr = m_pPolicyConfig->SetDefaultEndpoint(devID, reserved);

	return SUCCEEDED(hr);
}

bool EndPointController::ToggleEndPoint()
{
	if (m_EndPoints.size() > 1U)
	{
		IMMDeviceEnumerator *pEnum = NULL;
		IMMDevice *pDefaultDevice = NULL;
		UINT Result = 0U;

		// Create a multimedia device enumerator.
		HRESULT hr = ::CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
										__uuidof(IMMDeviceEnumerator), (void**)&pEnum);

		if (SUCCEEDED(hr))
		{
			pEnum->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDefaultDevice);

			if (pDefaultDevice != NULL)
			{
				LPWSTR wstrID = NULL;

				pDefaultDevice->GetId(&wstrID);
				pDefaultDevice->Release();

				EndPointMap::const_iterator DeviceIt = m_EndPoints.begin();
				EndPointMap::const_iterator EndIt = m_EndPoints.end();

				for (; DeviceIt != EndIt; ++DeviceIt)
				{
					if (_tcscmp(wstrID, DeviceIt->second.ID.c_str()) == 0)
						break;
				}

				++DeviceIt;

				if (DeviceIt == m_EndPoints.end())
					DeviceIt = m_EndPoints.begin();
					
				return SetDefaultAudioPlaybackDevice(DeviceIt->second.ID.c_str());
			}
		}
	}


	return false;
}

UINT EndPointController::EnumerateEndPoints()
{
	IMMDeviceEnumerator *pEnum = NULL;
	UINT Result = 0U;

	// Create a multimedia device enumerator.
	HRESULT hr = ::CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
									__uuidof(IMMDeviceEnumerator), (void**)&pEnum);
	if (SUCCEEDED(hr))
	{
		IMMDeviceCollection *pDevices;
		// Enumerate the output devices.
		hr = pEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevices);

		if (SUCCEEDED(hr))
		{
			EndPointInfo Info;			

			pDevices->GetCount(&Result);

			if (SUCCEEDED(hr))
			{
				IMMDevice *pDefaultDevice = NULL, *pDevice = NULL;
				LPWSTR wstrID = NULL;
				string_t DefName;
				
				pEnum->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDefaultDevice);
				GetFriendlyDeviceName(pDefaultDevice, Info.Name);
				pDefaultDevice->GetId(&wstrID);
				DefName = Info.ID = wstrID;
				pDefaultDevice->Release();

				m_EndPoints[0] = Info;

				for (UINT i = 0U; i < Result; ++i)
				{
					hr = pDevices->Item(i, &pDevice);

					Info.Name.clear();
					Info.ID.clear();

					if (SUCCEEDED(hr) && pDevice != pDefaultDevice)
					{
						hr = pDevice->GetId(&wstrID);
						
						if (_tcscmp(wstrID, DefName.c_str()) != 0)
						{
							GetFriendlyDeviceName(pDevice, Info.Name);
							Info.ID = wstrID;

							m_EndPoints[i+1] = Info;

							pDevice->Release();
						}
					}
				}
			}

			m_CurrentEndPoint = m_EndPoints.begin();
			pDevices->Release();
		}

		pEnum->Release();
	}

	return Result;
}

void EndPointController::GetFriendlyDeviceName(IMMDevice * pDevice_in, string_t &Name_out)
{
	if (pDevice_in != NULL)
	{
		IPropertyStore *pStore;

		HRESULT hr = pDevice_in->OpenPropertyStore(STGM_READ, &pStore);

		if (SUCCEEDED(hr))
		{
			PROPVARIANT friendlyName;
			PropVariantInit(&friendlyName);

			hr = pStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);

			if (SUCCEEDED(hr))
			{
				Name_out = friendlyName.pwszVal;
				PropVariantClear(&friendlyName);
			}

			pStore->Release();
		}
	}
}

bool EndPointController::Next(EndPointInfo &Info_out)
{
	if (IsEOF() == false)
	{
		Info_out = m_CurrentEndPoint->second;
		++m_CurrentEndPoint;

		return true;
	}

	return false;
}

void EndPointController::AddEndPoint(const string_t &Name, const string_t &ID)
{
	EndPointInfo Info;

	Info.Name = Name;
	Info.ID = ID;

	m_EndPoints.insert(std::make_pair( m_EndPoints.size(), Info));

	if (m_CurrentEndPoint == m_EndPoints.end())
		m_CurrentEndPoint = m_EndPoints.begin();
}