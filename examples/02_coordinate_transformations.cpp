/**
 * @file 02_coordinate_transformations.cpp
 * @brief C++ port of siderust/examples/02_coordinate_transformations.rs
 */

#include <cmath>
#include <iomanip>
#include <iostream>

#include <siderust/siderust.hpp>

using namespace siderust;
using namespace qtty::literals;


int main() {
  using AU = qtty::AstronomicalUnit;

  std::cout << "=== Coordinate Transformations (C++) ===\n\n";

  const JulianDate jd = JulianDate::J2000();
  std::cout << "Reference time: J2000.0 (JD " << jd << ")\n\n";

  // 1. Frame transformations (same center)
  std::cout << "1. FRAME TRANSFORMATIONS\n";
  std::cout << "------------------------\n";

  cartesian::position::EclipticMeanJ2000<AU> pos_ecl(1.0, 0.0, 0.0);
  std::cout << "Original (Heliocentric EclipticMeanJ2000):\n";
  std::cout << "  X = " << pos_ecl.x() << "\n";
  std::cout << "  Y = " << pos_ecl.y() << "\n";
  std::cout << "  Z = " << pos_ecl.z() << "\n\n";

  // to Equatorial (same heliocentric center
  auto pos_equ = pos_ecl.to_frame<frames::EquatorialMeanJ2000>(jd);
  std::cout << "Transformed to EquatorialMeanJ2000 frame:\n";
  std::cout << "  X = " << pos_equ.x() << "\n";
  std::cout << "  Y = " << pos_equ.y() << "\n";
  std::cout << "  Z = " << pos_equ.z() << "\n\n";

  // to ICRS — direct to_frame on cartesian::Position (no intermediate spherical)
  auto pos_icrs = pos_equ.to_frame<frames::ICRS>(jd);
  std::cout << "Transformed to ICRS frame:\n";
  std::cout << "  X = " << pos_icrs.x() << "\n";
  std::cout << "  Y = " << pos_icrs.y() << "\n";
  std::cout << "  Z = " << pos_icrs.z() << "\n\n";

  // 2. Center transformations (same frame)
  std::cout << "2. CENTER TRANSFORMATIONS\n";
  std::cout << "-------------------------\n";

  auto earth_helio = ephemeris::earth_heliocentric(jd);
  std::cout << "Earth (Heliocentric EclipticMeanJ2000):\n";
  std::cout << "  X = " << earth_helio.x() << "\n";
  std::cout << "  Y = " << earth_helio.y() << "\n";
  std::cout << "  Z = " << earth_helio.z() << "\n";
  std::cout << "  Distance = " << earth_helio.distance() << "\n\n";

  // Earth in geocentric (origin) — heliocentric minus itself -> zero
  cartesian::Position<centers::Geocentric, frames::EclipticMeanJ2000, AU>
      earth_geo(AU(0.0), AU(0.0), AU(0.0));
  std::cout << "Earth (Geocentric EclipticMeanJ2000) - at origin:\n";
  std::cout << "  X = " << earth_geo.x() << "\n";
  std::cout << "  Y = " << earth_geo.y() << "\n";
  std::cout << "  Z = " << earth_geo.z() << "\n\n";
  std::cout << "  Distance = " << earth_geo.distance() << " AU (should be ~0)\n\n";

  auto mars_helio = ephemeris::mars_heliocentric(jd);
  std::cout << "Mars (Heliocentric EclipticMeanJ2000):\n";
  std::cout << "  X = " << mars_helio.x() << "\n";
  std::cout << "  Y = " << mars_helio.y() << "\n";
  std::cout << "  Z = " << mars_helio.z() << "\n";
  std::cout << "  Distance = " << mars_helio.distance() << "\n\n";

  // Mars geocentric = Mars_helio - Earth_helio (component-wise)
  cartesian::Position<centers::Geocentric, frames::EclipticMeanJ2000, AU>
      mars_geo(mars_helio.x() - earth_helio.x(), mars_helio.y() - earth_helio.y(),
               mars_helio.z() - earth_helio.z());
  std::cout << "Mars (Geocentric EclipticMeanJ2000) - as seen from Earth:\n";
  std::cout << "  X = " << mars_geo.x() << "\n";
  std::cout << "  Y = " << mars_geo.y() << "\n";
  std::cout << "  Z = " << mars_geo.z() << "\n";
  std::cout << "  Distance = " << mars_geo.distance() << "\n\n";

  // 3. Combined transformations (center + frame)
  std::cout << "3. COMBINED TRANSFORMATIONS\n";
  std::cout << "---------------------------\n";

  std::cout << "Mars transformation chain:\n";
  std::cout << "  Start: Heliocentric EclipticMeanJ2000\n";

  // Step 1: transform Mars frame directly (cartesian::Position::to_frame)
  auto mars_helio_equ = mars_helio.to_frame<frames::EquatorialMeanJ2000>(jd);
  std::cout << "  Step 1: Transform frame → Heliocentric EquatorialMeanJ2000\n";

  // Step 2: convert center heliocentric -> geocentric by subtracting Earth's pos in the same frame
  auto earth_helio_equ = earth_helio.to_frame<frames::EquatorialMeanJ2000>(jd);

  cartesian::Position<centers::Geocentric, frames::EquatorialMeanJ2000, AU>
      mars_geo_equ(mars_helio_equ.x() - earth_helio_equ.x(),
                   mars_helio_equ.y() - earth_helio_equ.y(),
                   mars_helio_equ.z() - earth_helio_equ.z());

  std::cout << "  Step 2: Transform center → Geocentric EquatorialMeanJ2000\n";
  std::cout << "  Result:\n";
  std::cout << "    X = " << mars_geo_equ.x() << " AU\n";
  std::cout << "    Y = " << mars_geo_equ.y() << " AU\n";
  std::cout << "    Z = " << mars_geo_equ.z() << " AU\n\n";

  // Method 2: same in one chain — now trivial with direct to_frame
  auto Mars_geo_equ_direct = cartesian::Position<centers::Geocentric, frames::EquatorialMeanJ2000, AU>(
      mars_helio_equ.x() - earth_helio_equ.x(),
      mars_helio_equ.y() - earth_helio_equ.y(),
      mars_helio_equ.z() - earth_helio_equ.z());

  std::cout << "  Or using direct chain (same result):\n";
  std::cout << "    X = " << Mars_geo_equ_direct.x() << "\n";
  std::cout << "    Y = " << Mars_geo_equ_direct.y() << "\n";
  std::cout << "    Z = " << Mars_geo_equ_direct.z() << "\n\n";

  // 4. Barycentric coordinates
  std::cout << "4. BARYCENTRIC COORDINATES\n";
  std::cout << "--------------------------\n";

  auto earth_bary = ephemeris::earth_barycentric(jd);
  std::cout << "Earth (Barycentric EclipticMeanJ2000):\n";
  std::cout << "  X = " << earth_bary.x() << "\n";
  std::cout << "  Y = " << earth_bary.y() << "\n";
  std::cout << "  Z = " << earth_bary.z() << "\n";
  std::cout << "  Distance from SSB = " << earth_bary.distance() << "\n\n";

  // Mars barycentric = sun_barycentric + mars_helio
  auto sun_bary = ephemeris::sun_barycentric(jd);
  cartesian::Position<centers::Barycentric, frames::EclipticMeanJ2000, AU>
      mars_bary(sun_bary.x() + mars_helio.x(), sun_bary.y() + mars_helio.y(),
                sun_bary.z() + mars_helio.z());

  // Transform to geocentric (barycentric -> geocentric = target_bary - earth_bary)
  auto mars_geo_from_bary = cartesian::Position<centers::Geocentric, frames::EclipticMeanJ2000, AU>(
      mars_bary.x() - earth_bary.x(), mars_bary.y() - earth_bary.y(),
      mars_bary.z() - earth_bary.z());

  std::cout << "Mars (Geocentric, from Barycentric):\n";
  std::cout << "  X = " << mars_geo_from_bary.x() << "\n";
  std::cout << "  Y = " << mars_geo_from_bary.y() << "\n";
  std::cout << "  Z = " << mars_geo_from_bary.z() << "\n";
  std::cout << "  Distance = " << mars_geo_from_bary.distance() << "\n\n";

  // 5. ICRS frame transformations (barycentric -> geocentric)
  std::cout << "5. ICRS FRAME TRANSFORMATIONS\n";
  std::cout << "-----------------------------\n";

  // Create a sample star in barycentric ICRS cartesian coords
  cartesian::Position<centers::Barycentric, frames::ICRS, AU> star_icrs(AU(100.0), AU(50.0), AU(1000.0));
  std::cout << "Star (Barycentric ICRS):\n";
  std::cout << "  X = " << star_icrs.x() << "\n";
  std::cout << "  Y = " << star_icrs.y() << "\n";
  std::cout << "  Z = " << star_icrs.z() << "\n\n";

  // Convert Earth's barycentric position from EclipticMeanJ2000 -> ICRS directly
  auto earth_bary_icrs = earth_bary.to_frame<frames::ICRS>(jd);

  // Star geocentric ICRS (GCRS-equivalent for this demo)
  auto star_gcrs = cartesian::Position<centers::Geocentric, frames::ICRS, AU>(
      star_icrs.x() - earth_bary_icrs.x(), star_icrs.y() - earth_bary_icrs.y(),
      star_icrs.z() - earth_bary_icrs.z());

  std::cout << "Star (Geocentric ICRS/GCRS):\n";
  std::cout << "  X = " << star_gcrs.x() << "\n";
  std::cout << "  Y = " << star_gcrs.y() << "\n";
  std::cout << "  Z = " << star_gcrs.z() << "\n\n";

  // 6. Round-trip transformation
  std::cout << "6. ROUND-TRIP TRANSFORMATION\n";
  std::cout << "----------------------------\n";

  auto original = mars_helio;
  std::cout << "Original Mars (Heliocentric EclipticMeanJ2000):\n";
  std::cout << "  X = " << original.x() << "\n";
  std::cout << "  Y = " << original.y() << "\n";
  std::cout << "  Z = " << original.z() << "\n\n";

  // Helio Ecl -> Geo Equ -> back to Helio Ecl
  // (we reuse previously computed earth_helio_equ)
  auto temp = mars_geo_equ; // geocentric equ

  // Recover heliocentric equ by adding Earth's heliocentric equ
  auto recovered_helio_equ = cartesian::Position<centers::Heliocentric, frames::EquatorialMeanJ2000, AU>(
      temp.x() + earth_helio_equ.x(), temp.y() + earth_helio_equ.y(), temp.z() + earth_helio_equ.z());

  // Convert recovered heliocentric equ back to heliocentric ecliptic (direct to_frame)
  auto recovered = recovered_helio_equ.to_frame<frames::EclipticMeanJ2000>(jd);

  std::cout << "After round-trip transformation:\n";
  std::cout << "  X = " << recovered.x() << "\n";
  std::cout << "  Y = " << recovered.y() << "\n";
  std::cout << "  Z = " << recovered.z() << "\n\n";

  const double diff_x = std::abs(original.x().value() - recovered.x().value());
  const double diff_y = std::abs(original.y().value() - recovered.y().value());
  const double diff_z = std::abs(original.z().value() - recovered.z().value());
  std::cout << "Differences (should be tiny):\n";
  std::cout << "  \u0394X = " << diff_x << "\n";
  std::cout << "  \u0394Y = " << diff_y << "\n";
  std::cout << "  \u0394Z = " << diff_z << "\n\n";

  std::cout << "=== Example Complete ===\n";
  return 0;
}
