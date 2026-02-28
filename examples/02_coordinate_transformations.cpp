// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// Coordinate Transformations Example
///
/// Build & run:
///   cmake --build build-local --target 02_coordinate_transformations_example
///   ./build-local/02_coordinate_transformations_example

#include <siderust/siderust.hpp>

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace siderust;
using namespace siderust::frames;
using namespace siderust::centers;
using AU = qtty::AstronomicalUnit;

int main() {
    std::cout << "=== Coordinate Transformations Example ===\n" << std::endl;

    auto jd = JulianDate::J2000();
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Reference time: J2000.0 (JD " << jd << ")\n" << std::endl;

    // =========================================================================
    // 1. Frame Transformations (same center)
    // =========================================================================
    std::cout << "1. FRAME TRANSFORMATIONS" << std::endl;
    std::cout << "------------------------" << std::endl;

    // Start with ecliptic coordinates (heliocentric)
    cartesian::Position<Heliocentric, EclipticMeanJ2000, AU> pos_ecliptic(1.0, 0.0, 0.0);
    std::cout << std::setprecision(6);
    std::cout << "Original (Heliocentric EclipticMeanJ2000):" << std::endl;
    std::cout << "  X = " << pos_ecliptic.x() << std::endl;
    std::cout << "  Y = " << pos_ecliptic.y() << std::endl;
    std::cout << "  Z = " << pos_ecliptic.z() << "\n" << std::endl;

    // Transform to equatorial frame (same heliocentric center)
    auto pos_equatorial = pos_ecliptic.to_frame<EquatorialMeanJ2000>(jd);
    std::cout << "Transformed to EquatorialMeanJ2000 frame:" << std::endl;
    std::cout << "  X = " << pos_equatorial.x() << std::endl;
    std::cout << "  Y = " << pos_equatorial.y() << std::endl;
    std::cout << "  Z = " << pos_equatorial.z() << "\n" << std::endl;

    // Transform to ICRS frame
    auto pos_hcrs = pos_equatorial.to_frame<ICRS>(jd);
    std::cout << "Transformed to ICRS frame:" << std::endl;
    std::cout << "  X = " << pos_hcrs.x() << std::endl;
    std::cout << "  Y = " << pos_hcrs.y() << std::endl;
    std::cout << "  Z = " << pos_hcrs.z() << "\n" << std::endl;

    // =========================================================================
    // 2. Center Transformations (same frame)
    // =========================================================================
    std::cout << "2. CENTER TRANSFORMATIONS" << std::endl;
    std::cout << "-------------------------" << std::endl;

    // Get Earth's position (heliocentric ecliptic)
    auto earth_helio = ephemeris::earth_heliocentric(jd);
    std::cout << "Earth (Heliocentric EclipticMeanJ2000):" << std::endl;
    std::cout << "  X = " << earth_helio.x() << std::endl;
    std::cout << "  Y = " << earth_helio.y() << std::endl;
    std::cout << "  Z = " << earth_helio.z() << std::endl;
    std::cout << "  Distance = " << earth_helio.distance() << "\n" << std::endl;

    // Transform to geocentric (Earth becomes origin)
    auto earth_geo = earth_helio.to_center<Geocentric>(jd);
    std::cout << std::setprecision(10);
    std::cout << "Earth (Geocentric EclipticMeanJ2000) - at origin:" << std::endl;
    std::cout << "  X = " << earth_geo.x() << std::endl;
    std::cout << "  Y = " << earth_geo.y() << std::endl;
    std::cout << "  Z = " << earth_geo.z() << std::endl;
    std::cout << "  Distance = " << earth_geo.distance() << " (should be ~0)\n" << std::endl;

    // Get Mars position (heliocentric)
    auto mars_helio = ephemeris::mars_heliocentric(jd);
    std::cout << std::setprecision(6);
    std::cout << "Mars (Heliocentric EclipticMeanJ2000):" << std::endl;
    std::cout << "  X = " << mars_helio.x() << std::endl;
    std::cout << "  Y = " << mars_helio.y() << std::endl;
    std::cout << "  Z = " << mars_helio.z() << std::endl;
    std::cout << "  Distance = " << mars_helio.distance() << "\n" << std::endl;

    // Transform Mars to geocentric
    auto mars_geo = mars_helio.to_center<Geocentric>(jd);
    std::cout << "Mars (Geocentric EclipticMeanJ2000) - as seen from Earth:" << std::endl;
    std::cout << "  X = " << mars_geo.x() << std::endl;
    std::cout << "  Y = " << mars_geo.y() << std::endl;
    std::cout << "  Z = " << mars_geo.z() << std::endl;
    std::cout << "  Distance = " << mars_geo.distance() << "\n" << std::endl;

    // =========================================================================
    // 3. Combined Transformations (center + frame)
    // =========================================================================
    std::cout << "3. COMBINED TRANSFORMATIONS" << std::endl;
    std::cout << "---------------------------" << std::endl;

    // Mars: Heliocentric EclipticMeanJ2000 → Geocentric EquatorialMeanJ2000
    std::cout << "Mars transformation chain:" << std::endl;
    std::cout << "  Start: Heliocentric EclipticMeanJ2000" << std::endl;

    // Method 1: Step by step
    auto mars_helio_equ = mars_helio.to_frame<EquatorialMeanJ2000>(jd);
    std::cout << "  Step 1: Transform frame -> Heliocentric EquatorialMeanJ2000" << std::endl;

    auto mars_geo_equ = mars_helio_equ.to_center<Geocentric>(jd);
    std::cout << "  Step 2: Transform center -> Geocentric EquatorialMeanJ2000" << std::endl;
    std::cout << "  Result:" << std::endl;
    std::cout << "    X = " << mars_geo_equ.x() << std::endl;
    std::cout << "    Y = " << mars_geo_equ.y() << std::endl;
    std::cout << "    Z = " << mars_geo_equ.z() << "\n" << std::endl;

    // Method 2: Using transform (does both)
    auto mars_geo_equ_direct = mars_helio.transform<Geocentric, EquatorialMeanJ2000>(jd);
    std::cout << "  Or using .transform<C,F>(jd) directly:" << std::endl;
    std::cout << "    X = " << mars_geo_equ_direct.x() << std::endl;
    std::cout << "    Y = " << mars_geo_equ_direct.y() << std::endl;
    std::cout << "    Z = " << mars_geo_equ_direct.z() << "\n" << std::endl;

    // =========================================================================
    // 4. Barycentric Coordinates
    // =========================================================================
    std::cout << "4. BARYCENTRIC COORDINATES" << std::endl;
    std::cout << "--------------------------" << std::endl;

    // Get Earth in barycentric coordinates
    auto earth_bary = ephemeris::earth_barycentric(jd);
    std::cout << "Earth (Barycentric EclipticMeanJ2000):" << std::endl;
    std::cout << "  X = " << earth_bary.x() << std::endl;
    std::cout << "  Y = " << earth_bary.y() << std::endl;
    std::cout << "  Z = " << earth_bary.z() << std::endl;
    std::cout << "  Distance from SSB = " << earth_bary.distance() << "\n" << std::endl;

    // Transform to geocentric
    auto earth_geo_from_bary = earth_bary.to_center<Geocentric>(jd);
    std::cout << std::setprecision(10);
    std::cout << "Earth (Geocentric, from Barycentric):" << std::endl;
    std::cout << "  Distance = " << earth_geo_from_bary.distance()
              << " (should be ~0)\n" << std::endl;

    // Transform Mars from barycentric to geocentric
    auto mars_bary = ephemeris::mars_barycentric(jd);
    auto mars_geo_from_bary = mars_bary.to_center<Geocentric>(jd);
    std::cout << std::setprecision(6);
    std::cout << "Mars (Geocentric, from Barycentric):" << std::endl;
    std::cout << "  X = " << mars_geo_from_bary.x() << std::endl;
    std::cout << "  Y = " << mars_geo_from_bary.y() << std::endl;
    std::cout << "  Z = " << mars_geo_from_bary.z() << std::endl;
    std::cout << "  Distance = " << mars_geo_from_bary.distance() << "\n" << std::endl;

    // =========================================================================
    // 5. ICRS Frame Transformations
    // =========================================================================
    std::cout << "5. ICRS FRAME TRANSFORMATIONS" << std::endl;
    std::cout << "-----------------------------" << std::endl;

    // Barycentric ICRS (standard for catalogs)
    cartesian::position::ICRS<AU> star_icrs(100.0, 50.0, 1000.0);
    std::cout << std::setprecision(3);
    std::cout << "Star (Barycentric ICRS):" << std::endl;
    std::cout << "  X = " << star_icrs.x() << std::endl;
    std::cout << "  Y = " << star_icrs.y() << std::endl;
    std::cout << "  Z = " << star_icrs.z() << "\n" << std::endl;

    // Transform to Geocentric ICRS (GCRS)
    auto star_gcrs = star_icrs.to_center<Geocentric>(jd);
    std::cout << "Star (Geocentric ICRS/GCRS):" << std::endl;
    std::cout << "  X = " << star_gcrs.x() << std::endl;
    std::cout << "  Y = " << star_gcrs.y() << std::endl;
    std::cout << "  Z = " << star_gcrs.z() << std::endl;
    std::cout << "  (Difference is tiny for distant stars)\n" << std::endl;

    // =========================================================================
    // 6. Round-trip Transformation
    // =========================================================================
    std::cout << "6. ROUND-TRIP TRANSFORMATION" << std::endl;
    std::cout << "----------------------------" << std::endl;

    std::cout << std::setprecision(10);
    std::cout << "Original Mars (Heliocentric EclipticMeanJ2000):" << std::endl;
    std::cout << "  X = " << mars_helio.x() << std::endl;
    std::cout << "  Y = " << mars_helio.y() << std::endl;
    std::cout << "  Z = " << mars_helio.z() << "\n" << std::endl;

    // Transform: Helio Ecl → Geo EquatorialMeanJ2000 → Helio Ecl
    auto temp = mars_helio.transform<Geocentric, EquatorialMeanJ2000>(jd);
    auto recovered = temp.transform<Heliocentric, EclipticMeanJ2000>(jd);

    std::cout << "After round-trip transformation:" << std::endl;
    std::cout << "  X = " << recovered.x() << std::endl;
    std::cout << "  Y = " << recovered.y() << std::endl;
    std::cout << "  Z = " << recovered.z() << "\n" << std::endl;

    double diff_x = std::abs(mars_helio.x().value() - recovered.x().value());
    double diff_y = std::abs(mars_helio.y().value() - recovered.y().value());
    double diff_z = std::abs(mars_helio.z().value() - recovered.z().value());
    std::cout << std::scientific << std::setprecision(3);
    std::cout << "Differences (should be tiny):" << std::endl;
    std::cout << "  dX = " << diff_x << std::endl;
    std::cout << "  dY = " << diff_y << std::endl;
    std::cout << "  dZ = " << diff_z << "\n" << std::endl;

    std::cout << "=== Example Complete ===" << std::endl;

    return 0;
}