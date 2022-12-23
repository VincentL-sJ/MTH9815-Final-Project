#include "riskservice.hpp"

template<typename T>
PV01<T>::PV01(const T& _product, double _pv01, long _quantity) :
	product(_product)
{
	pv01 = _pv01;
	quantity = _quantity;
}

template<typename T>
const T& PV01<T>::GetProduct() const
{
	return product;
}

template<typename T>
double PV01<T>::GetPV01() const
{
	return pv01;
}

template<typename T>
long PV01<T>::GetQuantity() const
{
	return quantity;
}

template<typename T>
void PV01<T>::SetQuantity(long _quantity)
{
	quantity = _quantity;
}

template<typename T>
vector<string> PV01<T>::ToStrings() const
{
	string _product = product.GetProductId();
	string _pv01 = to_string(pv01);
	string _quantity = to_string(quantity);

	vector<string> _strings;
	_strings.push_back(_product);
	_strings.push_back(_pv01);
	_strings.push_back(_quantity);
	return _strings;
}


template<typename T>
BucketedSector<T>::BucketedSector(const vector<T>& _products, string _name) :
	products(_products)
{
	name = _name;
}

template<typename T>
const vector<T>& BucketedSector<T>::GetProducts() const
{
	return products;
}

template<typename T>
const string& BucketedSector<T>::GetName() const
{
	return name;
}


//Risk Service
template<typename T>
RiskService<T>::RiskService()
{
	pv01s = map<string, PV01<T>>();
	listeners = vector<ServiceListener<PV01<T>>*>();
	listener = new RiskToPositionListener<T>(this);
}

template<typename T>
RiskService<T>::~RiskService() {}

template<typename T>
PV01<T>& RiskService<T>::GetData(string _key)
{
	return pv01s[_key];
}

template<typename T>
void RiskService<T>::OnMessage(PV01<T>& _data)
{
	pv01s[_data.GetProduct().GetProductId()] = _data;
}

template<typename T>
void RiskService<T>::AddListener(ServiceListener<PV01<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<PV01<T>>*>& RiskService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
RiskToPositionListener<T>* RiskService<T>::GetListener()
{
	return listener;
}

template<typename T>
void RiskService<T>::AddPosition(Position<T>& _position)
{
	T _product = _position.GetProduct();
	string _productId = _product.GetProductId();
	double _pv01Value = GetPV01Value(_productId);
	long _quantity = _position.GetAggregatePosition();
	PV01<T> _pv01(_product, _pv01Value, _quantity);
	pv01s[_productId] = _pv01;

	for (auto& l : listeners)
	{
		l->ProcessAdd(_pv01);
	}
}

template<typename T>
const PV01<BucketedSector<T>>& RiskService<T>::GetBucketedRisk(const BucketedSector<T>& _sector) const
{
	BucketedSector<T> _product = _sector;
	double _pv01 = 0;
	long _quantity = 1;

	vector<T>& _products = _sector.GetProducts();
	for (auto& p : _products)
	{
		string _pId = p.GetProductId();
		_pv01 += pv01s[_pId].GetPV01() * pv01s[_pId].GetQuantity();
	}

	return PV01<BucketedSector<T>>(_product, _pv01, _quantity);
}


//TODO: Risk to position listener
template<typename T>
RiskToPositionListener<T>::RiskToPositionListener(RiskService<T>* _service)
{
	service = _service;
}

template<typename T>
RiskToPositionListener<T>::~RiskToPositionListener() {}

template<typename T>
void RiskToPositionListener<T>::ProcessAdd(Position<T>& _data)
{
	service->AddPosition(_data);
}

template<typename T>
void RiskToPositionListener<T>::ProcessRemove(Position<T>& _data) {}

template<typename T>
void RiskToPositionListener<T>::ProcessUpdate(Position<T>& _data) {}