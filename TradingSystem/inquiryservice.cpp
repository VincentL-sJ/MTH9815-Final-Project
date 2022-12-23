#include "inquiryservice.hpp"

template<typename T>
Inquiry<T>::Inquiry(string _inquiryId, const T& _product, Side _side, long _quantity, double _price, InquiryState _state) :
	product(_product)
{
	inquiryId = _inquiryId;
	side = _side;
	quantity = _quantity;
	price = _price;
	state = _state;
}

template<typename T>
const string& Inquiry<T>::GetInquiryId() const
{
	return inquiryId;
}

template<typename T>
const T& Inquiry<T>::GetProduct() const
{
	return product;
}

template<typename T>
Side Inquiry<T>::GetSide() const
{
	return side;
}

template<typename T>
long Inquiry<T>::GetQuantity() const
{
	return quantity;
}

template<typename T>
double Inquiry<T>::GetPrice() const
{
	return price;
}

template<typename T>
double Inquiry<T>::SetPrice(double _price)
{
	price = _price;
}

template<typename T>
InquiryState Inquiry<T>::GetState() const
{
	return state;
}

template<typename T>
void Inquiry<T>::SetState(InquiryState _state)
{
	state = _state;
}

template<typename T>
vector<string> Inquiry<T>::ToStrings() const
{
	string _inquiryId = inquiryId;
	string _product = product.GetProductId();
	string _side;
	if (side == BUY)
	{
		_side = "BUY";
		break;
	} else {
		_side = "SELL";
		break;
	}
	string _quantity = to_string(quantity);
	string _price = ConvertPrice(price);
	string _state;
	switch (state)
	{
	case RECEIVED:
		_state = "RECEIVED";
		break;
	case QUOTED:
		_state = "QUOTED";
		break;
	case DONE:
		_state = "DONE";
		break;
	case REJECTED:
		_state = "REJECTED";
		break;
	case CUSTOMER_REJECTED:
		_state = "CUSTOMER_REJECTED";
		break;
	}

	vector<string> _strings;
	_strings.push_back(_inquiryId);
	_strings.push_back(_product);
	_strings.push_back(_side);
	_strings.push_back(_quantity);
	_strings.push_back(_price);
	_strings.push_back(_state);
	return _strings;
}



//InquiryServuce
template<typename T>
InquiryService<T>::InquiryService()
{
	inquiries = map<string, Inquiry<T>>();
	listeners = vector<ServiceListener<Inquiry<T>>*>();
	connector = new InquiryConnector<T>(this);
}

template<typename T>
InquiryService<T>::~InquiryService() {}

template<typename T>
Inquiry<T>& InquiryService<T>::GetData(string _key)
{
	return inquiries[_key];
}

template<typename T>
void InquiryService<T>::OnMessage(Inquiry<T>& _data)
{
	InquiryState _state = _data.GetState();
	switch (_state)
	{
	case RECEIVED:
		inquiries[_data.GetInquiryId()] = _data;
		connector->Publish(_data);
		break;
	case QUOTED:
		_data.SetState(DONE);
		inquiries[_data.GetInquiryId()] = _data;

		for (auto& l : listeners)
		{
			l->ProcessAdd(_data);
		}

		break;
	}
}

template<typename T>
void InquiryService<T>::AddListener(ServiceListener<Inquiry<T>>* _listener)
{
	listeners.push_back(_listener);
}

template<typename T>
const vector<ServiceListener<Inquiry<T>>*>& InquiryService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
InquiryConnector<T>* InquiryService<T>::GetConnector()
{
	return connector;
}

template<typename T>
void InquiryService<T>::SendQuote(const string& _inquiryId, double _price)
{
	Inquiry<T>& _inquiry = inquiries[_inquiryId];
	InquiryState _state = _inquiry.GetState();
	_inquiry.SetPrice(_price);
	for (auto& l : listeners)
	{
		l->ProcessAdd(_inquiry);
	}
}

template<typename T>
void InquiryService<T>::RejectInquiry(const string& _inquiryId)
{
	Inquiry<T>& _inquiry = inquiries[_inquiryId];
	_inquiry.SetState(REJECTED);
}


//TODO : Inquiry connector
template<typename T>
InquiryConnector<T>::InquiryConnector(InquiryService<T>* _service)
{
	service = _service;
}

template<typename T>
InquiryConnector<T>::~InquiryConnector() {}

template<typename T>
void InquiryConnector<T>::Publish(Inquiry<T>& _data)
{
	InquiryState _state = _data.GetState();
	if (_state == RECEIVED)
	{
		_data.SetState(QUOTED);
		this->Subscribe(_data);
	}
}

template<typename T>
void InquiryConnector<T>::Subscribe(ifstream& _data)
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

		string _inquiryId = _cells[0];
		string _productId = _cells[1];
		Side _side;
		if (_cells[2] == "BUY") _side = BUY;
		else if (_cells[2] == "SELL") _side = SELL;
		long _quantity = stol(_cells[3]);
		double _price = ConvertPrice(_cells[4]);
		InquiryState _state;
		if (_cells[5] == "RECEIVED") _state = RECEIVED;
		else if (_cells[5] == "QUOTED") _state = QUOTED;
		else if (_cells[5] == "DONE") _state = DONE;
		else if (_cells[5] == "REJECTED") _state = REJECTED;
		else if (_cells[5] == "CUSTOMER_REJECTED") _state = CUSTOMER_REJECTED;
		T _product = GetBond(_productId);
		Inquiry<T> _inquiry(_inquiryId, _product, _side, _quantity, _price, _state);
		service->OnMessage(_inquiry);
	}
}

template<typename T>
void InquiryConnector<T>::Subscribe(Inquiry<T>& _data)
{
	service->OnMessage(_data);
}