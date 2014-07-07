/**************************************************************************
	created		:	2012-10-26
	filename	: 	EndPointController.h
	author		:	Xebeth`
	copyright	:	North Edge (2012)
	purpose		:	
**************************************************************************/
#ifndef __ENDPOINTCONTROLLER_H__
#define __ENDPOINTCONTROLLER_H__

interface IPolicyConfigVista;

typedef struct _EndPointInfo
{
	string_t Name;
	string_t ID;
} EndPointInfo;

typedef stdext::hash_map<int, EndPointInfo> EndPointMap;

class EndPointController
{
public:
	EndPointController();
	~EndPointController();

	bool ToggleEndPoint();
	UINT EnumerateEndPoints();
	bool SetDefaultAudioPlaybackDevice(LPCWSTR devID);
	void GetFriendlyDeviceName(IMMDevice * pDevice_in, string_t &Name_out);

	bool IsEOF() { return m_CurrentEndPoint == m_EndPoints.end(); }
	void AddEndPoint(const string_t &Name, const string_t &ID);
	bool Next(EndPointInfo &Info_out);
	
protected:
	EndPointMap::const_iterator m_CurrentEndPoint;
	IPolicyConfigVista *m_pPolicyConfig;
	EndPointMap m_EndPoints;	
};

#endif//__ENDPOINTCONTROLLER_H__
