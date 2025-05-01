#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Date {
    string text;
    int digit[3]; // year, month, day
};

struct BondRecord {
    string issuer_id;
    Date maturity;
    Date offering_date;
    double offering_price;
    double offering_yield;
    Date delivery_date;
    double coupon;
};

double MD(int n, double c, double r) {
    double Value = 0, Discount, Duration = 0;
    cout << "   期數: " << n << endl;
    cout << "   債息: " << c << endl;
    cout << "   利率: " << r << endl;
    for (int i = 1; i <= n; i++) {
        Discount = 1;
        Discount /= pow(1 + r, i);
        Duration += i * c * Discount;
        Value += Discount * c;
        if (i == n) {
            Duration += n * 100 * Discount;
            Value += Discount * 100;
        }
    }
    // cout << "   Value: " << Value << endl;
    Duration /= Value;
    return Duration;
}

int main() {
    // 1. 利用FISD的資料計算原始Duration
    cout << "1. 利用FISD的資料計算原始Duration" << endl;
    string filename = "alnk7y2rjxjtjygt.csv";
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Cannot open the file: " << filename << endl;
        return 1;
    }

    string line;
    getline(file, line); // skip the first line (header)

    vector<BondRecord> records;

    while (getline(file, line)) { // read the file
        stringstream ss(line);
        string item;
        BondRecord record;

        getline(ss, record.issuer_id, ',');
        getline(ss, record.maturity.text, ',');
        for (int d = 0; d < 2; d++) {
            for (int i = 0; i < record.maturity.text.size(); i++) {
                if (record.maturity.text[i] == '/') {
                    record.maturity.digit[d] =
                        stoi(record.maturity.text.substr(0, i));
                    record.maturity.text = record.maturity.text.substr(i + 1);
                    break;
                }
            }
        }
        record.maturity.digit[2] = stoi(record.maturity.text);

        getline(ss, record.offering_date.text, ',');
        for (int d = 0; d < 2; d++) {
            for (int i = 0; i < record.offering_date.text.size(); i++) {
                if (record.offering_date.text[i] == '/') {
                    record.offering_date.digit[d] =
                        stoi(record.offering_date.text.substr(0, i));
                    record.offering_date.text =
                        record.offering_date.text.substr(i + 1);
                    break;
                }
            }
        }
        record.offering_date.digit[2] = stoi(record.offering_date.text);

        getline(ss, item, ',');
        record.offering_price = stod(item);

        getline(ss, item, ',');
        record.offering_yield = stod(item);

        getline(ss, item, ',');
        record.coupon = stod(item);

        records.push_back(record);
    }

    file.close();

    double MD1 =
        MD(records[0].maturity.digit[0] - records[0].offering_date.digit[0],
           records[0].coupon, records[0].offering_yield / 100);
    cout << "   Duration: " << MD1 << endl;

    // 2. 計算當債息為0時的Duration
    cout << "2. 計算當債息為0時的Duration" << endl;
    double MD2 =
        MD(records[0].maturity.digit[0] - records[0].offering_date.digit[0], 0,
           records[0].offering_yield / 100);
    cout << "   Duration: " << MD2 << endl;

    // 3. 計算當債息提高&下降時的Duration(上漲以+10%計算，下跌以-10%計算)
    cout << "3-1. 計算當債息提高10%時的Duration" << endl;
    double MD3 =
        MD(records[0].maturity.digit[0] - records[0].offering_date.digit[0],
           records[0].coupon * 1.1, records[0].offering_yield / 100);
    cout << "   Duration: " << MD3 << endl;

    cout << "3-2. 計算當債息下降10%時的Duration" << endl;
    double MD4 =
        MD(records[0].maturity.digit[0] - records[0].offering_date.digit[0],
           records[0].coupon * 0.9, records[0].offering_yield / 100);
    cout << "   Duration: " << MD4 << endl;

    // 4. 利用計算出的原始Duration去計算Modified duration
    cout << "4. 利用計算出的原始Duration去計算Modified duration" << endl;
    double MD5 = MD1 / (1 + records[0].offering_yield / 100);
    cout << "   Modified Duration: " << MD5 << endl;

    // 5. 計算當殖利率變動一個basis point時，該債券價格變動的百分比
    // basis point = 0.01%
    // price change (%) = -MD * delta_r
    cout << "5. 計算當殖利率變動一個basis point時，該債券價格變動的百分比"
         << endl;
    double delta_r = 0.0001;
    double price_change = -MD5 * delta_r;
    cout << "   Price Change (%): " << price_change * 100 << endl;
    return 0;
}
