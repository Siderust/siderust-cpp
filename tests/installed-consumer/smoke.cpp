// Minimal smoke test for an installed siderust-cpp consumer.
//
// Verifies that:
//   - find_package(siderust_cpp REQUIRED) resolves
//   - <siderust/siderust.hpp> can be included from the install prefix
//   - linking against siderust::siderust_cpp pulls in libsiderust_ffi
//     (and transitively libtempoch_ffi / libqtty_ffi) so that a trivial
//     runtime call into the FFI succeeds.
//
// If the install layout is broken (missing imported target, missing .so,
// wrong RPATH, unresolved FFI symbol), this binary either fails to link
// or aborts at runtime — both of which fail the CI job.

#include <siderust/siderust.hpp>

#include <cstdio>
#include <cstdlib>

int main() {
  using namespace siderust;
  // J2000.0 = 2000-01-01 12:00:00 UTC (close enough — exact J2000 is TT).
  const auto mjd = Time<TT, MJD>::from_utc({2000, 1, 1, 12, 0, 0});
  const double mjd_value = mjd.value();

  std::printf("siderust-cpp installed-consumer smoke: J2000 MJD = %.6f\n", mjd_value);

  // Sanity check: J2000.0 MJD is approximately 51544.5.
  if (mjd_value < 51540.0 || mjd_value > 51550.0) {
    std::fprintf(stderr, "unexpected MJD: %.6f\n", mjd_value);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
