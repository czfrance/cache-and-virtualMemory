#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int pow(int base, int power) {
    int ret = 1;
    for (int i = 0; i < power; i++) {
        ret = ret*base;
    }
    return ret;
}

int log2(int n) {
    if (n == 0) {
        return 0;
    }
    int ret = 0;
    while (n != 1) {
        n = n >> 1;
        ret++;
    }
    return ret;
}

int ones(int n) {
    return ((1<<n)-1);
}

int to_decimal(char *hex) {
    int l = strlen(hex);
    int power = 0;
    int dec = 0;
    for (int i = l-1; i >= 0; i--) {
        if (hex[i] >= '0' && hex[i] <= '9') {
            dec += (hex[i] - 48) * pow(16, power);
        }
        else if (hex[i] >= 'A' && hex[i] <= 'F') {
            dec += (hex[i] - 55) * pow(16, power);
        }
        else if (hex[i] >= 'a' && hex[i] <= 'f') {
            dec += (hex[i] - 87) * pow(16, power);
        }
        power++;
    }

    return dec;
}

void to_hex(int *dec, int *len, char *hex) {
    int d = *dec;

    if (d == 0) {
        hex[0] = '0';
        hex[1] = '\0';
    }
    else {
        char hex_reverse[6];

        int i = 0;
        int temp = 0;
        int rem = 0;
        while (d != 0) {
            temp = d / 16;
            rem = d - (temp*16);

            if (rem < 10) {
                hex_reverse[i] = rem + 48;
            }
            else {
                hex_reverse[i] = rem + 87;
            }
            i++;
            d = d / 16;
        }

        int count = 0;
        hex[i] = '\0';
        for (int j = i-1; j >= 0; j--) {
            hex[j] = hex_reverse[count];
            count++;
        }
    }
}

int getlower(int *num, int *n) {
    return (*num & ones(*n));
}

int dellower(int *num, int *n) {
    return (*num >> *n);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Syntax: arglen <file> \n");
        return EXIT_SUCCESS;
    }
    else {
        FILE *file;
        file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Was not able to open the file\n");
            return EXIT_SUCCESS;
        }

        char virt_addr_hex[6];
        strcpy(virt_addr_hex, argv[2]);
        int virt_addr = to_decimal(virt_addr_hex);
        int len = strlen(virt_addr_hex);

        int word_size;
        int page_size;
        int ppn;

        if (fscanf(file, "%d", &word_size) == 1 && 
                fscanf(file, "%d", &page_size) == 1) {
            int offset_bits = log2(page_size);
            int vpn_bits = word_size - offset_bits;

            int offset = getlower(&virt_addr, &offset_bits);
            int vpn = dellower(&virt_addr, &offset_bits);

            int counter = 0;
            int ppn;
            for (int i = 0; i <= vpn; i++) {
                fscanf(file, "%d", &ppn);
            }
            if (ppn >= 0) {
                int phys_addr = offset | (ppn << offset_bits);
                char phys_addr_hex[6];
                to_hex(&phys_addr, &len, phys_addr_hex);

                printf("%s\n", phys_addr_hex);
            }
            else {
                printf("PAGEFAULT\n");
            }
        }

        else {
            printf("Failed to read\n");
        }

        fclose(file);
        
        return EXIT_SUCCESS;
    }
}