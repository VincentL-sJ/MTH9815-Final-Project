#include "pricingservice.hpp"


template<typename T>
Price<T>::Price(const T& _product, double _mid, double _bidOfferSpread) :
	product(_product)
{
	mid = _mid;
	bidOfferSpread = _bidOfferSpread;
}

template<typename T>
const T& Price<T>::GetProduct() const
{
	return product;
}

template<typename T>
double Price<T>::GetMid() const
{
	return mid;
}

template<typename T>
double Price<T>::GetBidOfferSpread() const
{
	return bidOfferSpread;
}

template<typename T>
vector<string> Price<T>::ToStrings() const
{
	string _product = product.GetProductId();
	string _mid = ConvertPrice(mid);
	string _bidOfferSpread = ConvertPrice(bidOfferSpread);

	vector<string> _strings;
	_strings.push_back(_product);
	_strings.push_back(_mid);
	_strings.push_back(_bidOfferSpread);
	return _strings;
}


template<typename T>
PricingService<T>::PricingService()
{
	prices = map<string, Price<T>>();
	listeners = vector<ServiceListener<Price<T>>*>();
	connector = new PricingConnector<T>(this);
}

template<typename T>
PricingService<T>::~PricingService() {}

template<typename T>
Price<T>& PricingService<T>::GetData(string _key)
{
	return prices[_key];
}

template<typename T>
void PricingService<T>::OnMessage(Price<T>& _data)
{
	prices[_data.GetProduct().GetProductId()] = _data;

	for (auto& l : listeners)
	{
		l->ProcessAdd(_data);
	}
}

template<typename T>
void PricingService<T>::AddListener(ServiceListener<Price<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<Price<T>>*>& PricingService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
PricingConnector<T>* PricingService<T>::GetConnector()
{
	return connector;
}



//Pricing Connector
template<typename T>
PricingConnector<T>::PricingConnector(PricingService<T>* _service)
{
	service = _service;
}

template<typename T>
PricingConnector<T>::~PricingConnector() {}

template<typename T>
void PricingConnector<T>::Publish(Price<T>& _data) {}

template<typename T>
void PricingConnector<T>::Subscribe(ifstream& _data)
{
	string _line;
	while (getline(_data, _line))
	{
		stringstream _lineStream(_line);
		string _cell;
		vector<string> _cells;
		while (getline(_lineStream, _cell, ','))
		{
			_cells.push_back(_cell);
		}

		string _productId = _cells[0];
		double _bidPrice = ConvertPrice(_cells[1]);
		double _offerPrice = ConvertPrice(_cells[2]);
		double _midPrice = (_bidPrice + _offerPrice) / 2.0;
		double _spread = _offerPrice - _bidPrice;
		T _product = GetBond(_productId);
		Price<T> _price(_product, _midPrice, _spread);
		service->OnMessage(_price);
	}
}