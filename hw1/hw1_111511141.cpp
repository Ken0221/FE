#include <cmath>
#include <iostream>
using namespace std;

double f(double x, double c[], int n, double r) {
    double value = 0;
    double discount;
    for (int i = 1; i <= n; i++) {
        discount = 1;
        for (int j = 1; j <= i; j++) {
            discount /= (1 + r); // (1 + r)^-n
        }
        value += discount * c[i - 1]; // c1 * (1 + r)^-1 + c2 * (1 + r)^-2
    }
    value -= x; // f(r) = c1 * (1 + r)^-1 + c2 * (1 + r)^-2 - x
    return value;
}

double bisection(double x, double c[], int n, double low, double high,
                 double error) {
    double discount;
    double low_value, high_value, mid_value;
    low_value = f(x, c, n, low);
    high_value = f(x, c, n, high);
    if (low_value == 0) {
        return low;
    } else if (high_value == 0) {
        return high;
    } else if (low_value * high_value > 0) {
        // cout << "f(low) * f(high) > 0, ";
        return -1;
    }

    while (high - low >= error) {
        double mid = (low + high) / 2;
        mid_value = f(x, c, n, mid);
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

double newton(double x, double c[], int n, double r, double error) {
    double discount, value, derivative;
    do {
        value = 0, derivative = 0;
        for (int i = 1; i <= n; i++) {
            discount = 1;
            for (int j = 1; j <= i; j++) {
                discount /= (1 + r); // (1 + r)^-n
            }
            value += discount * c[i - 1]; // c1 * (1 + r)^-1 + c2 * (1 + r)^-2
            derivative += -i * (discount / (1 + r)) * c[i - 1];
            // -n * c[n] * (1 + r)^-n-1
            // -1 * c1 * (1 + r)^-2 + -2 * c2 * (1 + r)^-3
        }
        value -= x;            // f(r) = c1 * (1 + r)^-1 + c2 * (1 + r)^-2 - x
        if (derivative != 0) { // f'(r) = 0
            r -= value / derivative; // r_k+1 = r_k - f(r) / f'(r)
        } else {
            // cout << "f'(r) = 0, ";
            // break;
            return -1;
        }
        // cout << "derivative = " << derivative << endl;
    } while (fabs(value) >= error);
    return r;
}

int main() {
    double x = -9702, c[] = {-19700, 10000}, error = 1e-12;
    int n = 2;

    cout << "bisection method" << endl;

    cout << "1. ";
    double low_1 = 0, high_1 = 0.015;
    double bisect_1 = bisection(x, c, n, low_1, high_1, error);
    if (bisect_1 == -1) {
        cout << "f(low) * f(high) > 0" << endl;
    } else
        cout << "IRR = " << bisect_1 * 100 << " %" << endl;

    cout << "2. ";
    double low_2 = 0.015, high_2 = 0.03;
    double bisect_2 = bisection(x, c, n, low_2, high_2, error);
    if (bisect_2 == -1) {
        cout << "f(low) * f(high) > 0" << endl;
    } else
        cout << "IRR = " << bisect_2 * 100 << " %" << endl;

    cout << endl;
    cout << "newton method" << endl;

    cout << "1. ";
    double initial_r_1 = 0.01;
    double newt_1 = newton(x, c, n, initial_r_1, error);
    if (newt_1 == -1) {
        cout << "f'(r) = 0" << endl;
    } else
        cout << "IRR = " << newt_1 * 100 << " %" << endl;

    cout << "2. ";
    // double initial_r_2 = 0.0152284; // f'(r) = 0
    double initial_r_2 = 0.02;
    double newt_2 = newton(x, c, n, initial_r_2, error);
    if (newt_2 == -1) {
        cout << "f'(r) = 0" << endl;
    } else
        cout << "IRR = " << newt_2 * 100 << " %" << endl;

    return 0;
}