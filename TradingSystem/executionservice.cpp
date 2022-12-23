#include "executionservice.h"

/**
 * Service for executing orders on an exchange.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class ExecutionService : public Service<string, ExecutionOrder <T> >
{

public:

	// Execute an order on a market
	void ExecuteOrder(const ExecutionOrder<T>& order, Market market) = 0;

};

template<typename T>
ExecutionOrder<T>::ExecutionOrder(const T& _product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder) :
	product(_product)
{
	side = _side;
	orderId = _orderId;
	orderType = _orderType;
	price = _price;
	visibleQuantity = _visibleQuantity;
	hiddenQuantity = _hiddenQuantity;
	parentOrderId = _parentOrderId;
	isChildOrder = _isChildOrder;
}

template<typename T>
const T& ExecutionOrder<T>::GetProduct() const
{
	return product;
}

template<typename T>
const string& ExecutionOrder<T>::GetOrderId() const
{
	return orderId;
}

template<typename T>
OrderType ExecutionOrder<T>::GetOrderType() const
{
	return orderType;
}

template<typename T>
double ExecutionOrder<T>::GetPrice() const
{
	return price;
}

template<typename T>
long ExecutionOrder<T>::GetVisibleQuantity() const
{
	return visibleQuantity;
}

template<typename T>
long ExecutionOrder<T>::GetHiddenQuantity() const
{
	return hiddenQuantity;
}

template<typename T>
const string& ExecutionOrder<T>::GetParentOrderId() const
{
	return parentOrderId;
}

template<typename T>
bool ExecutionOrder<T>::IsChildOrder() const
{
	return isChildOrder;
}

template<typename T>
vector<string> ExecutionOrder<T>::ToStrings() const
{
	string _product = product.GetProductId();
	string _side;
	if (side == BID)
	{
		_side = "BID";
		break;
	}
	else {
		_side = "OFFER";
		break;
	}
	string _orderId = orderId;
	string _orderType;
	switch (orderType)
	{
	case FOK:
		_orderType = "FOK";
		break;
	case IOC:
		_orderType = "IOC";
		break;
	case MARKET:
		_orderType = "MARKET";
		break;
	case LIMIT:
		_orderType = "LIMIT";
		break;
	case STOP:
		_orderType = "STOP";
		break;
	}
	string _price = ConvertPrice(price);
	string _visibleQuantity = to_string(visibleQuantity);
	string _hiddenQuantity = to_string(hiddenQuantity);
	string _parentOrderId = parentOrderId;
	string _isChildOrder;
	if (isChildOrder == true)
	{
		_isChildOrder = "YES";
		break;
	}
	else {
		_isChildOrder = "NO";
		break;
	}

	vector<string> _strings;
	_strings.push_back(_product);
	_strings.push_back(_side);
	_strings.push_back(_orderId);
	_strings.push_back(_orderType);
	_strings.push_back(_price);
	_strings.push_back(_visibleQuantity);
	_strings.push_back(_hiddenQuantity);
	_strings.push_back(_parentOrderId);
	_strings.push_back(_isChildOrder);
	return _strings;
}

template<typename T>
AlgoExecution<T>::AlgoExecution(const T& _product, PricingSide _side, string _orderId, OrderType _orderType, double _price, long _visibleQuantity, long _hiddenQuantity, string _parentOrderId, bool _isChildOrder)
{
	executionOrder = new ExecutionOrder<T>(_product, _side, _orderId, _orderType, _price, _visibleQuantity, _hiddenQuantity, _parentOrderId, _isChildOrder);
}

template<typename T>
ExecutionOrder<T>* AlgoExecution<T>::GetExecutionOrder() const
{
	return executionOrder;
}

template<typename T>
AlgoExecutionService<T>::AlgoExecutionService()
{
	algoExecutions = map<string, AlgoExecution<T>>();
	listeners = vector<ServiceListener<AlgoExecution<T>>*>();
	listener = new AlgoExecutionToMarketDataListener<T>(this);
	spread = 1.0 / 128.0;
	count = 0;
}

template<typename T>
AlgoExecutionService<T>::~AlgoExecutionService() {};

template<typename T>
AlgoExecution<T>& AlgoExecutionService<T>::GetData(string _key)
{
	return algoExecutions[_key];
}

template<typename T>
void AlgoExecutionService<T>::OnMessage(AlgoExecution<T>& _data)
{
	algoExecutions[_data.GetExecutionOrder()->GetProduct().GetProductId()] = _data;
}

template<typename T>
void AlgoExecutionService<T>::AddListener(ServiceListener<AlgoExecution<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<AlgoExecution<T>>*>& AlgoExecutionService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
AlgoExecutionToMarketDataListener<T>* AlgoExecutionService<T>::GetListener()
{
	return listener;
}

template<typename T>
void AlgoExecutionService<T>::AlgoExecuteOrder(OrderBook<T>& _orderBook)
{
	T _product = _orderBook.GetProduct();
	string _productId = _product.GetProductId();
	PricingSide _side;
	string _orderId = GenerateId();
	double _price;
	long _quantity;

	BidOffer _bidOffer = _orderBook.GetBidOffer();
	Order _bidOrder = _bidOffer.GetBidOrder();
	double _bidPrice = _bidOrder.GetPrice();
	long _bidQuantity = _bidOrder.GetQuantity();
	Order _offerOrder = _bidOffer.GetOfferOrder();
	double _offerPrice = _offerOrder.GetPrice();
	long _offerQuantity = _offerOrder.GetQuantity();

	if (_offerPrice - _bidPrice <= spread)
	{
		if (count % 2 == 0)
		{
			_price = _bidPrice;
			_quantity = _bidQuantity;
			_side = BID;
			break;
		}
		else {
			_price = _offerPrice;
			_quantity = _offerQuantity;
			_side = OFFER;
			break;
		}
		count++;
		AlgoExecution<T> _algoExecution(_product, _side, _orderId, MARKET, _price, _quantity, 0, "", false);
		algoExecutions[_productId] = _algoExecution;

		for (auto& l : listeners)
		{
			l->ProcessAdd(_algoExecution);
		}
	}
}


template<typename T>
AlgoExecutionToMarketDataListener<T>::AlgoExecutionToMarketDataListener(AlgoExecutionService<T>* _service)
{
	service = _service;
}

template<typename T>
AlgoExecutionToMarketDataListener<T>::~AlgoExecutionToMarketDataListener() {}

template<typename T>
void AlgoExecutionToMarketDataListener<T>::ProcessAdd(OrderBook<T>& _data)
{
	service->AlgoExecuteOrder(_data);
}

template<typename T>
void AlgoExecutionToMarketDataListener<T>::ProcessRemove(OrderBook<T>& _data) {}

template<typename T>
void AlgoExecutionToMarketDataListener<T>::ProcessUpdate(OrderBook<T>& _data) {}





//ExecutionService::ExecutionSerice()
template<typename T>
ExecutionService<T>::ExecutionService()
{
	executionOrders = map<string, ExecutionOrder<T>>();
	listeners = vector<ServiceListener<ExecutionOrder<T>>*>();
	listener = new ExecutionToAlgoExecutionListener<T>(this);
}

template<typename T>
ExecutionService<T>::~ExecutionService() {}

template<typename T>
ExecutionOrder<T>& ExecutionService<T>::GetData(string _key)
{
	return executionOrders[_key];
}

template<typename T>
void ExecutionService<T>::OnMessage(ExecutionOrder<T>& _data)
{
	executionOrders[_data.GetProduct().GetProductId()] = _data;
}

template<typename T>
void ExecutionService<T>::AddListener(ServiceListener<ExecutionOrder<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<ExecutionOrder<T>>*>& ExecutionService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
ExecutionToAlgoExecutionListener<T>* ExecutionService<T>::GetListener()
{
	return listener;
}

template<typename T>
void ExecutionService<T>::ExecuteOrder(ExecutionOrder<T>& _executionOrder)
{
	string _productId = _executionOrder.GetProduct().GetProductId();
	executionOrders[_productId] = _executionOrder;

	for (auto& l : listeners)
	{
		l->ProcessAdd(_executionOrder);
	}
}





template<typename T>
ExecutionToAlgoExecutionListener<T>::~ExecutionToAlgoExecutionListener() {}

template<typename T>
void ExecutionToAlgoExecutionListener<T>::ProcessAdd(AlgoExecution<T>& _data)
{
	ExecutionOrder<T>* _executionOrder = _data.GetExecutionOrder();
	service->OnMessage(*_executionOrder);
	service->ExecuteOrder(*_executionOrder);
}

template<typename T>
ExecutionToAlgoExecutionListener<T>::ExecutionToAlgoExecutionListener(ExecutionService<T>* _service)
{
	service = _service;
}

template<typename T>
ExecutionToAlgoExecutionListener<T>::~ExecutionToAlgoExecutionListener() {}

template<typename T>
void ExecutionToAlgoExecutionListener<T>::ProcessAdd(AlgoExecution<T>& _data)
{
	ExecutionOrder<T>* _executionOrder = _data.GetExecutionOrder();
	service->OnMessage(*_executionOrder);
	service->ExecuteOrder(*_executionOrder);
}

template<typename T>
void ExecutionToAlgoExecutionListener<T>::ProcessRemove(AlgoExecution<T>& _data) {}

template<typename T>
void ExecutionToAlgoExecutionListener<T>::ProcessUpdate(AlgoExecution<T>& _data) {};
