# Create a file with 1000 lines, each containing a single zero
with open("zeros.txt", "w") as file:
    for _ in range(100000):
        file.write("0\n")

print("File 'zeros.txt' created with 1000 lines of zeroes.")

