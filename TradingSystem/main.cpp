//driver for services

#include <iostream>
#include <string>
#include <map>

#include "soa.hpp"
#include "products.hpp"
#include "executionservice.h"
#include "marketdataservice.hpp"
#include "pricingservice.hpp"
#include "historicaldataservice.hpp"
#include "riskservice.hpp"
#include "tradebookingservice.hpp"
#include "inquiryservice.hpp"

using namespace std;


int main()
{
	cout << TimeStamp() << "Trading System Starting..." << endl;
	cout << TimeStamp() << "Trading System Started." << endl;

	cout << TimeStamp() << "Services Starting up..." << endl;

	MarketDataService<Bond> marketDataService;
	PricingService<Bond> pricingService;
	ExecutionService<Bond> executionService;
	TradeBookingService<Bond> tradeBookingService;
	PositionService<Bond> positionService;
	RiskService<Bond> riskService;
	InquiryService<Bond> inquiryService;
	HistoricalDataService<Position<Bond>> historicalPositionService(POSITION);
	HistoricalDataService<ExecutionOrder<Bond>> historicalExecutionService(EXECUTION);
	cout << TimeStamp() << "Services Initialized." << endl;

	cout << TimeStamp() << "Services Linking..." << endl;

	marketDataService.AddListener(executionService.GetListener());
	executionService.AddListener(tradeBookingService.GetListener());
	executionService.AddListener(historicalExecutionService.GetListener());
	tradeBookingService.AddListener(positionService.GetListener());
	positionService.AddListener(riskService.GetListener());
	positionService.AddListener(historicalPositionService.GetListener());
	inquiryService.AddListener(historicalInquiryService.GetListener());

	cout << TimeStamp() << "Processing Price Data ..." << endl;
	ifstream priceData("prices.txt");
	pricingService.GetConnector()->Subscribe(priceData);
	cout << TimeStamp() << "Price Data Processed." << endl;

	cout << TimeStamp() << "Processing Trade Data..." << endl;
	ifstream tradeData("trades.txt");
	tradeBookingService.GetConnector()->Subscribe(tradeData);
	cout << TimeStamp() << "Trade Data Processed." << endl;

	cout << TimeStamp() << "Processing Market Data ..." << endl;
	ifstream marketData("marketdata.txt");
	marketDataService.GetConnector()->Subscribe(marketData);
	cout << TimeStamp() << "Market Data Processed." << endl;

	//TODO Inquiry Data

	cout << TimeStamp() << "Program Ending..." << endl;
	cout << TimeStamp() << "Program Ended." << endl;
	system("pause");
	return 0;
}

