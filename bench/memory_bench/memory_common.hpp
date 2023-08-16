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
    X(11000)          \
    X(12000)          \
    X(13000)          \
    X(14000)          \
    X(15000)          \
    X(16000)          \
    X(17000)          \
    X(18000)          \
    X(19000)          \
    X(16384)          \
    X(20000)          \
    X(21000)          \
    X(22000)          \
    X(23000)          \
    X(24000)          \
    X(25000)          \
    X(26000)          \
    X(27000)          \
    X(28000)          \
    X(29000)          \
    X(30000)          \
    X(31000)          \
    X(32000)          \
    X(33000)          \
    X(34000)          \
    X(35000)          \
    X(36000)          \
    X(37000)          \
    X(38000)          \
    X(39000)          \
    X(40000)          \
    X(41000)          \
    X(42000)          \
    X(43000)          \
    X(44000)          \
    X(45000)          \
    X(46000)          \
    X(47000)          \
    X(48000)          \
    X(49000)          \
    X(50000)          \
    X(60000)          \
    X(70000)          \
    X(80000)          \
    X(90000)          \
    X(100000)         \
    X(1000000)        \
    X(10000000)

void run_benchmark(int instances, int64_t depth, int padding);

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <instances> <depth> <padding>\n", argv[0]);
        return 1;
    }

    try {
        int instances = std::stoi(argv[1]);
        int64_t depth = std::stoi(argv[2]);
        int padding = std::stoi(argv[3]);

        run_benchmark(instances, depth, padding);

    } catch (const std::invalid_argument &exn) {
        printf("Invalid argument in %s\n", exn.what());
        return 1;
    }
}
