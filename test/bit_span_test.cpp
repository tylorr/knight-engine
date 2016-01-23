#include "bit_span.h"

#include <catch.hpp>

#include <limits>

using knight::BitSpan;

TEST_CASE("BitSpan") {
  SECTION("Empty bit span") {
    auto bit_span = BitSpan<uint8_t>(nullptr, 0);
    REQUIRE(bit_span.size() == 0);
    REQUIRE(bit_span.empty());
    REQUIRE(bit_span.all());
  }

  SECTION("Using all bits of blocks") {
    constexpr int kBlockCount = 2;
    constexpr int kBitCount = kBlockCount * std::numeric_limits<uint8_t>::digits;

    uint8_t blocks[kBlockCount] = {0, 0};
    auto bit_span = BitSpan<uint8_t>(blocks, kBitCount);

    REQUIRE(bit_span.size() == 16);
    REQUIRE(!bit_span.empty());

    SECTION("Test bits") {
      CHECK(!bit_span.test(0));
      blocks[0] = 1;
      CHECK(bit_span.test(0));
    }

    SECTION("Test all bits") {
      CHECK(bit_span.none());
      CHECK(!bit_span.any());
      CHECK(!bit_span.all());

      blocks[0] = 1;
      CHECK(!bit_span.none());
      CHECK(bit_span.any());
      CHECK(!bit_span.all());

      blocks[0] = ~0;
      blocks[1] = ~0;

      CHECK(!bit_span.none());
      CHECK(bit_span.any());
      CHECK(bit_span.all());
    }

    SECTION("Set and reset bits") {
      bit_span.set(0);
      CHECK(bit_span.test(0));

      bit_span.reset(0);
      CHECK(!bit_span.test(0));
    }

    SECTION("Set all bits") {
      bit_span.set();
      CHECK(bit_span.all());
    }

    SECTION("Reset all bits") {
      bit_span.set(0);
      bit_span.set(1);
      bit_span.reset();
      CHECK(bit_span.none());

      bit_span.set();
      bit_span.reset();
      CHECK(bit_span.none());
    }

    SECTION("Flip bits") {
      bit_span.flip(0);
      REQUIRE(bit_span.test(0));
      bit_span.flip(0);
      REQUIRE(!bit_span.test(0));
      bit_span.flip(0);
      REQUIRE(bit_span.test(0));
    }

    SECTION("Flip all bits") {
      bit_span.set(0);
      bit_span.set(2);
      bit_span.set(4);
      bit_span.set(6);
      bit_span.flip();

      CHECK(!bit_span.test(0));
      CHECK(bit_span.test(1));
      CHECK(!bit_span.test(2));
      CHECK(bit_span.test(3));
      CHECK(!bit_span.test(4));
      CHECK(bit_span.test(5));
      CHECK(!bit_span.test(6));
      CHECK(bit_span.test(7));
    }
  } // Span all blocks

  SECTION("Using a subset of bits") {
    constexpr int kBlockCount = 2;
    constexpr int kBitCount = kBlockCount * std::numeric_limits<uint8_t>::digits - 4;

    uint8_t blocks[kBlockCount] = {0, 0};
    auto bit_span = BitSpan<uint8_t>(blocks, kBitCount);

    REQUIRE(bit_span.size() == 12);
    REQUIRE(!bit_span.empty());

    CHECK(bit_span.none());
    CHECK(!bit_span.any());
    CHECK(!bit_span.all());

    bit_span.set(0);

    CHECK(!bit_span.none());
    CHECK(bit_span.any());
    CHECK(!bit_span.all());

    bit_span.set();

    CHECK(!bit_span.none());
    CHECK(bit_span.any());
    CHECK(bit_span.all());
  }

  SECTION("Subspan") {
    constexpr int kBlockCount = 2;
    constexpr int kBitCount = kBlockCount * std::numeric_limits<uint8_t>::digits;

    uint8_t blocks[kBlockCount] = {0, 0};
    auto bit_span = BitSpan<uint8_t>(blocks, 16);

    bit_span.set(0);
    bit_span.set(1);

    auto front_span = bit_span.subspan(0, 8);
    CHECK(front_span.size() == 8);

    CHECK(front_span.test(0));
    CHECK(front_span.test(1));
    CHECK(!front_span.test(2));
    CHECK(!front_span.test(3));

    auto back_span = bit_span.subspan(1, 8);
    CHECK(back_span.size() == 8);
    CHECK(back_span.none());
  }
}
