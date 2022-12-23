#include "tradebookingservice.hpp"

template<typename T>
Trade<T>::Trade(const T& _product, string _tradeId, double _price, string _book, long _quantity, Side _side) :
	product(_product)
{
	tradeId = _tradeId;
	price = _price;
	book = _book;
	quantity = _quantity;
	side = _side;
}

template<typename T>
const T& Trade<T>::GetProduct() const
{
	return product;
}

template<typename T>
const string& Trade<T>::GetTradeId() const
{
	return tradeId;
}

template<typename T>
double Trade<T>::GetPrice() const
{
	return price;
}

template<typename T>
const string& Trade<T>::GetBook() const
{
	return book;
}

template<typename T>
long Trade<T>::GetQuantity() const
{
	return quantity;
}

template<typename T>
Side Trade<T>::GetSide() const
{
	return side;
}


template<typename T>
TradeBookingService<T>::TradeBookingService()
{
	trades = map<string, Trade<T>>();
	listeners = vector<ServiceListener<Trade<T>>*>();
	connector = new TradeBookingConnector<T>(this);
	listener = new TradeBookingToExecutionListener<T>(this);
}

template<typename T>
TradeBookingService<T>::~TradeBookingService() {}

template<typename T>
Trade<T>& TradeBookingService<T>::GetData(string _key)
{
	return trades[_key];
}

template<typename T>
void TradeBookingService<T>::OnMessage(Trade<T>& _data)
{
	trades[_data.GetTradeId()] = _data;

	for (auto& l : listeners)
	{
		l->ProcessAdd(_data);
	}
}

template<typename T>
void TradeBookingService<T>::AddListener(ServiceListener<Trade<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<Trade<T>>*>& TradeBookingService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
TradeBookingConnector<T>* TradeBookingService<T>::GetConnector()
{
	return connector;
}

template<typename T>
TradeBookingToExecutionListener<T>* TradeBookingService<T>::GetListener()
{
	return listener;
}

template<typename T>
void TradeBookingService<T>::BookTrade(Trade<T>& _trade)
{
	for (auto& l : listeners)
	{
		l->ProcessAdd(_trade);
	}
}


template<typename T>
TradeBookingConnector<T>::TradeBookingConnector(TradeBookingService<T>* _service)
{
	service = _service;
}

template<typename T>
TradeBookingConnector<T>::~TradeBookingConnector() {}

template<typename T>
void TradeBookingConnector<T>::Publish(Trade<T>& _data) {}

template<typename T>
void TradeBookingConnector<T>::Subscribe(ifstream& _data)
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
		string _tradeId = _cells[1];
		double _price = ConvertPrice(_cells[2]);
		string _book = _cells[3];
		long _quantity = stol(_cells[4]);
		Side _side;
		if (_cells[5] == "BUY") _side = BUY;
		else if (_cells[5] == "SELL") _side = SELL;
		T _product = GetBond(_productId);
		Trade<T> _trade(_product, _tradeId, _price, _book, _quantity, _side);
		service->OnMessage(_trade);
	}
}


template<typename T>
TradeBookingToExecutionListener<T>::TradeBookingToExecutionListener(TradeBookingService<T>* _service)
{
	service = _service;
	count = 0;
}

template<typename T>
TradeBookingToExecutionListener<T>::~TradeBookingToExecutionListener() {}

template<typename T>
void TradeBookingToExecutionListener<T>::ProcessAdd(ExecutionOrder<T>& _data)
{
	count++;
	T _product = _data.GetProduct();
	PricingSide _pricingSide = _data.GetPricingSide();
	string _orderId = _data.GetOrderId();
	double _price = _data.GetPrice();
	long _visibleQuantity = _data.GetVisibleQuantity();
	long _hiddenQuantity = _data.GetHiddenQuantity();

	Side _side;
	if (_pricingSide == BID)
	{
		_side = SELL;
		break;
	}
	else {
		_side = BUY;
		break;
	}
	string _book;
	switch (count % 3)
	{
	case 0:
		_book = "TRSY1";
		break;
	case 1:
		_book = "TRSY2";
		break;
	case 2:
		_book = "TRSY3";
		break;
	}
	long _quantity = _visibleQuantity + _hiddenQuantity;

	Trade<T> _trade(_product, _orderId, _price, _book, _quantity, _side);
	service->OnMessage(_trade);
	service->BookTrade(_trade);
}

template<typename T>
void TradeBookingToExecutionListener<T>::ProcessRemove(ExecutionOrder<T>& _data) {}

template<typename T>
void TradeBookingToExecutionListener<T>::ProcessUpdate(ExecutionOrder<T>& _data) {}
