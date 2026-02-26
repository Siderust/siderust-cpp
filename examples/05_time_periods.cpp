/**
 * @file 05_time_periods.cpp
 * @brief C++ port of siderust/examples/41_time_periods.rs
 *
 * Demonstrates the generic Period<T> with different time types:
 * - JulianDate periods
 * - MJD (ModifiedJulianDate) periods
 * - Conversions between time systems
 *
 * Run with: cmake --build build --target time_periods_example
 */

#include <iomanip>
#include <iostream>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;

int main() {
    std::cout << "Generic Time Period Examples\n";
    std::cout << "============================\n\n";

    // =========================================================================
    // 1. Period with JulianDate
    // =========================================================================
    std::cout << "1. Period with JulianDate:\n";
    const JulianDate jd_start = JulianDate(2451545.0); // J2000.0
    const JulianDate jd_end   = JulianDate(2451546.5); // 1.5 days later
    std::cout << "   Start: JD " << std::fixed << std::setprecision(6) << jd_start.value() << "\n";
    std::cout << "   End:   JD " << jd_end.value() << "\n";
    std::cout << "   Duration: " << (jd_end.value() - jd_start.value()) << " days\n\n";

    // =========================================================================
    // 2. Period with ModifiedJulianDate (MJD)
    // =========================================================================
    std::cout << "2. Period with ModifiedJulianDate:\n";
    const MJD mjd_start = MJD(59000.0);
    const MJD mjd_end   = MJD(59002.5);
    const Period mjd_period(mjd_start, mjd_end);
    std::cout << "   Start: MJD " << mjd_start.value() << "\n";
    std::cout << "   End:   MJD " << mjd_end.value() << "\n";
    std::cout << "   Duration: " << mjd_period.duration().value() << " days\n\n";

    // =========================================================================
    // 3. JulianDate <-> MJD conversion
    //    Relationship: MJD = JD - 2400000.5
    // =========================================================================
    std::cout << "3. Converting between time systems:\n";
    const MJD mjd_j2000 = MJD(51544.5);       // MJD at J2000.0
    const JulianDate jd_from_mjd = JulianDate(mjd_j2000.value() + 2400000.5);
    std::cout << "   MJD: " << mjd_j2000.value() << "\n";
    std::cout << "   JD:  " << jd_from_mjd.value() << " (should be 2451545.0)\n\n";

    // =========================================================================
    // 4. Period arithmetic
    // =========================================================================
    std::cout << "4. Period arithmetic:\n";
    const MJD night_start = MJD(60000.0);
    const MJD night_end   = MJD(60000.5);          // 12 hours later
    const Period night(night_start, night_end);

    std::cout << "   Night period start:    MJD " << night.start().value() << "\n";
    std::cout << "   Night period end:      MJD " << night.end().value() << "\n";
    std::cout << "   Duration in days:      " << night.duration().value() << " days\n";
    std::cout << "   Duration in hours:     " << night.duration<qtty::Hour>().value() << " h\n\n";

    // =========================================================================
    // 5. J2000.0 epoch constant
    // =========================================================================
    std::cout << "5. J2000.0 constants:\n";
    std::cout << "   JulianDate::J2000() = JD " << std::setprecision(1)
              << JulianDate::J2000().value() << "\n";
    std::cout << "   Corresponding MJD = " << std::setprecision(3)
              << (JulianDate::J2000().value() - 2400000.5) << "\n\n";

    std::cout << "=== Example Complete ===\n";
    return 0;
}
