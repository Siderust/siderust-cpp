\page coordinate_types Coordinate types

This page is a quick reference to the coordinate types exposed by `siderust-cpp`,
and the main methods you can expect each type to provide.

## Includes

- Prefer `#include <siderust/coordinates.hpp>` (or `#include <siderust/siderust.hpp>`).
- Common aliases live in `#include <siderust/coordinates/types.hpp>`.
- For selective aliases, include only what you need, e.g.:
  - `#include <siderust/coordinates/types/spherical/direction/equatorial.hpp>`
  - `#include <siderust/coordinates/types/spherical/direction/ecliptic.hpp>`
  - `#include <siderust/coordinates/types/spherical/position/horizontal.hpp>`
  - `#include <siderust/coordinates/types/cartesian/position/geodetic.hpp>`

## Primary coordinate types

### `siderust::Geodetic`

WGS84 geodetic position (fixed to center = `centers::Geocentric`, frame = `frames::ECEF`).

- Fields: `lon`, `lat`, `height`
- Methods: `to_c()`, `from_c(...)`, `to_cartesian<U>()`

### `siderust::spherical::Direction<F>`

Spherical direction tagged by reference frame `F`.

- Internal components are not exposed as public fields.
- Methods: `to_c()`, `from_c(...)`
- Frame-dependent convenience accessors:
  - Equatorial frames (`frames::has_ra_dec_v<F>`): `ra()`, `dec()`
  - Horizontal (`frames::has_az_alt_v<F>`): `az()`, `al()` / `alt()`
  - Lon/lat frames (`frames::has_lon_lat_v<F>`): `lon()`, `lat()`
- Transforms:
  - `to_frame<Target>(jd)` / `to<Target>(jd)` (only when `frames::has_frame_transform_v<F, Target>`)
  - `to_horizontal(jd, observer)` (only when `frames::has_horizontal_transform_v<F>`)

### `siderust::spherical::Position<C, F, U>`

Spherical position (direction + distance) tagged by center `C`, frame `F`, and unit `U`.

- Internal components are not exposed as public fields.
- Methods: `direction()`, `distance()`
- Frame-dependent convenience accessors:
  - Equatorial frames (`frames::has_ra_dec_v<F>`): `ra()`, `dec()`
  - Horizontal (`frames::has_az_alt_v<F>`): `az()`, `al()` / `alt()`
  - Lon/lat frames (`frames::has_lon_lat_v<F>`): `lon()`, `lat()`

### `siderust::cartesian::Direction<F>`

Cartesian direction vector tagged by reference frame `F`.

- Fields: `x`, `y`, `z`
- Methods: `frame_id()`

### `siderust::cartesian::Position<C, F, U>`

Cartesian position tagged by center `C`, frame `F`, and unit `U`.

- Fields: `comp_x`, `comp_y`, `comp_z`
- Methods: `x()`, `y()`, `z()`, `to_c()`, `from_c(...)`, `frame_id()`, `center_id()`

## Common aliases (namespace-scoped)

Aliases are grouped by coordinate kind and meaning is encoded in namespace
rather than suffixes. Examples:

- `spherical::direction::ICRS` = `spherical::Direction<frames::ICRS>`
- `spherical::position::ICRS<qtty::Meter>` = `spherical::Position<centers::Barycentric, frames::ICRS, qtty::Meter>`
- `cartesian::position::ECEF<qtty::Meter>` = `cartesian::Position<centers::Geocentric, frames::ECEF, qtty::Meter>`
- `cartesian::position::EclipticMeanJ2000<qtty::AstronomicalUnit>` = `cartesian::Position<centers::Heliocentric, frames::EclipticMeanJ2000, qtty::AstronomicalUnit>`

Browse the full list in the `coordinates/types/` headers.
