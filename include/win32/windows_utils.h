#pragma once

#include "common.h"

#include <string_stream.h>

namespace knight {
namespace windows {

foundation::string_stream::Buffer GetLastErrorMessage();

}
}
