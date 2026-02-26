/**
 * @file 08_night_quality_scoring.cpp
 * @brief C++ port of siderust/examples/35_night_quality_scoring.rs
 *
 * Scores each night in a month based on Moon interference and astronomical
 * darkness duration. Uses Sun/Moon altitude APIs for both criteria.
 *
 * Run with: cmake --build build --target night_quality_scoring_example
 */

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

struct NightScore {
    double start_mjd;
    double dark_hours;
    double moon_up_hours;
    double score;
};

static NightScore score_night(double night_start_mjd, const Geodetic &obs) {
    const MJD start(night_start_mjd);
    const MJD end(night_start_mjd + 1.0);
    const Period window(start, end);

    // Astronomical darkness (Sun below -18°)
    const auto dark_periods = sun::below_threshold(obs, window, qtty::Degree(-18.0));
    double dark_hours = 0.0;
    for (const auto &p : dark_periods)
        dark_hours += p.duration<qtty::Hour>().value();

    // Moon above horizon
    const auto moon_up = moon::above_threshold(obs, window, qtty::Degree(0.0));
    double moon_hours = 0.0;
    for (const auto &p : moon_up)
        moon_hours += p.duration<qtty::Hour>().value();

    // Score = dark_hours * (1 - 0.7 * moon_interference)
    double moon_interference = std::min(moon_hours / 24.0, 1.0);
    double sc = dark_hours * (1.0 - 0.7 * moon_interference);

    return {night_start_mjd, dark_hours, moon_hours, sc};
}

int main() {
    std::cout << "\u2554\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\n";
    std::cout << "\u2551         Monthly Observing Conditions Report           \u2551\n";
    std::cout << "\u255a\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\n\n";

    // Mauna Kea Observatory, Hawaii
    const Geodetic obs = Geodetic(-155.472, 19.826, 4207.0);
    std::cout << "Observatory: Mauna Kea, Hawaii\n";
    std::cout << "  lat = 19.826 N, lon = -155.472 E, elev = 4207 m\n\n";

    const double start_mjd = 60000.0;
    std::vector<NightScore> scores;
    scores.reserve(30);

    std::cout << "Analyzing 30 nights starting MJD " << std::fixed
              << std::setprecision(0) << start_mjd << "...\n\n";

    for (int day = 0; day < 30; ++day) {
        scores.push_back(score_night(start_mjd + day, obs));
    }

    // Print table header
    std::cout << std::setw(10) << "MJD"
              << std::setw(12) << "Dark(h)"
              << std::setw(12) << "Moon(h)"
              << std::setw(10) << "Score"
              << "\n";
    std::cout << std::string(44, '-') << "\n";

    for (const auto &s : scores) {
        std::cout << std::setw(10) << std::setprecision(0) << s.start_mjd
                  << std::setw(12) << std::setprecision(2) << s.dark_hours
                  << std::setw(12) << s.moon_up_hours
                  << std::setw(10) << std::setprecision(3) << s.score
                  << "\n";
    }

    // Summary statistics
    auto best  = *std::max_element(scores.begin(), scores.end(),
                                   [](const NightScore &a, const NightScore &b) {
                                       return a.score < b.score;
                                   });
    auto worst = *std::min_element(scores.begin(), scores.end(),
                                   [](const NightScore &a, const NightScore &b) {
                                       return a.score < b.score;
                                   });

    std::cout << "\n--- Summary ---\n";
    std::cout << "Best night:  MJD " << std::setprecision(0) << best.start_mjd
              << "  score=" << std::setprecision(3) << best.score
              << "  dark=" << std::setprecision(2) << best.dark_hours << " h\n";
    std::cout << "Worst night: MJD " << std::setprecision(0) << worst.start_mjd
              << "  score=" << std::setprecision(3) << worst.score
              << "  dark=" << std::setprecision(2) << worst.dark_hours << " h\n";

    std::cout << "\n=== Example Complete ===\n";
    return 0;
}
