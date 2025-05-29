#include <cstdio>
#include <cstdarg>

#include <SDL2/SDL.h>

static int SDL_vsnprintf2(char *str, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int len = SDL_vsnprintf(str, size, format, args);
    va_end(args);
    return len;
}

int main(int argc, char *argv[]) {
    printf("[SDL_snprintf]\n");
    {
        char buffer[2048];
        SDL_snprintf(buffer, sizeof(buffer), "d: %d, ld: %ld, lu: %lu, f: %f, s: %s", 123, 123L, 123UL, 3.14,
                     "Hello, World!");
        printf("%s\n", buffer);
    }
    printf("[SDL_vsnprintf]\n");
    {
        char buffer[2048];
        SDL_vsnprintf2(buffer, sizeof(buffer), "d: %d, ld: %ld, lu: %lu, f: %f, s: %s", 123, 123L, 123UL, 3.14,
                       "Hello, World!");
        printf("%s\n", buffer);
    }
    return 0;
}