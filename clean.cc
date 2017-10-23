#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <stdlib.h>
#include <time.h>

using namespace std;

struct Trade {
  struct tm ts;
  string broker;
  int id;
  char type;
  string symbol;
  int quantity;
  float price;
  bool side;
  bool accepted;
};

int main() {
  ofstream acceptedOrders, rejectedOrders, allAccepted, allRejected;
  acceptedOrders.open("accepted_orders.txt");
  rejectedOrders.open("rejected_orders.txt");
  allAccepted.open("all_accepted_orders_in_detail.txt");
  allRejected.open("all_rejected_order_in_detail.txt");
  ifstream symbolsFile("symbols.txt");
  string line;
  set<string> symbols;
  while(getline(symbolsFile, line)) {
    symbols.insert(line);
  }
  ifstream firmsFile("firms.txt");
  set<string> firms;
  while(getline(firmsFile, line)) {
    firms.insert(line);
  }
  map<string, vector<tm> > timeSeries;
  map<int, int> idMap;
  ifstream tradesFile("trades.csv");
  int count = 0;
  // Read and discard the first line, i.e., column names
  getline(tradesFile, line);
  while(getline(tradesFile, line)) {
    istringstream s(line);
    string field;
    vector<string> fields;
    bool accepted = true;
    while(getline(s, field, ',')) {
      // cout << field;
      if (field.empty()) {
        cout << "Empty field! " << endl;
        accepted = false;
      }
      fields.push_back(field);
    }
    // cout << endl;
    Trade aTrade;
    string tsString;
    vector<string> tsStrings;
    istringstream ts(fields[0]);
    while(getline(ts, tsString, ' ')) {
      tsStrings.push_back(tsString);
    }
    string d;
    vector<int> ds;
    istringstream ts0(tsStrings[0]);
    while(getline(ts0, d, '/')) {
      ds.push_back(atoi(d.c_str()));
    }
    string t;
    istringstream ts1(tsStrings[1]);
    while(getline(ts1, t, ':')) {
      ds.push_back(atoi(t.c_str()));
    }
    aTrade.ts.tm_mon = ds[0];
    aTrade.ts.tm_mday = ds[1];
    aTrade.ts.tm_year = ds[2];
    aTrade.ts.tm_hour = ds[3];
    aTrade.ts.tm_min = ds[4];
    aTrade.ts.tm_sec = ds[5];

    aTrade.broker = fields[1];

    vector<tm> *tsp = &timeSeries[aTrade.broker];
    if (tsp->size() < 3) {
      tsp->push_back(aTrade.ts);
    } else {
      double seconds = difftime(mktime(&aTrade.ts), mktime(&((*tsp)[0])));
      if (seconds > 60) {
        tsp->erase(timeSeries[aTrade.broker].begin());
        tsp->push_back(aTrade.ts);
      } else {
        accepted = false;
      }
    }

    aTrade.id = atoi(fields[2].c_str());
    if (idMap.find(aTrade.id) != idMap.end()) {
      cout << "Id " << aTrade.id << " already exists!" << endl;
      accepted = false;
    } else {
      idMap[aTrade.id] = count;
      cout << "idMap: id = " << aTrade.id << "; index = " << count << endl;
    }
    aTrade.type = fields[3].c_str()[0];
    aTrade.symbol = fields[4];
    if (symbols.find(aTrade.symbol) == symbols.end()) {
      cout << "Symbol " << aTrade.symbol << " does not exist!" << endl;
      accepted = false;
    }
    aTrade.quantity = atoi(fields[5].c_str());
    aTrade.price = atof(fields[6].c_str());
    aTrade.side = strcasecmp(fields[7].c_str(), "Buy");
    aTrade.accepted = accepted;
    if (accepted) {
      acceptedOrders << aTrade.broker << " " << aTrade.id << endl; 
      allAccepted << line << endl; 
    } else {
      rejectedOrders << aTrade.broker << " " << aTrade.id << endl;
      allRejected << line << endl;
    }
    count++;
  }
  cout << "count = " << count << endl;
  acceptedOrders.close();
  rejectedOrders.close();
  return 0;
}
