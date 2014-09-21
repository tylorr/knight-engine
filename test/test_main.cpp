#include "color_formatter.h"
#include "cout_flush.h"

#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

#include <logog.hpp>

int main(int argc, char **argv) {
  int result;

  LOGOG_INITIALIZE();
  {
    logog::CoutFlush out;
    logog::ColorFormatter formatter;
    out.SetFormatter(formatter);

    result = Catch::Session().run( argc, argv );
  }
  LOGOG_SHUTDOWN();

  return result;
}
