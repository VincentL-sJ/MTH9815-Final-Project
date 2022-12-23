#include "positionservice.hpp"

template<typename T>
Position<T>::Position(const T& _product) :
	product(_product) {}

template<typename T>
const T& Position<T>::GetProduct() const
{
	return product;
}

template<typename T>
long Position<T>::GetPosition(string& _book)
{
	return positions[_book];
}



template<typename T>
long Position<T>::GetAggregatePosition()
{
	long aggregatePosition = 0;
	for (auto& p : positions)
	{
		aggregatePosition += p.second;
	}
	return aggregatePosition;
}

template<typename T>
vector<string> Position<T>::ToStrings() const
{
	string _product = product.GetProductId();
	vector<string> _positions;
	for (auto& p : positions)
	{
		string _book = p.first;
		string _position = to_string(p.second);
		_positions.push_back(_book);
		_positions.push_back(_position);
	}

	vector<string> _strings;
	_strings.push_back(_product);
	_strings.insert(_strings.end(), _positions.begin(), _positions.end());
	return _strings;
}


template<typename T>
PositionService<T>::PositionService()
{
	positions = map<string, Position<T>>();
	listeners = vector<ServiceListener<Position<T>>*>();
	listener = new PositionToTradeBookingListener<T>(this);
}

template<typename T>
PositionService<T>::~PositionService() {}

template<typename T>
Position<T>& PositionService<T>::GetData(string _key)
{
	return positions[_key];
}

template<typename T>
void PositionService<T>::OnMessage(Position<T>& _data)
{
	positions[_data.GetProduct().GetProductId()] = _data;
}

template<typename T>
void PositionService<T>::AddListener(ServiceListener<Position<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
PositionToTradeBookingListener<T>* PositionService<T>::GetListener()
{
	return listener;
}

template<typename T>
const vector<ServiceListener<Position<T>>*>& PositionService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
void PositionService<T>::AddTrade(const Trade<T>& _trade)
{
	T _product = _trade.GetProduct();
	string _productId = _product.GetProductId();
	double _price = _trade.GetPrice();
	string _book = _trade.GetBook();
	long _quantity = _trade.GetQuantity();
	Side _side = _trade.GetSide();
	Position<T> _positionTo(_product);
	if (_side == BUY)
	{
		_positionTo.AddPosition(_book, _quantity);
		break;
	}else {
		_positionTo.AddPosition(_book, -_quantity);
		break;
	}

	Position<T> _positionFrom = positions[_productId];
	map <string, long> _positionMap = _positionFrom.GetPositions();
	for (auto& p : _positionMap)
	{
		_book = p.first;
		_quantity = p.second;
		_positionTo.AddPosition(_book, _quantity);
	}
	positions[_productId] = _positionTo;

	for (auto& l : listeners)
	{
		l->ProcessAdd(_positionTo);
	}
}


template<typename T>
PositionToTradeBookingListener<T>::PositionToTradeBookingListener(PositionService<T>* _service)
{
	service = _service;
}

template<typename T>
PositionToTradeBookingListener<T>::~PositionToTradeBookingListener() {}

template<typename T>
void PositionToTradeBookingListener<T>::ProcessAdd(Trade<T>& _data)
{
	service->AddTrade(_data);
}

template<typename T>
void PositionToTradeBookingListener<T>::ProcessRemove(Trade<T>& _data) {}

template<typename T>
void PositionToTradeBookingListener<T>::ProcessUpdate(Trade<T>& _data) {}
