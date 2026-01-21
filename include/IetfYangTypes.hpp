#pragma once

#include <cstdint>
#include <string>

namespace yang {

  // Mappings of YANG typedefs (ietf-yang-types) to nearest C++ storage types.
  // Original YANG typedef names are shown in comments; C++ identifiers use
  // underscores instead of hyphens where necessary.

  using counter32 = std::uint32_t;        // YANG: counter32 -> uint32
  using zero_based_counter32 = counter32; // YANG: zero-based-counter32

  using counter64 = std::uint64_t;        // YANG: counter64 -> uint64
  using zero_based_counter64 = counter64; // YANG: zero-based-counter64

  using gauge32 = std::uint32_t; // YANG: gauge32 -> uint32
  using gauge64 = std::uint64_t; // YANG: gauge64 -> uint64

  using object_identifier = std::string; // YANG: object-identifier -> string
  using object_identifier_128 =
      object_identifier; // YANG: object-identifier-128

  using yang_identifier = std::string; // YANG: yang-identifier -> string

  using date_and_time = std::string; // YANG: date-and-time -> string

  using timeticks = std::uint32_t; // YANG: timeticks -> uint32
  using timestamp = timeticks;     // YANG: timestamp -> timeticks

  using phys_address = std::string; // YANG: phys-address -> string
  using mac_address = std::string;  // YANG: mac-address -> string

  using xpath_1_0 = std::string; // YANG: xpath1.0 -> string

  using hex_string = std::string;  // YANG: hex-string -> string
  using uuid_t = std::string;      // YANG: uuid -> string
  using dotted_quad = std::string; // YANG: dotted-quad -> string

} // namespace yang
