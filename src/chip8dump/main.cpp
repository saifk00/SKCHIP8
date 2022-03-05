#include <SKChip8/Utils/ROMLoader.h>

#include <iostream>
#include <string>
#include <fstream>

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " ROM [output file]" << std::endl;
    return 1;
  }

  SKChip8::ROMLoader r(argv[1]);

  auto outputFile = argc >= 3 ? argv[2] : "ch8asm.out";
  std::ofstream result(outputFile);

  result << r.getDisassembly();

  result.close();

  return 0;
}
