#include <iostream>
#include <iomanip>
#include <vector>
#include "usbdrive.h"

int main(__attribute__((unused)) int argc,
         __attribute__((unused)) char *argv[])
{
  std::vector<USBDrive> drives;
  USBDrive::FindStorageDrives(drives);

  if (drives.empty()) {
    std::cout << "No drives found." << std::endl;
    return EXIT_SUCCESS;
  }

  const std::size_t GIG = 1073741824;
  const std::size_t MEG =    1048576;
  const std::size_t KIL =       1024;

  for (auto d : drives) {
    std::cout << d.getFullname() << std::endl;
    for (auto p : d.getDisk().getPartitions()) {
      std::cout << "  " << p.getName() << '\t';
      auto n = p.getSize();
      if (n >= GIG) {
        auto rem = (p.getSize() % GIG);
        if (rem >= MEG) {
          // we want decimal, we have at least 0.1G rem
          double dbl = ((double)n / (double)GIG);
          std::cout << std::fixed << std::setprecision(1) << dbl << "G";
        }
        else {
          n = (p.getSize() / GIG);
          std::cout << n << "G";
        }
      }
      else if (n >= MEG) {
        auto rem = (p.getSize() % MEG);
        if (rem >= KIL) {
          // we want decimal, we have at least 0.1M rem
          double dbl = ((double)n / (double)MEG);
          std::cout << std::fixed << std::setprecision(1) << dbl << "M";
        }
        else {
          n = (p.getSize() / MEG);
          std::cout << n << "M";
        }
      }
      else if (n >= KIL) {
        n = (p.getSize() / KIL);
        std::cout << n << "K";
      }
      else
        std::cout << n;
      std::cout << std::endl;
    }
  }

  return EXIT_SUCCESS;
}
