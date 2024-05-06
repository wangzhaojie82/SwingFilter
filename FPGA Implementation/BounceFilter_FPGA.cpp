#include <stdint.h>
#include "MurmurHash3.h"
#include <cmath>
#define KEY_LEN 16

const uint8_t b1 = 4;
const uint8_t b2 = 8;
const uint32_t d = 2;

const uint32_t row = 3;
const uint32_t width = 50000;

const uint32_t num1 = 3;
const uint32_t num2 = 3;

const uint32_t M1 = 80000;
const uint32_t M2 = 160000;
const uint32_t m1 = M1 / b1 + 1;
const uint32_t m2 = M2 / b2 + 1;

const uint32_t random_seed = 51241;

const uint32_t seed1 = 75123;
const uint32_t seed2 = 47612;
const uint32_t seed3 = 31213;

uint32_t B1[M1];
uint32_t B2[M2];

uint32_t C[row][width];

void init() {
    for (unsigned int i = 0; i < M1; i++)
        B1[i] = 0;
    for (unsigned int i = 0; i < M2; ++i)
        B2[i] = 0;
    for (unsigned int i = 0; i < row; ++i)
        for (unsigned int j = 0; j < width; ++j)
            C[i][j] = 0;
}

void update(uint8_t* key, uint8_t* ele) {
    uint32_t hashIndex, hashValue, randomHashValue, opt;
    uint32_t real_counter_index, offset, mask;
    uint32_t counter_value, sign_flag;
    char* XOR = new char[KEY_LEN];
    for (int i = 0; i < KEY_LEN; ++i)
        XOR[i] = key[i] ^ ele[i];
    MurmurHash3_x86_32(XOR, KEY_LEN, random_seed, &randomHashValue);
    // stage1
    hashIndex = randomHashValue % num1;
    MurmurHash3_x86_32(key, KEY_LEN, random_seed + hashIndex, &hashValue);
    hashIndex = hashValue % m1;
    MurmurHash3_x86_32(key, KEY_LEN, hashIndex, &opt);
    opt = opt & 1;
    real_counter_index = (hashIndex * b1) >> 5;
    offset = (hashIndex * b1) & 0x1F;
    mask = ((1 << b1) - 1) << (32 - offset - b1);
    counter_value = (B1[real_counter_index] & mask) >> (32 - offset - b1);
    if (counter_value != (1 << (b1)) - 1 && counter_value != (1 << (b1 - 1)) - 1) {
        sign_flag = counter_value & (1 << (b1 - 1));
        if (opt == 0 && (counter_value == 0))
            counter_value = (1 << (b1 - 1)) + 1;
        else if (opt == 1 && (counter_value == (1 << (b1 - 1)) + 1))
            counter_value = 0;
        else if (opt == 1 && sign_flag == 0)
            counter_value += 1;
        else if (opt == 1 && sign_flag != 0)
            counter_value -= 1;
        else if (opt == 0 && sign_flag == 0)
            counter_value -= 1;
        else if (opt == 0 && sign_flag != 0)
            counter_value += 1;
        B1[real_counter_index] = B1[real_counter_index] & ~mask;
        B1[real_counter_index] = B1[real_counter_index] | (counter_value << (32 - offset - b1));
        return;
    }
    //stage2
    hashIndex = randomHashValue % num2;
    MurmurHash3_x86_32(key, KEY_LEN, random_seed + hashIndex, &hashValue);
    hashIndex = hashValue % m2;
    MurmurHash3_x86_32(key, KEY_LEN, hashIndex, &opt);
    opt = opt & 1;
    real_counter_index = (hashIndex * b2) >> 5;
    offset = (hashIndex * b2) & 0x1F;
    mask = ((1 << b2) - 1) << (32 - offset - b2);
    counter_value = (B2[real_counter_index] & mask) >> (32 - offset - b2);
    if (counter_value != (1 << (b2)) - 1 && counter_value != (1 << (b2 - 1)) - 1) {
        sign_flag = counter_value & (1 << (b2 - 1));
        if (opt == 0 && (counter_value == 0))
            counter_value = (1 << (b2 - 1)) + 1;
        else if (opt == 1 && (counter_value == (1 << (b2 - 1)) + 1))
            counter_value = 0;
        else if (opt == 1 && sign_flag == 0)
            counter_value += 1;
        else if (opt == 1 && sign_flag != 0)
            counter_value -= 1;
        else if (opt == 0 && sign_flag == 0)
            counter_value -= 1;
        else if (opt == 0 && sign_flag != 0)
            counter_value += 1;
        B2[real_counter_index] = B2[real_counter_index] & ~mask;
        B2[real_counter_index] = B2[real_counter_index] | (counter_value << (32 - offset - b2));
        return;
    }
    // stage3 sketch
    MurmurHash3_x86_32(key, KEY_LEN, seed1, &hashValue);
    hashIndex = hashValue % width;
    C[0][hashIndex] = C[0][hashIndex] + 1;

    MurmurHash3_x86_32(key, KEY_LEN, seed2, &hashValue);
    hashIndex = hashValue % width;
    C[1][hashIndex] = C[1][hashIndex] + 1;

    MurmurHash3_x86_32(key, KEY_LEN, seed3, &hashValue);
    hashIndex = hashValue % width;
    C[2][hashIndex] = C[2][hashIndex] + 1;
}

int estimate(uint8_t* key) {
    uint32_t min_value = 0x7fffffff;
    uint32_t hashValue, hashIndex, opt;
    uint32_t real_counter_index, offset, mask;
    uint32_t counter_value;
    int counter_value2, sum_value, T, estimated_size = 0;
    sum_value = 0;
    // stage1
    for (int j = 0; j < num1; ++j) {
        MurmurHash3_x86_32(key, KEY_LEN, random_seed + j, &hashValue);
        hashIndex = hashValue % m1;
        MurmurHash3_x86_32(key, KEY_LEN, hashIndex, &opt);
        opt = opt % 2;
        real_counter_index = (hashIndex * b1) >> 5;
        offset = (hashIndex * b1) % 32;
        mask = ((1 << b1) - 1) << (32 - offset - b1);
        counter_value = (B1[real_counter_index] & mask) >> (32 - offset - b1);
        if ((counter_value & (1 << (b1 - 1))) != 0)
            counter_value2 = - 1 * (counter_value & ((1 << (b1 - 1)) - 1));
        else
            counter_value2 = counter_value;
        if (opt == 0)
            sum_value += - 1 * counter_value2;
        else
            sum_value += counter_value2;
    }
    T = num1 * ((1 << (b1 - 1)) - 1);
    estimated_size += sum_value;
    if (sum_value < T)
        return estimated_size;
    // stage2
    for (int j = 0; j < num2; ++j) {
        MurmurHash3_x86_32(key, KEY_LEN, random_seed + j, &hashValue);
        hashIndex = hashValue % m2;
        MurmurHash3_x86_32(key, KEY_LEN, hashIndex, &opt);
        opt = opt % 2;
        real_counter_index = (hashIndex * b2) >> 5;
        offset = (hashIndex * b2) % 32;
        mask = ((1 << b2) - 1) << (32 - offset - b2);
        counter_value = (B2[real_counter_index] & mask) >> (32 - offset - b2);
        if ((counter_value & (1 << (b2 - 1))) != 0)
            counter_value2 = - 1 * (counter_value & ((1 << (b2 - 1)) - 1));
        else
            counter_value2 = counter_value;
        if (opt == 0)
            sum_value += - 1 * counter_value2;
        else
            sum_value += counter_value2;
    }
    T = num2 * ((1 << (b2 - 1)) - 1);
    estimated_size += sum_value;
    if (sum_value < T)
        return estimated_size;
    // sketch stage
    MurmurHash3_x86_32(key, KEY_LEN, seed1, &hashValue);
    hashIndex = hashValue % width;
    if (C[0][hashValue] < min_value)
        min_value = C[0][hashValue];

    MurmurHash3_x86_32(key, KEY_LEN, seed2, &hashValue);
    hashIndex = hashValue % width;
    if (C[1][hashValue] < min_value)
        min_value = C[1][hashValue];

    MurmurHash3_x86_32(key, KEY_LEN, seed3, &hashValue);
    hashIndex = hashValue % width;
    if (C[2][hashValue] < min_value)
        min_value = C[2][hashValue];

    estimated_size = estimated_size + min_value;
    return estimated_size;
}