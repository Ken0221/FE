#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct OptionData {
    int secid;
    string date;
    string exdate;
    string last_date;
    char cp_flag;
    double strike_price;
    double best_bid;
    double best_offer;
    int volume;
    long long optionid;
    int index_flag;
    string issuer;
    char exercise_style;
};

const double r = 0.01844;      // 年利率
const double T = 30.0 / 365.0; // 時間（到期 30 天）
const double S = 3369.20;      // 現貨價格
const double F = 3361.50;      // 期貨價格

// 各商品每點價值
const double index_point = 1.0;
const double future_point = 250.0;
const double option_point = 100.0;

double transaction_cost(int num_index, int num_future, int num_option) {
    return num_index * index_point + num_future * future_point +
           num_option * option_point;
}

int main() {
    // 1. 讀取 S&P 500 Historical Data.csv
    // Date	Price	Open	High	Low	Vol.	Change %
    // 11/03/2020	3,369.20	3,336.20	3,389.50	3,336.20		1.78%
    string filename1 = "S&P 500 Historical Data.csv";
    ifstream index_file(filename1);

    if (!index_file.is_open()) {
        cout << "Error: 無法開啟檔案 " << filename1 << endl;
        return 1;
    }

    cout << "Reading index data from " << filename1 << endl;
    string header_index;
    getline(index_file, header_index); // 跳過標題行
    string line_index;
    getline(index_file, line_index);
    vector<string> tokens_index; // [0:Date, 1:Price, 2:Open, 3:High, 4:Low,
                                 // 5:Vol., 6:Change %]
    while (!line_index.empty()) {
        // 處理引號包裹的欄位
        size_t end_quote = line_index.find('"', 1);
        string token;
        if (end_quote != string::npos) {
            token = line_index.substr(1, end_quote - 1); // 提取引號內的內容
            // cout << "Token: " << token << endl;
            tokens_index.push_back(token);
            line_index.erase(0, end_quote + 2); // 移除引號和逗號
        } else {
            // 引號未閉合，報錯或跳過
            break;
        }
    }

    // 2. 讀取 S&P 500 Futures Historical Data.csv
    // Date	Price	Open	High	Low	Vol.	Change %
    // 11/03/2020	3,361.50	3,304.00	3,382.75 3,301.25	1.66M	1.85%
    string filename2 = "S&P 500 Futures Historical Data.csv";
    ifstream future_file(filename2);

    if (!future_file.is_open()) {
        cout << "Error: 無法開啟檔案 " << filename2 << endl;
        return 1;
    }

    cout << "Reading futures data from " << filename2 << endl;
    string header_future;
    getline(future_file, header_future); // 跳過標題行
    string line_future;
    getline(future_file, line_future); // [0:Date, 1:Price, 2:Open, 3:High,
                                       // 4:Low, 5:Vol., 6:Change %]
    vector<string> tokens_future;
    while (!line_future.empty()) {
        // 處理引號包裹的欄位
        size_t end_quote = line_future.find('"', 1);
        string token;
        if (end_quote != string::npos) {
            token = line_future.substr(1, end_quote - 1); // 提取引號內的內容
            // cout << "Token: " << token << endl;
            tokens_future.push_back(token);
            line_future.erase(0, end_quote + 2); // 移除引號和逗號
        } else {
            // 引號未閉合，報錯或跳過
            break;
        }
    }

    // 3. 讀取 wrds-www.wharton.upenn.edu.txt
    // secid	date	exdate	last_date	cp_flag	strike_price	best_bid
    // best_offer	volume	optionid	index_flag	issuer	exercise_style
    // 108105	2020-11-03	2020-11-20	2020-10-29	C	100000	3263.4
    // 3268.7   0	134786452	1	CBOE S&P 500 INDEX	E
    string filename3 = "wrds-www.wharton.upenn.edu.txt";
    ifstream option_file(filename3);

    if (!option_file.is_open()) {
        cout << "Error: 無法開啟檔案 " << filename3 << endl;
        return 1;
    }

    cout << "Reading options data from " << filename3 << endl;
    string header_option;
    getline(option_file, header_option); // 跳過標題行

    // 定義存儲資料的向量
    vector<OptionData> options;

    // 逐行讀取資料
    string line;
    while (getline(option_file, line)) {
        stringstream ss(line);
        OptionData data;

        // 解析每個欄位
        ss >> data.secid;
        ss >> data.date;
        ss >> data.exdate;
        ss >> data.last_date;
        ss >> data.cp_flag;
        ss >> data.strike_price;
        ss >> data.best_bid;
        ss >> data.best_offer;
        ss >> data.volume;
        ss >> data.optionid;
        ss >> data.index_flag;
        ss >> data.issuer;
        ss >> data.exercise_style;

        // 將解析後的資料加入向量
        options.push_back(data);
    }

    // 測試輸出解析後的資料
    // int cnt = 0;
    // for (const auto& opt : options) {
    //     cnt++;
    //     if (cnt > 10) break; // 只顯示前10筆資料
    //     cout << "SecID: " << opt.secid << ", Date: " << opt.date
    //          << ", Strike Price: " << opt.strike_price
    //          << ", Best Bid: " << opt.best_bid
    //          << ", Best Offer: " << opt.best_offer << endl;
    // }

    cout << fixed << setprecision(2);

    // for (const auto& opt : options) {
    //     double K = opt.strike_price;
    //     double disc = exp(-r * T);

    //     // 理論價格差
    //     double parity_theory = S - K * disc;
    //     double future_theory = F * disc - K * disc;

    //     // 實際價格差
    //     double c_ask = opt.call_ask;
    //     double c_bid = opt.call_bid;
    //     double p_ask = opt.put_ask;
    //     double p_bid = opt.put_bid;

    //     double actual1 = c_ask - p_bid; // 若 C - P 太貴，可能套利
    //     double actual2 = c_bid - p_ask; // 若 C - P 太便宜，可能套利

    //     double cost_combo = transaction_cost(1, 0, 2); // Long Call + Short
    //     Put

    //             cout
    //         << "Strike = " << K << ":\n";

    //     // Put-Call Parity Arbitrage
    //     if (actual1 > parity_theory + cost_combo / option_point) {
    //         double profit =
    //             (actual1 - parity_theory) * option_point - cost_combo;
    //         cout << "  ✅ Arbitrage: Long Call + Short Put\n";
    //         cout << "     Profit = $" << profit << "\n";
    //     }

    //     if (actual2 < parity_theory - cost_combo / option_point) {
    //         double profit =
    //             (parity_theory - actual2) * option_point - cost_combo;
    //         cout << "  ✅ Arbitrage: Short Call + Long Put\n";
    //         cout << "     Profit = $" << profit << "\n";
    //     }

    //     // Put-Call-Futures Parity Arbitrage
    //     double actual_fut = c_ask - p_bid;
    //     double cost_combo2 =
    //         transaction_cost(0, 1, 2); // Long Call + Short Put + Short
    //     Future

    //         if (actual_fut > future_theory + cost_combo2 / option_point) {
    //         double profit =
    //             (actual_fut - future_theory) * option_point - cost_combo2;
    //         cout << "  ✅ Future Arbitrage: Long Call + Short Put + Short "
    //                 "Future\n";
    //         cout << "     Profit = $" << profit << "\n";
    //     }

    //     cout << "----------------------------------\n";
    // }

    return 0;
}
