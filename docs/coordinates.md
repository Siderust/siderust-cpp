\page coordinate_types Coordinate types

This page is a quick reference to the coordinate types exposed by `siderust-cpp`,
and the main methods you can expect each type to provide.

## Includes

- Prefer `#include <siderust/coordinates.hpp>` (or `#include <siderust/siderust.hpp>`).
- Common aliases live in `#include <siderust/coordinates/types.hpp>`.

## Primary coordinate types

### `siderust::Geodetic`

WGS84 geodetic position (fixed to center = `centers::Geocentric`, frame = `frames::ECEF`).

- Fields: `lon`, `lat`, `height`
- Methods: `to_c()`, `from_c(...)`, `to_cartesian<U>()`

### `siderust::spherical::Direction<F>`

Spherical direction tagged by reference frame `F`.

- Fields: `lon`, `lat` (both `qtty::Degree`)
- Always available: `azimuthal()`, `polar()`, `to_c()`, `from_c(...)`
- Frame-dependent convenience accessors:
  - Equatorial frames (`frames::has_ra_dec_v<F>`): `ra()`, `dec()`
  - Horizontal (`frames::has_az_alt_v<F>`): `az()`, `alt()`, `azimuth()`, `altitude()`
  - Lon/lat frames (`frames::has_lon_lat_v<F>`): `longitude()`, `latitude()`
- Transforms:
  - `to_frame<Target>(jd)` / `to<Target>(jd)` (only when `frames::has_frame_transform_v<F, Target>`)
  - `to_horizontal(jd, observer)` (only when `frames::has_horizontal_transform_v<F>`)

### `siderust::spherical::Position<C, F, U>`

Spherical position (direction + distance) tagged by center `C`, frame `F`, and unit `U`.

- Fields: `lon`, `lat`, `dist`
- Methods: `direction()`, `distance()`

### `siderust::cartesian::Direction<F>`

Cartesian direction vector tagged by reference frame `F`.

- Fields: `x`, `y`, `z`
- Methods: `frame_id()`

### `siderust::cartesian::Position<C, F, U>`

Cartesian position tagged by center `C`, frame `F`, and unit `U`.

- Fields: `comp_x`, `comp_y`, `comp_z`
- Methods: `x()`, `y()`, `z()`, `to_c()`, `from_c(...)`, `frame_id()`, `center_id()`

## Common aliases (`siderust::types::*`)

Most code should use the concise aliases in `siderust::types` (also re-exported at
namespace root for backward compatibility). Examples:

- `types::IcrsDir` = `spherical::Direction<frames::ICRS>`
- `types::IcrsPos` = `spherical::Position<centers::Barycentric, frames::ICRS>`
- `types::EcefCartPos` = `cartesian::Position<centers::Geocentric, frames::ECEF>`

Browse the full list in the `coordinates/types.hpp` documentation.

