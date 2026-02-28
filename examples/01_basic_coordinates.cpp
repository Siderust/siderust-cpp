// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Vallés Puig, Ramon

/// Basic Coordinates Example
///
/// Build with: cmake --build build-local --target basic_coordinates_example
/// Run with: ./build-local/basic_coordinates_example

#include <siderust/siderust.hpp>

#include <cstdio>
#include <iomanip>
#include <iostream>

using namespace siderust;
using namespace siderust::frames;
using namespace siderust::centers;
using namespace qtty::literals;

int main() {
    std::cout << std::fixed;
    std::cout << "=== Siderust Basic Coordinates Example ===" << std::endl
              << std::endl;

    // =========================================================================
    // 1. Cartesian Coordinates
    // =========================================================================
    std::cout << "1. CARTESIAN COORDINATES" << std::endl;
    std::cout << "------------------------" << std::endl;

    // Create a heliocentric ecliptic position (1 AU along X-axis)
    cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit> earth_position(
        1.0, 0.0, 0.0);
    std::cout << "Earth position (Heliocentric EclipticMeanJ2000):" << std::endl;
    std::cout << std::setprecision(6);
    std::cout << "  X = " << earth_position.x() << std::endl;
    std::cout << "  Y = " << earth_position.y() << std::endl;
    std::cout << "  Z = " << earth_position.z() << std::endl;
    std::cout << "  Distance from Sun = " << earth_position.distance() << std::endl << std::endl;

    // Create a geocentric equatorial position (Moon at ~384,400 km)
    // No pre-defined alias for Geocentric EquatorialMeanJ2000, so use full type
    cartesian::Position<Geocentric, EquatorialMeanJ2000, qtty::Kilometer>
        moon_position(300000.0, 200000.0, 100000.0);
    std::cout << "Moon position (Geocentric EquatorialMeanJ2000):" << std::endl;
    std::cout << std::setprecision(1);
    std::cout << "  X = " << moon_position.x() << std::endl;
    std::cout << "  Y = " << moon_position.y() << std::endl;
    std::cout << "  Z = " << moon_position.z() << std::endl;
    std::cout << "  Distance from Earth = " << moon_position.distance() <<std::endl << std::endl;

    // =========================================================================
    // 2. Spherical Coordinates
    // =========================================================================
    std::cout << "2. SPHERICAL COORDINATES" << std::endl;
    std::cout << "------------------------" << std::endl;

    // Create a star direction (Polaris approximately)
    spherical::direction::EquatorialMeanJ2000 polaris(
        qtty::Degree(37.95), // Right Ascension (converted to degrees)
        qtty::Degree(89.26)  // Declination
    );
    std::cout << "Polaris (Geocentric EquatorialMeanJ2000 Direction):" << std::endl;
    std::cout << std::setprecision(2);
    std::cout << "  Right Ascension = " << polaris.ra() << std::endl;
    std::cout << "  Declination = " << polaris.dec() << std::endl
              << std::endl;

    // Create a position with distance (Betelgeuse at ~500 light-years)
    auto betelgeuse_distance = qtty::LightYear(500.0).to<qtty::AstronomicalUnit>(); // Convert 500 ly to AU
    spherical::position::ICRS<qtty::AstronomicalUnit> betelgeuse(88.79_deg, 7.41_deg, betelgeuse_distance);
    std::cout << "Betelgeuse (Barycentric ICRS Position):" << std::endl;
    std::cout << "  Right Ascension = " << betelgeuse.ra() << std::endl;
    std::cout << "  Declination = " << betelgeuse.dec() << std::endl;
    std::cout << std::setprecision(1);
    std::cout << "  Distance = " << betelgeuse.distance()
              << " (~500 ly)" << std::endl
              << std::endl;

    // =========================================================================
    // 3. Directions (Unit Vectors)
    // =========================================================================
    std::cout << "3. DIRECTIONS (UNIT VECTORS)" << std::endl;
    std::cout << "----------------------------" << std::endl;

    // Directions are unitless (implicit radius = 1) and frame-only (no center)
    // Note: Directions don't carry observer site — they're pure directions
    spherical::direction::Horizontal zenith(
        qtty::Degree(0.0),  // Azimuth (North — doesn't matter for zenith)
        qtty::Degree(90.0)  // Altitude (straight up)
    );
    std::cout << "Zenith direction (Horizontal frame):" << std::endl;
    std::cout << "  Altitude = " << zenith.alt() << std::endl;
    std::cout << "  Azimuth  = " << zenith.az() << std::endl
              << std::endl;

    // Convert direction to position at a specific distance
    // Build a spherical Position from the direction + distance (pure geometry)
    auto cloud_distance = qtty::Kilometer(5000.0);
    spherical::Position<Geocentric, Horizontal, qtty::Kilometer> cloud(
        zenith.az(), zenith.alt(), cloud_distance);
    std::cout << "Cloud at zenith, 5 km altitude (relative to geocenter):" << std::endl;
    std::cout << "  Distance = " << cloud.distance() << std::endl
              << std::endl;

    // =========================================================================
    // 4. Cartesian <-> Spherical Conversion
    // =========================================================================
    std::cout << "4. CARTESIAN <-> SPHERICAL CONVERSION" << std::endl;
    std::cout << "-----------------------------------" << std::endl;

    // Start with cartesian
    // Use Geocentric EquatorialMeanJ2000 (same as Rust example's implied center)
    cartesian::Position<Geocentric, EquatorialMeanJ2000, qtty::AstronomicalUnit>
        cart_pos(0.5, 0.5, 0.707);
    std::cout << "Cartesian position:" << std::endl;
    std::cout << std::setprecision(3);
    std::cout << "  X = " << cart_pos.x() << std::endl;
    std::cout << "  Y = " << cart_pos.y() << std::endl;
    std::cout << "  Z = " << cart_pos.z() << std::endl
              << std::endl;

    // Convert to spherical
    auto sph_pos = cart_pos.to_spherical();
    std::cout << "Converted to Spherical:" << std::endl;
    std::cout << std::setprecision(2);
    std::cout << "  RA  = " << sph_pos.ra() << std::endl;
    std::cout << "  Dec = " << sph_pos.dec() << std::endl;
    std::cout << std::setprecision(3);
    std::cout << "  Distance = " << sph_pos.distance() << std::endl;

    // Convert back to cartesian
    auto cart_pos_back = sph_pos.to_cartesian();
    std::cout << std::endl << "Converted back to Cartesian:" << std::endl;
    std::cout << "  X = " << cart_pos_back.x() << std::endl;
    std::cout << "  Y = " << cart_pos_back.y() << std::endl;
    std::cout << "  Z = " << cart_pos_back.z() << std::endl
              << std::endl;

    // =========================================================================
    // 5. Type Safety
    // =========================================================================
    std::cout << "5. TYPE SAFETY" << std::endl;
    std::cout << "--------------" << std::endl;

    // Different coordinate types are incompatible at compile time
    cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit> helio_pos(
        1.0, 0.0, 0.0);
    cartesian::Position<Geocentric, EquatorialMeanJ2000, qtty::AstronomicalUnit>
        geo_pos(0.0, 1.0, 0.0);

    std::cout << "Type-safe coordinates prevent mixing incompatible systems:"
              << std::endl;
    std::cout << "  Heliocentric EclipticMeanJ2000: " << helio_pos << std::endl;
    std::cout << "  Geocentric EquatorialMeanJ2000: " << geo_pos << std::endl;
    std::cout << std::endl
              << "  Cannot directly compute distance between them!" << std::endl;
    std::cout << "  (Must transform to same center/frame first)" << std::endl
              << std::endl;

    // But operations within the same type are allowed
    cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit> pos1(
        1.0, 0.0, 0.0);
    cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit> pos2(
        1.5, 0.0, 0.0);
    auto distance = pos1.distance_to(pos2);
    std::cout << "Distance between two Heliocentric EclipticMeanJ2000 positions:"
              << std::endl;
    std::cout << "  " << distance << std::endl
              << std::endl;

    // =========================================================================
    // 6. Different Centers and Frames
    // =========================================================================
    std::cout << "6. CENTERS AND FRAMES" << std::endl;
    std::cout << "---------------------" << std::endl;

    std::cout << "Reference Centers:" << std::endl;
    std::cout << "  Barycentric:  " << CenterTraits<Barycentric>::name() << std::endl;
    std::cout << "  Heliocentric: " << CenterTraits<Heliocentric>::name() << std::endl;
    std::cout << "  Geocentric:   " << CenterTraits<Geocentric>::name() << std::endl;
    std::cout << "  Topocentric:  " << CenterTraits<Topocentric>::name() << std::endl;
    std::cout << "  Bodycentric:  " << CenterTraits<Bodycentric>::name() << std::endl
              << std::endl;

    std::cout << "Reference Frames:" << std::endl;
    std::cout << "  EclipticMeanJ2000:   " << FrameTraits<EclipticMeanJ2000>::name()
              << std::endl;
    std::cout << "  EquatorialMeanJ2000: " << FrameTraits<EquatorialMeanJ2000>::name()
              << std::endl;
    std::cout << "  Horizontal: " << FrameTraits<Horizontal>::name() << std::endl;
    std::cout << "  ICRS:       " << FrameTraits<ICRS>::name() << std::endl;
    std::cout << "  ECEF:       " << FrameTraits<ECEF>::name() << std::endl
              << std::endl;

    std::cout << "=== Example Complete ===" << std::endl;
    return 0;
}
