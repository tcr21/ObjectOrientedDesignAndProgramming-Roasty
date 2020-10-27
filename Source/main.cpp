#include "Roasty.hpp"
#include "Storage/DiskStorage.hpp"

int main() {

  Roasty<DiskStorage> roasty;

  roasty.startServer();
}
