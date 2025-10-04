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

#define TODO(msg)                                                                                  \
    printf("TODO: " msg "\n");                                                                     \
    exit(1);
