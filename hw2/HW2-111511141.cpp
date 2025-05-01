#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// const double YTM_range[12][2] = {{0.03, 0.04}, {0.07, 0.08}, {0.02, 0.03},
//                                  {0.05, 0.06}, {0.06, 0.07}, {0.04, 0.05},
//                                  {0.02, 0.03}, {0.05, 0.06}, {0.01, 0.02},
//                                  {0.03, 0.04}, {0.04, 0.05}, {0.05, 0.06}};
const int MONTHS[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const double ERROR = 1e-12;

bool IsLeapYear(int year) {
    if (year % 4 == 0) {
        if (year % 100 == 0) {
            if (year % 400 == 0) {
                return true;
            }
            return false;
        }
        return true;
    }
    return false;
}

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

double f(double P, double FV, double c, int n, double r) {
    double value = 0;
    for (int i = 1; i <= n; i++) {
        value += c / pow(1 + r, i);
    }
    value += FV / pow(1 + r, n);
    value -= P;
    return value;
}

double bisection(double P, double FV, double c, int n, double low, double high,
                 double error) {
    double discount;
    double low_value, high_value, mid_value;
    low_value = f(P, FV, c, n, low);
    high_value = f(P, FV, c, n, high);
    if (low_value == 0) {
        return low;
    } else if (high_value == 0) {
        return high;
    } else if (low_value * high_value > 0) {
        // cout << "f(low) * f(high) > 0, ";
        cout << "low_value = " << low_value << ", high_value = " << high_value
             << endl;
        return -1;
    }

    while (high - low >= error) {
        double mid = (low + high) / 2;
        mid_value = f(P, FV, c, n, mid);
        if (mid_value == 0) {
            return mid;
        } else if (mid_value > 0 && low_value > 0 ||
                   mid_value < 0 && low_value < 0) {
            low = mid;
        } else {
            high = mid;
        }
        // cout << "value = " << value << endl;
        // cout << "low = " << low << ", high = " << high << endl;
    }
    return high;
}

// double newton(double P, double FV, double c, int n, double r, double error) {
//     double value, derivative;
//     do {
//         value = 0, derivative = 0;
//         for (int i = 1; i <= n; i++) {
//             value += c / pow(1 + r, i);
//             derivative += -i * c / pow(1 + r, i + 1);
//         }
//         value += FV / pow(1 + r, n);
//         value -= P;
//         if (derivative != 0) {
//             r -= value / derivative;
//         } else {
//             // cout << "f'(r) = 0, ";
//             // break;
//             return -1;
//         }
//         // cout << "derivative = " << derivative << endl;
//     } while (fabs(value) >= error);
//     return r;
// }

double Approx_YTM(double P, double FV, double C, int N) {
    double value = (C + (FV - P) / N) / ((FV + P) / 2);
    return value;
}

// calculate the bond YTM
double YTM(int half_year_diff, double coupon, double offering_price) {
    double FV = 100;

    double initial = Approx_YTM(offering_price, FV, coupon / 2, half_year_diff);

    double range = 0.1;
    double ytm = bisection(offering_price, FV, coupon / 2, half_year_diff,
                           initial - range, initial + range, ERROR);
    // double ytm_n =
    //     newton(offering_price, FV, coupon / 2, half_year_diff, initial,
    //     ERROR);
    // cout << "newton: " << ytm_n << endl;
    return ytm;
}

int main() {
    cout << fixed << setprecision(5);

    string filename = "qj2v53pmgqa0oh5p.csv";
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

        getline(ss, record.delivery_date.text, ',');
        for (int d = 0; d < 2; d++) {
            for (int i = 0; i < record.delivery_date.text.size(); i++) {
                if (record.delivery_date.text[i] == '/') {
                    record.delivery_date.digit[d] =
                        stoi(record.delivery_date.text.substr(0, i));
                    record.delivery_date.text =
                        record.delivery_date.text.substr(i + 1);
                    break;
                }
            }
        }
        record.delivery_date.digit[2] = stoi(record.delivery_date.text);

        getline(ss, item, ',');
        record.coupon = stod(item);

        records.push_back(record);
    }

    file.close();

    // calculate the bond YTM
    // Note: The bond is assumed to pay coupon semiannually
    for (int i = 0; i < records.size(); i++) {
        cout << i + 1 << ". " << endl;
        BondRecord rec = records[i];

        int year_diff = rec.maturity.digit[0] - rec.offering_date.digit[0];
        int month_diff = rec.maturity.digit[1] - rec.offering_date.digit[1];
        if (month_diff < 0) {
            year_diff--;
            month_diff += 12;
        }
        int day_diff = rec.maturity.digit[2] - rec.offering_date.digit[2];
        if (day_diff < 0) {
            month_diff--;
            day_diff += 30; // assume 30 days in a month
        }
        int total_month_diff = year_diff * 12 + month_diff;
        int half_year_diff = total_month_diff / 6;
        if (total_month_diff % 6 != 0) {
            half_year_diff++;
        }

        double ytm = YTM(half_year_diff, rec.coupon, rec.offering_price);
        ytm *= 2;
        cout << setw(20) << right << "YTM (calculated):" << setw(10) << right
             << ytm * 100 << endl;
        cout << setw(20) << right << "Offering Yield:" << setw(10) << right
             << rec.offering_yield << endl;

        // cout << "days_act: " << days_act << endl;

        // calculate the dirty price and clean price
        // actual/actual
        cout << " Actual/Actual:" << endl;
        int settle = 0;
        Date start_day = rec.offering_date;
        while (start_day.digit[0] != rec.delivery_date.digit[0] ||
               start_day.digit[1] != rec.delivery_date.digit[1] ||
               start_day.digit[2] != rec.delivery_date.digit[2]) {
            settle++;
            start_day.digit[2]++;
            if (IsLeapYear(start_day.digit[0]) && start_day.digit[1] == 2) {
                if (start_day.digit[2] > 29) {
                    start_day.digit[2] = 1;
                    start_day.digit[1]++;
                }
            } else if (start_day.digit[2] > MONTHS[start_day.digit[1] - 1]) {
                start_day.digit[2] = 1;
                start_day.digit[1]++;
                if (start_day.digit[1] > 12) {
                    start_day.digit[1] = 1;
                    start_day.digit[0]++;
                }
            }
        }

        double duration = 0;
        Date start_day_2 = rec.offering_date;
        while (start_day_2.digit[0] != rec.maturity.digit[0] ||
               start_day_2.digit[1] != rec.maturity.digit[1] ||
               start_day_2.digit[2] != rec.maturity.digit[2]) {
            duration++;
            start_day_2.digit[2]++;
            if (IsLeapYear(start_day_2.digit[0]) && start_day_2.digit[1] == 2) {
                if (start_day_2.digit[2] > 29) {
                    start_day_2.digit[2] = 1;
                    start_day_2.digit[1]++;
                }
            } else if (start_day_2.digit[2] >
                       MONTHS[start_day_2.digit[1] - 1]) {
                start_day_2.digit[2] = 1;
                start_day_2.digit[1]++;
                if (start_day_2.digit[1] > 12) {
                    start_day_2.digit[1] = 1;
                    start_day_2.digit[0]++;
                }
            }
        }
        duration /= half_year_diff;
        // cout << "duration: " << duration << endl;

        double omega = (duration - settle) / duration;
        // cout << "days_to_next_payment_day: " << days_to_next_payment_day
        //      << endl;
        // cout << "omega: " << omega << endl;
        double dirty_price = 0;
        for (int j = 0; j < half_year_diff; j++) {
            dirty_price += (rec.coupon / 2) / pow(1 + ytm / 2, j + omega);
        }
        dirty_price += 100 / pow(1 + ytm / 2, half_year_diff - 1 + omega);
        cout << setw(20) << right << "Dirty Price:" << setw(10) << right
             << dirty_price << endl;

        double accrued_interest = rec.coupon / 2 * (1 - omega);
        double clean_price = dirty_price - accrued_interest;
        cout << setw(20) << right << "Clean Price:" << setw(10) << right
             << clean_price << endl;

        // 30/360 ///////////////////////////////////////////////////////
        cout << " 30/360:" << endl;

        int days_30360 = 360 * year_diff + 30 * month_diff + day_diff;
        // cout << "days_30360: " << days_30360 << endl;

        int settle_30360 =
            360 * (rec.delivery_date.digit[0] - rec.offering_date.digit[0]) +
            30 * (rec.delivery_date.digit[1] - rec.offering_date.digit[1]) +
            (rec.delivery_date.digit[2] - rec.offering_date.digit[2]);
        // cout << "settle_30360: " << settle_30360 << endl;

        double omega_30360 = (180.0 - settle_30360) / 180.0;
        // cout << "omega_30360: " << omega_30360 << endl;

        double dirty_price_30360 = 0;
        for (int j = 0; j < half_year_diff; j++) {
            dirty_price_30360 +=
                (rec.coupon / 2) / pow(1 + ytm / 2, j + omega_30360);
        }
        dirty_price_30360 +=
            100 / pow(1 + ytm / 2, half_year_diff - 1 + omega_30360);
        cout << setw(20) << right << "Dirty Price:" << setw(10) << right
             << dirty_price_30360 << endl;

        double accrued_interest_30360 = rec.coupon / 2 * (1 - omega_30360);
        double clean_price_30360 = dirty_price_30360 - accrued_interest_30360;
        cout << setw(20) << right << "Clean Price:" << setw(10) << right
             << clean_price_30360 << endl;
    }

    return 0;
}
