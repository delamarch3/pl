#pragma once

#define box(x)                                                                                     \
    ({                                                                                             \
        auto v = x;                                                                                \
        auto ptr = malloc(sizeof(v));                                                              \
        if (ptr == nullptr) {                                                                      \
            fprintf(stderr, "malloc failed");                                                      \
            exit(1);                                                                               \
        }                                                                                          \
        memcpy(ptr, &v, sizeof(v));                                                                \
        ptr;                                                                                       \
    })

#define panic(...)                                                                                 \
    printf(__VA_ARGS__);                                                                           \
    exit(1);

#define todo(msg) panic("TODO: " msg "\n");

#define min(x, y) x < y ? x : y
