#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// Constants
const double r = 0.01844;      // Annual risk-free rate (1.844%)
const double T = 31.0 / 365.0; // Time to maturity (1 month = 31 days)

// Point values
const double index_point_value = 1.0; // Each point for S&P 500 index worth $1
const double future_point_value =
    250.0; // Each point for S&P 500 future worth $250
const double option_point_value =
    100.0; // Each point for S&P 500 option worth $100

// Transaction costs (per trade)
const double index_transaction_cost = 1.0 * index_point_value;   // $1
const double future_transaction_cost = 1.0 * future_point_value; // $250
const double option_transaction_cost = 1.0 * option_point_value; // $100

struct MarketData {
    double index_bid = 0, index_ask = 0;
    double future_bid = 0, future_ask = 0;
    bool has_index = false, has_future = false;
};

struct OptionData {
    string option_id;
    string date;
    char cp_flag;        // 'C' for Call, 'P' for Put
    char exercise_style; // 'E' for European
    string exdate;
    double strike;
    double best_bid;
    double best_offer;
    int volume;
};

struct ArbitrageOpportunity {
    string strategy;
    double profit;
    double strike;
    string details;
};

class ArbitrageScanner {
   private:
    MarketData market;
    vector<OptionData> options;
    vector<ArbitrageOpportunity> opportunities;

    double calculateTransactionCost(int num_index, int num_futures,
                                    int num_options) {
        return num_index * index_transaction_cost +
               num_futures * future_transaction_cost +
               num_options * option_transaction_cost;
    }

    void scanPutCallParity(const OptionData& call, const OptionData& put) {
        if (call.strike != put.strike) return;

        double K =
            call.strike / 100000.0; // Strike is in format like 100000 = 1000.00
        double S = (market.index_bid + market.index_ask) / 2.0; // Mid price
        double disc = exp(-r * T);

        // Theoretical Put-Call Parity: C - P = S - K*e^(-rT)
        double theoretical_diff = S - K * disc;

        // Actual market differences
        double actual_diff_high =
            call.best_offer - put.best_bid; // C_ask - P_bid
        double actual_diff_low =
            call.best_bid - put.best_offer; // C_bid - P_ask

        // Strategy 1: Long Call + Short Put (when C - P is too cheap)
        double cost1 =
            calculateTransactionCost(1, 0, 2); // Long index, 2 options
        double cost1_per_point = cost1 / option_point_value;

        if (theoretical_diff - actual_diff_low > cost1_per_point) {
            double profit =
                (theoretical_diff - actual_diff_low) * option_point_value -
                cost1;
            ArbitrageOpportunity opp;
            opp.strategy =
                "Put-Call Parity: Long Call + Short Put + Long Index";
            opp.profit = profit;
            opp.strike = K;
            opp.details = "Long Call@" + to_string(call.best_offer) +
                          ", Short Put@" + to_string(put.best_bid) +
                          ", Long Index@" + to_string(market.index_ask);
            opportunities.push_back(opp);
        }

        // Strategy 2: Short Call + Long Put (when C - P is too expensive)
        double cost2 =
            calculateTransactionCost(1, 0, 2); // Short index, 2 options
        double cost2_per_point = cost2 / option_point_value;

        if (actual_diff_high - theoretical_diff > cost2_per_point) {
            double profit =
                (actual_diff_high - theoretical_diff) * option_point_value -
                cost2;
            ArbitrageOpportunity opp;
            opp.strategy =
                "Put-Call Parity: Short Call + Long Put + Short Index";
            opp.profit = profit;
            opp.strike = K;
            opp.details = "Short Call@" + to_string(call.best_bid) +
                          ", Long Put@" + to_string(put.best_offer) +
                          ", Short Index@" + to_string(market.index_bid);
            opportunities.push_back(opp);
        }
    }

    void scanPutCallFuturesParity(const OptionData& call,
                                  const OptionData& put) {
        if (call.strike != put.strike || !market.has_future) return;

        double K = call.strike / 100000.0;
        double F = (market.future_bid + market.future_ask) / 2.0; // Mid price
        double disc = exp(-r * T);

        // Theoretical Put-Call-Futures Parity: C - P = F*e^(-rT) - K*e^(-rT)
        double theoretical_diff = F * disc - K * disc;

        // Actual market differences
        double actual_diff_high = call.best_offer - put.best_bid;
        double actual_diff_low = call.best_bid - put.best_offer;

        // Strategy 1: Long Call + Short Put + Short Future
        double cost1 = calculateTransactionCost(0, 1, 2); // 1 future, 2 options
        double cost1_per_point = cost1 / option_point_value;

        if (actual_diff_high - theoretical_diff > cost1_per_point) {
            double profit =
                (actual_diff_high - theoretical_diff) * option_point_value -
                cost1;
            ArbitrageOpportunity opp;
            opp.strategy =
                "Put-Call-Futures Parity: Long Call + Short Put + Short Future";
            opp.profit = profit;
            opp.strike = K;
            opp.details = "Long Call@" + to_string(call.best_offer) +
                          ", Short Put@" + to_string(put.best_bid) +
                          ", Short Future@" + to_string(market.future_bid);
            opportunities.push_back(opp);
        }

        // Strategy 2: Short Call + Long Put + Long Future
        if (theoretical_diff - actual_diff_low > cost1_per_point) {
            double profit =
                (theoretical_diff - actual_diff_low) * option_point_value -
                cost1;
            ArbitrageOpportunity opp;
            opp.strategy =
                "Put-Call-Futures Parity: Short Call + Long Put + Long Future";
            opp.profit = profit;
            opp.strike = K;
            opp.details = "Short Call@" + to_string(call.best_bid) +
                          ", Long Put@" + to_string(put.best_offer) +
                          ", Long Future@" + to_string(market.future_ask);
            opportunities.push_back(opp);
        }
    }

   public:
    bool loadIndexData(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Error: Cannot open " << filename << endl;
            return false;
        }

        string line;
        getline(file, line); // Skip header

        // Read the most recent data (assuming data is sorted by date)
        if (getline(file, line)) {
            stringstream ss(line);
            string date, price_str, open_str, high_str, low_str, vol_str,
                change_str;

            getline(ss, date, ',');
            getline(ss, price_str, ',');
            getline(ss, open_str, ',');
            getline(ss, high_str, ',');
            getline(ss, low_str, ',');

            try {
                double price = stod(price_str);
                // Assuming bid-ask spread of 0.1% around mid price
                market.index_bid = price * 0.999;
                market.index_ask = price * 1.001;
                market.has_index = true;
                cout << "Loaded S&P 500 Index: " << price
                     << " (Bid: " << market.index_bid
                     << ", Ask: " << market.index_ask << ")" << endl;
            } catch (const exception& e) {
                cout << "Error parsing index price: " << e.what() << endl;
                return false;
            }
        }

        file.close();
        return market.has_index;
    }

    bool loadFuturesData(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Error: Cannot open " << filename << endl;
            return false;
        }

        string line;
        getline(file, line); // Skip header

        if (getline(file, line)) {
            stringstream ss(line);
            string date, price_str, open_str, high_str, low_str, vol_str,
                change_str;

            getline(ss, date, ',');
            getline(ss, price_str, ',');
            getline(ss, open_str, ',');
            getline(ss, high_str, ',');
            getline(ss, low_str, ',');

            try {
                double price = stod(price_str);
                // Assuming bid-ask spread of 0.05% around mid price
                market.future_bid = price * 0.9995;
                market.future_ask = price * 1.0005;
                market.has_future = true;
                cout << "Loaded S&P 500 Futures: " << price
                     << " (Bid: " << market.future_bid
                     << ", Ask: " << market.future_ask << ")" << endl;
            } catch (const exception& e) {
                cout << "Error parsing futures price: " << e.what() << endl;
                return false;
            }
        }

        file.close();
        return market.has_future;
    }

    bool loadOptionsData(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Error: Cannot open " << filename << endl;
            return false;
        }

        string line;
        while (getline(file, line)) {
            if (line.empty() || line[0] == '#')
                continue; // Skip comments and empty lines

            stringstream ss(line);
            string token;
            vector<string> fields;

            while (getline(ss, token, '\t')) { // Assuming tab-separated
                fields.push_back(token);
            }

            if (fields.size() >= 11) { // Ensure we have enough fields
                try {
                    OptionData opt;
                    opt.option_id = fields[0];
                    opt.date = fields[1];
                    opt.cp_flag = fields[2][0];
                    opt.exercise_style = fields[3][0];
                    opt.exdate = fields[5];
                    opt.strike = stod(
                        fields[10]); // Strike is already multiplied by 1000
                    opt.best_bid = stod(fields[8]);
                    opt.best_offer = stod(fields[9]);
                    opt.volume = stoi(fields[7]);

                    // Only include European options with valid bid/ask
                    if (opt.exercise_style == 'E' && opt.best_bid > 0 &&
                        opt.best_offer > 0) {
                        options.push_back(opt);
                    }
                } catch (const exception& e) {
                    // Skip invalid lines
                    continue;
                }
            }
        }

        file.close();
        cout << "Loaded " << options.size() << " valid European options"
             << endl;
        return !options.empty();
    }

    void scanArbitrageOpportunities() {
        // Group options by strike price
        map<double, vector<OptionData>> options_by_strike;

        for (const auto& opt : options) {
            options_by_strike[opt.strike].push_back(opt);
        }

        cout << "\nScanning for arbitrage opportunities...\n";
        cout << "====================================\n";

        for (const auto& [strike, strike_options] : options_by_strike) {
            OptionData call, put;
            bool has_call = false, has_put = false;

            // Find call and put with same strike
            for (const auto& opt : strike_options) {
                if (opt.cp_flag == 'C' && !has_call) {
                    call = opt;
                    has_call = true;
                } else if (opt.cp_flag == 'P' && !has_put) {
                    put = opt;
                    has_put = true;
                }
            }

            if (has_call && has_put) {
                cout << "Checking Strike: " << (strike / 1000.0) << endl;

                // Scan Put-Call Parity
                if (market.has_index) {
                    scanPutCallParity(call, put);
                }

                // Scan Put-Call-Futures Parity
                if (market.has_future) {
                    scanPutCallFuturesParity(call, put);
                }
            }
        }
    }

    void displayResults() {
        cout << "\n=== ARBITRAGE OPPORTUNITIES ===\n";

        if (opportunities.empty()) {
            cout << "No profitable arbitrage opportunities found.\n";
            return;
        }

        // Sort by profit (descending)
        sort(opportunities.begin(), opportunities.end(),
             [](const ArbitrageOpportunity& a, const ArbitrageOpportunity& b) {
                 return a.profit > b.profit;
             });

        // Group by strike and show best opportunity for each strike
        map<double, ArbitrageOpportunity> best_by_strike;

        for (const auto& opp : opportunities) {
            if (best_by_strike.find(opp.strike) == best_by_strike.end() ||
                best_by_strike[opp.strike].profit < opp.profit) {
                best_by_strike[opp.strike] = opp;
            }
        }

        cout << fixed << setprecision(2);
        int count = 1;

        for (const auto& [strike, opp] : best_by_strike) {
            if (opp.profit > 0) {
                cout << count++ << ". Strike: " << strike << endl;
                cout << "   Strategy: " << opp.strategy << endl;
                cout << "   Net Profit: $" << opp.profit << endl;
                cout << "   Details: " << opp.details << endl;
                cout << "   ----------------------------------------" << endl;
            }
        }

        if (count == 1) {
            cout << "No profitable arbitrage opportunities found after "
                    "transaction costs.\n";
        }
    }
};

int main() {
    ArbitrageScanner scanner;

    cout << "S&P 500 Options Arbitrage Scanner" << endl;
    cout << "==================================" << endl;
    cout << "Risk-free rate: " << (r * 100) << "%" << endl;
    cout << "Time to maturity: " << (T * 365) << " days" << endl;
    cout << "Date: 2020-11-03" << endl << endl;

    // Load market data
    bool index_loaded = false, futures_loaded = false, options_loaded = false;

    // Try to load index data
    vector<string> index_files = {"S&P 500 Historical Data.csv"};
    for (const string& file : index_files) {
        if (scanner.loadIndexData(file)) {
            index_loaded = true;
            break;
        }
    }

    // Try to load futures data
    vector<string> futures_files = {"S&P 500 Futures Historical Data.csv"};
    for (const string& file : futures_files) {
        if (scanner.loadFuturesData(file)) {
            futures_loaded = true;
            break;
        }
    }

    // Try to load options data
    vector<string> options_files = {"wrds-www.wharton.upenn.edu.txt"};
    for (const string& file : options_files) {
        if (scanner.loadOptionsData(file)) {
            options_loaded = true;
            break;
        }
    }

    if (!options_loaded) {
        cout << "Error: Could not load options data. Please ensure the file "
                "exists."
             << endl;
        return 1;
    }

    if (!index_loaded && !futures_loaded) {
        cout << "Warning: No underlying or futures data loaded. Limited "
                "arbitrage scanning."
             << endl;
    }

    // Scan for arbitrage opportunities
    scanner.scanArbitrageOpportunities();

    // Display results
    scanner.displayResults();

    return 0;
}