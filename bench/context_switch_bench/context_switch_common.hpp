#include <cstdio>
#include <stdexcept>
#include <string>

#define PADDING_SIZES \
    X(0)              \
    X(1)              \
    X(2)              \
    X(4)              \
    X(8)              \
    X(9)              \
    X(10)             \
    X(11)             \
    X(12)             \
    X(13)             \
    X(14)             \
    X(15)             \
    X(16)             \
    X(20)             \
    X(32)             \
    X(30)             \
    X(40)             \
    X(50)             \
    X(60)             \
    X(64)             \
    X(70)             \
    X(80)             \
    X(90)             \
    X(100)            \
    X(128)            \
    X(200)            \
    X(256)            \
    X(300)            \
    X(400)            \
    X(500)            \
    X(512)            \
    X(600)            \
    X(700)            \
    X(800)            \
    X(900)            \
    X(1000)           \
    X(1024)           \
    X(2000)           \
    X(2048)           \
    X(3000)           \
    X(4000)           \
    X(4096)           \
    X(5000)           \
    X(6000)           \
    X(7000)           \
    X(8000)           \
    X(8192)           \
    X(9000)           \
    X(10000)          \
    X(16384)          \
    X(20000)          \
    X(30000)          \
    X(40000)          \
    X(50000)          \
    X(60000)          \
    X(70000)          \
    X(80000)          \
    X(90000)          \
    X(100000)         \
    X(1000000)        \
    X(10000000)

void run_benchmark(int iterations, int64_t depth, int padding);

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <iterations> <depth> <padding>\n", argv[0]);
        return 1;
    }

    try {
        int iterations = std::stoi(argv[1]);
        int64_t depth = std::stoi(argv[2]);
        int padding = std::stoi(argv[3]);

        run_benchmark(iterations, depth, padding);

    } catch (const std::invalid_argument &exn) {
        printf("Invalid argument in %s\n", exn.what());
        return 1;
    }
}
