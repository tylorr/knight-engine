#pragma once

#include <logog.hpp>

namespace logog {

class CoutFlush : public Target
{
  virtual int Output(const LOGOG_STRING &data) {
    LOGOG_COUT << (const LOGOG_CHAR *)data << std::flush;

    return 0;
  }
};

}
