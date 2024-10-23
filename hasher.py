import sys

mod_val = 65521


def main():
    args = sys.argv

    if len(args) < 2:
        print("Please provide a function name")
        exit(1)

    hash = adler32(args[1])

    print(hex(hash))


def adler32(func_str):
    A = 1
    B = 0

    for char in str(func_str):
        A = (A + ord(char)) % mod_val
        B = (B + A) % mod_val

    return (B << 16) + A


if __name__ == "__main__":
    main()
