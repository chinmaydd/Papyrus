#include "Papyrus/Papyrus.h"
#include "Papyrus/Logger/Logger.h"

structlog LOGCFG = {};

int main(int argc, char *argv[]) {
  // Logger configuration (optional)
  LOGCFG.headers = true;
  LOGCFG.level = DEBUG;
  // Log output
  LOG(INFO) << "Hello, world!";
  return 0;
}
