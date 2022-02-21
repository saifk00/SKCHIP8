import math

def genBCD(bits):
    print (f"constexpr std::array<uint16_t, {2**bits}> BCD_Table = {{")
    for i in range(0, 2**bits - 1):
        print("0x{},".format(i), end="")
        if (i % 20 == 0 and i > 0):
            print("")

    print("\n}")

if __name__ == "__main__":
    genBCD(8)
