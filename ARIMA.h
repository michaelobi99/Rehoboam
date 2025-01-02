#pragma once
#include <iostream>
#include <vector>
#include <numeric>
#include <cmath>
#include <string>
#include <deque>

// Constants for feature engineering
#define MOVING_AVG_WINDOW 3
#define MOMENTUM_WINDOW 5
#define SEASONAL_PERIOD 7  // Weekly seasonality

// Structure to hold team statistics
struct TeamStats {
    double recent_avg;
    double momentum;
    double seasonal_factor;
    double home_advantage;
    std::vector<double> arima_params;
};

// Calculate moving average
double calculate_moving_average(const std::vector<double>& data, int window) {
    if (data.size() < window) return 0.0;

    double sum = 0.0;
    for (int i = data.size() - window; i < data.size(); ++i) {
        sum += data[i];
    }
    return sum / window;
}

// Calculate momentum (rate of change)
double calculate_momentum(const std::vector<double>& data, int window) {
    if (data.size() < window) return 0.0;

    std::vector<double> changes;
    for (size_t i = data.size() - window; i < data.size() - 1; ++i) {
        changes.push_back(data[i + 1] - data[i]);
    }

    return std::accumulate(changes.begin(), changes.end(), 0.0) / changes.size();
}

// ARIMA model implementation (simplified)
class ARIMA {
private:
    int p, d, q; // ARIMA parameters
    std::vector<double> ar_coeffs;
    std::vector<double> ma_coeffs;

    // Difference the time series
    std::vector<double> difference(const std::vector<double>& data, int d) {
        if (d == 0) return data;
        std::vector<double> diff(data.size() - 1);
        for (size_t i = 0; i < data.size() - 1; ++i) {
            diff[i] = data[i + 1] - data[i];
        }
        return difference(diff, d - 1);
    }

    // Estimate AR coefficients using Yule-Walker equations
    void estimate_ar_coefficients(const std::vector<double>& data) {
        ar_coeffs.resize(p);
        if (p == 0) return;

        // Simple autocorrelation calculation
        for (int i = 0; i < p; ++i) {
            double numerator = 0.0;
            double denominator = 0.0;
            for (size_t t = i + 1; t < data.size(); ++t) {
                numerator += data[t] * data[t - i - 1];
                denominator += data[t - i - 1] * data[t - i - 1];
            }
            ar_coeffs[i] = numerator / denominator;
        }
    }

public:
    ARIMA(int p_ = 1, int d_ = 1, int q_ = 0) : p(p_), d(d_), q(q_) {
        ma_coeffs.resize(q, 0.0);  // Initialize MA coefficients to 0
    }

    void fit(const std::vector<double>& data) {
        // Difference the data
        std::vector<double> diff_data = difference(data, d);

        // Estimate AR coefficients
        estimate_ar_coefficients(diff_data);
    }

    double predict_next(const std::vector<double>& data) {
        std::vector<double> diff_data = difference(data, d);

        // AR prediction
        double prediction = 0.0;
        for (int i = 0; i < p && i < diff_data.size(); ++i) {
            prediction += ar_coeffs[i] * diff_data[diff_data.size() - 1 - i];
        }

        // Revert differencing
        for (int i = 0; i < d; ++i) {
            prediction += data[data.size() - 1 - i];
        }

        return prediction;
    }
};

// Enhanced prediction model
class BasketballPredictor {
private:
    std::vector<double> points_scored;
    TeamStats stats;
    ARIMA arima_model;

public:
    BasketballPredictor() : arima_model(1, 1, 0) {}  // ARIMA(1,1,0) model

    void add_game_data(double scored) {
        points_scored.push_back(scored);
    }

    void calculate_features() {
        // Calculate recent average
        stats.recent_avg = calculate_moving_average(points_scored, MOVING_AVG_WINDOW);

        // Calculate momentum
        stats.momentum = calculate_momentum(points_scored, MOMENTUM_WINDOW);

        // Calculate seasonal factor (simplified)
        if (points_scored.size() >= SEASONAL_PERIOD) {
            double seasonal_sum = 0.0;
            int count = 0;
            for (size_t i = points_scored.size() % SEASONAL_PERIOD;
                i < points_scored.size();
                i += SEASONAL_PERIOD) {
                seasonal_sum += points_scored[i];
                count++;
            }
            stats.seasonal_factor = count > 0 ? seasonal_sum / count : 0.0;
        }

        // Fit ARIMA model
        if (points_scored.size() >= 3) {  // Minimum data requirement
            arima_model.fit(points_scored);
        }
    }

    double predict_next_score() {
        if (points_scored.empty()) return 0.0;

        calculate_features();

        // Combine predictions from different models
        double arima_pred = arima_model.predict_next(points_scored);
        double feature_pred = stats.recent_avg + stats.momentum;

        // Weighted average of predictions
        double alpha = 0.7;  // Weight for ARIMA prediction
        return alpha * arima_pred + (1 - alpha) * feature_pred;
    }
};

void predictARIMA(std::vector<int> const& scores, std::string name){
    BasketballPredictor predictor;
    std::vector<double> points_scored;

    for (int i = 0; i < std::size(scores); ++i) points_scored.push_back(scores[i]);
    
    // Add data to predictor
    for (size_t i = 0; i < points_scored.size(); ++i) {
        predictor.add_game_data(points_scored[i]);
    }

    // Make predictions
    double next_score = predictor.predict_next_score();

    std::cout << "Predicted scoring points for " <<name <<": " << next_score << "\n";


}