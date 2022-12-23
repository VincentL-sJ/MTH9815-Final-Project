#include "historicaldataservice.hpp"


template<typename T>
HistoricalDataService<T>::HistoricalDataService()
{
	historicalDatas = map<string, T>();
	listeners = vector<ServiceListener<T>*>();
	connector = new HistoricalDataConnector<T>(this);
	listener = new HistoricalDataListener<T>(this);
	type = INQUIRY;
}

template<typename T>
HistoricalDataService<T>::HistoricalDataService(ServiceType _type)
{
	historicalDatas = map<string, V>();
	listeners = vector<ServiceListener<V>*>();
	connector = new HistoricalDataConnector<V>(this);
	listener = new HistoricalDataListener<V>(this);
	type = _type;
}

template<typename T>
HistoricalDataService<T>::~HistoricalDataService() {}

template<typename T>
T& HistoricalDataService<T>::GetData(string _key)
{
	return historicalDatas[_key];
}

template<typename T>
void HistoricalDataService<T>::OnMessage(T& _data)
{
	historicalDatas[_data.GetProduct().GetProductId()] = _data;
}

template<typename T>
void HistoricalDataService<T>::AddListener(ServiceListener<T>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<T>*>& HistoricalDataService<T>::GetListeners() const
{
	return listeners;
}









//TODO:: CONNECTOR
template<typename V>
HistoricalDataConnector<V>::HistoricalDataConnector(HistoricalDataService<V>* _service)
{
	service = _service;
}

template<typename V>
HistoricalDataConnector<V>::~HistoricalDataConnector() {}

template<typename V>
void HistoricalDataConnector<V>::Publish(V& _data)
{
	ServiceType _type = service->GetServiceType();
	ofstream _file;
	switch (_type)
	{
	case POSITION:
		_file.open("positions.txt", ios::app);
		break;
	case RISK:
		_file.open("risk.txt", ios::app);
		break;
	case EXECUTION:
		_file.open("executions.txt", ios::app);
		break;
	case STREAMING:
		_file.open("streaming.txt", ios::app);
		break;
	case INQUIRY:
		_file.open("allinquiries.txt", ios::app);
		break;
	}

	_file << TimeStamp() << ",";
	vector<string> _strings = _data.ToStrings();
	for (auto& s : _strings)
	{
		_file << s << ",";
	}
	_file << endl;
}

template<typename V>
void HistoricalDataConnector<V>::Subscribe(ifstream& _data) {}

template<typename V>
HistoricalDataListener<V>::HistoricalDataListener(HistoricalDataService<V>* _service)
{
	service = _service;
}

template<typename V>
HistoricalDataListener<V>::~HistoricalDataListener() {}

template<typename V>
void HistoricalDataListener<V>::ProcessAdd(V& _data)
{
	string _persistKey = _data.GetProduct().GetProductId();
	service->PersistData(_persistKey, _data);
}

template<typename V>
void HistoricalDataListener<V>::ProcessRemove(V& _data) {}

template<typename V>
void HistoricalDataListener<V>::ProcessUpdate(V& _data) {}

;
