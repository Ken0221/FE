#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

double interpolate(double x, double x0, double x1, double y0, double y1) {
    return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}

int main() {
    map<int, double> swap_rate = {
        {1, 2.26}, {2, 2.275}, {3, 2.285}, {5, 2.355}, {7, 2.44}};
    swap_rate[4] = interpolate(4, 3, 5, swap_rate[3], swap_rate[5]);
    swap_rate[6] = interpolate(6, 5, 7, swap_rate[5], swap_rate[7]);

    cout << "年期\tSwap Rate (%)" << endl;
    cout << fixed << setprecision(3);
    for (int i = 1; i <= 7; ++i) {
        cout << i << "\t" << swap_rate[i] << "%" << endl;
    }

    map<int, double> spot_rate;

    for (int N = 1; N <= 7; N++) {
        double F = swap_rate[N] / 100.0;

        if (N == 1) {
            // 第一年直接由公式解出
            // (1 + F) * e^{-s1} = 1，因為swap價值為0，所以貼現率為0，價值為1
            // s1 = -log(1 / (1 + F))
            double s1 = -log(1.0 / (1.0 + F));
            spot_rate[1] = s1;
        } else {
            // 解聯立方程：Σ_i=1^N-1 (F * e^{-si*i}) + (1 + F)*e^{-sN*N} = 1
            // 用 newton method 找 sN，使得公式收斂到 1
            double sN = 0.03;
            double diff = 1.0;
            double epsilon = 1e-6;
            while (fabs(diff) > epsilon) {
                double sum = 0.0;
                for (int i = 1; i < N; ++i) {
                    sum += F * exp(-spot_rate[i] * i);
                }
                sum += (1.0 + F) * exp(-sN * N);
                diff = sum - 1.0;

                // 更新 sN
                double derivative = 0.0;
                for (int i = 1; i < N; ++i) {
                    derivative -= F * i * exp(-spot_rate[i] * i);
                }
                derivative -= (1.0 + F) * N * exp(-sN * N);

                sN -= diff / derivative;
            }

            spot_rate[N] = sN;
        }
    }
    cout << "年期\tZero Rate (%)" << endl;
    for (int i = 1; i <= 7; ++i) {
        cout << i << "\t" << fixed << setprecision(6) << spot_rate[i] * 100.0
             << endl;
    }
    return 0;
}