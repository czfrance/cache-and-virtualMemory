#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char memory[16777216*2];

struct frame {
    int valid = 0;
    int tag;
    char data[2048];
    int last_use = 0; //when not used, subtract one. When used, set to 0
};

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
        else {
            break;
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

int getlower(int num, int n) {
    return (num & ones(n));
}

int dellower(int num, int n) {
    return (num >> n);
}

void printmem(int a, int b) {
    for (int i = a; i < b; i++) {
        printf("%c", memory[i]);
    }
    printf("\n");
}


int main(int argc, char *argv[]) {
    if (argc != 5) {
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

        for (int i = 0; i < 16777216*2; i++) {
            memory[i] = '0';
        }

        char *a = argv[2];
        int cache_sz_kB = atoi(a);
        char *b = argv[3];
        int assoc = atoi(b);
        char *c = argv[4];
        int kB = pow(2, 10);

        int blk_sz = atoi(c);
        int cache_sz = cache_sz_kB * kB;
        int num_frames = cache_sz / blk_sz;
        int num_sets = num_frames / assoc;
        int index_sz = log2(num_sets);
        int offset_sz = log2(blk_sz);
        int tag_sz = 24-index_sz - offset_sz;


        frame* cache[num_sets][assoc];

        for (int i = 0; i < num_sets; i++) {
            for (int j = 0; j < assoc; j++) {
                cache[i][j] = (frame*)malloc(sizeof(frame));
            }
        }

        char insn[9];
        char addr_hex[9];
        int bytes;
        char result[5];


        while (true) {
            if (fscanf(file, "%s", insn) == EOF) {
                break;
            }
            else if (strcmp(insn, "store") == 0) {
                fscanf(file, "%s", addr_hex);
                fscanf(file, "%d", &bytes);
                char dataIn[(bytes*2) + 1];
                fscanf(file, "%s", dataIn);
                int addr = to_decimal(addr_hex);
                int offset = getlower(addr, offset_sz);
                int index = getlower(dellower(addr, offset_sz), index_sz);
                int tag = dellower(addr, offset_sz+index_sz);

                //add to memory
                for (int i = 0; i < (bytes*2); i++) {
                    memory[(addr*2)+i] = dataIn[i];
                }

                bool hit = false;
                int way;
                for (int i = 0; i < assoc; i++) {
                    if ((cache[index][i]->tag == tag) && cache[index][i]->valid) {
                        way = i;
                        hit = true;
                        break;
                    }
                }
                if (hit) {
                    strcpy(result, "hit");
                    for (int i = 0; i < (bytes*2); i++) {
                        cache[index][way]->data[(offset*2)+i] = dataIn[i];
                    }
                    cache[index][way]->last_use = 0;
                    for (int i = 0; i < assoc; i++) {
                        if (i != way) {
                            cache[index][i]->last_use = cache[index][i]->last_use - 1;
                        }
                    }
                }
                else {
                    strcpy(result, "miss");
                }
                printf("%s %s %s\n", insn, addr_hex, result);
            }
            else { //insn is load
                fscanf(file, "%s", addr_hex);
                fscanf(file, "%d", &bytes);
                int addr = to_decimal(addr_hex);
                int offset = getlower(addr, offset_sz);
                int index = getlower(dellower(addr, offset_sz), index_sz);
                int tag = dellower(addr, offset_sz+index_sz);

                char dataOut[(bytes*2) + 1];

                bool hit = false;
                int way;
                for (int i = 0; i < assoc; i++) {
                    if ((cache[index][i]->tag == tag) && cache[index][i]->valid) {
                        way = i;
                        hit = true;
                        break;
                    }
                }

                if (hit) {
                    strcpy(result, "hit");
                    for (int i = 0; i < (bytes*2); i++) {
                        dataOut[i] = cache[index][way]->data[(offset*2)+i];
                    }
                    dataOut[bytes*2] = '\0';
                    cache[index][way]->last_use = 0;
                    for (int i = 0; i < assoc; i++) {
                        if (i != way) {
                            cache[index][i]->last_use = cache[index][i]->last_use - 1;
                        }
                    }
                }
                else {
                    strcpy(result, "miss");

                    //find cache with smallest last_use
                    int farthest=0;
                    int farthest_count = 2147483647;
                    for (int i = 0; i < assoc; i++) {
                        if (cache[index][i]->valid == 0) {
                            farthest = i;
                            break;
                        }
                        if (cache[index][i]->last_use < farthest_count) {
                            farthest = i;
                            farthest_count = cache[index][i]->last_use;
                        }
                    }
                    //replace
                    cache[index][farthest]->valid = 1;
                    cache[index][farthest]->tag = tag;
                    cache[index][farthest]->last_use = 0;
                    int blk_addr = dellower(addr, offset_sz);
                    blk_addr = blk_addr << offset_sz;
                    for (int i = 0; i < (blk_sz*2); i++) {
                        cache[index][farthest]->data[i] = memory[(blk_addr*2)+i];
                    }

                    for (int i = 0; i < (bytes*2); i++) {
                        dataOut[i] = cache[index][farthest]->data[(offset*2)+i];
                    }
                    dataOut[bytes*2] = '\0';

                    for (int i = 0; i < assoc; i++) {
                        if (i != farthest) {
                            cache[index][i]->last_use = cache[index][i]->last_use - 1;
                        }
                    }
                    
                }

                printf("%s %s %s %s\n", insn, addr_hex, result, dataOut);
            }
        }

        fclose(file);
        
        return EXIT_SUCCESS;
    }
}