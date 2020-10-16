#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/queue.h>

#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>

void one_spin(const unsigned lcore_id) {
    const uint64_t tsc_cycles_in_one_second = rte_get_tsc_hz();
    const uint64_t tsc_cycles_in_one_ms = rte_get_tsc_hz() / 1000;
    const uint64_t hpet_cycles_in_one_second = rte_get_hpet_hz();
    const uint64_t hpet_cycles_in_one_ms = rte_get_hpet_hz() / 1000;

    const uint64_t tsc_start = rte_get_tsc_cycles();
    const uint64_t hpet_start = rte_get_hpet_cycles();

    u_int64_t tsc_result = 0;
    u_int64_t tsc_hpet_result = 0;
    u_int64_t hpet_result = 0;
    u_int64_t hpet_tsc_result = 0;

    while (true) {

        constexpr uint64_t seconds = 60;

        const uint64_t hpet_now = rte_get_hpet_cycles();
        const uint64_t tsc_now = rte_get_tsc_cycles();

        const bool tsc_done =  tsc_now - tsc_start >= tsc_cycles_in_one_second * seconds;
        const bool hpet_done = hpet_now - hpet_start >= hpet_cycles_in_one_second * seconds;

        if (tsc_done && tsc_result == 0)
        {
            tsc_result = tsc_now;
            tsc_hpet_result = hpet_now;
        }

        if (hpet_done && hpet_result == 0)
        {
            hpet_result = hpet_now;
            hpet_tsc_result = tsc_now;
        }

        if (tsc_done && hpet_done) {
            const uint64_t hpet_missed_cycles = hpet_result - hpet_start - hpet_cycles_in_one_second * seconds;
            const int64_t hpet_tsc_missed_cycles = hpet_tsc_result - tsc_start - tsc_cycles_in_one_second * seconds;

            const uint64_t tsc_missed_cycles = tsc_result - tsc_start - tsc_cycles_in_one_second * seconds;
            const int64_t tsc_hpet_missed_cycles = tsc_hpet_result - hpet_start - hpet_cycles_in_one_second * seconds;

            printf("#%d hpet %8" PRIu64 " %5ldms %8ld %5ldms tsc %8" PRIu64 "%5ldms %8ld %5ldms\n", lcore_id,
                    hpet_missed_cycles, hpet_missed_cycles / hpet_cycles_in_one_ms ,
                    hpet_tsc_missed_cycles, hpet_tsc_missed_cycles / tsc_cycles_in_one_ms,
                    tsc_missed_cycles, tsc_missed_cycles / tsc_cycles_in_one_ms,
                    tsc_hpet_missed_cycles, tsc_hpet_missed_cycles / (int64_t)hpet_cycles_in_one_ms
                    );
            break;
        }

        // Delay function that uses system sleep. Does not block the CPU core. val in microseconds
        rte_delay_us_sleep(10);

        //Blocking delay function. val in microseconds
        //rte_delay_ms(1);
    }
}

static int
lcore_hello(__attribute__((unused)) void *arg) {
    unsigned lcore_id;
    lcore_id = rte_lcore_id();
    printf("hello from core %u\n", lcore_id);

    printf("#%d rte_get_tsc_hz %" PRIu64 "\n", lcore_id, rte_get_tsc_hz());
    printf("#%d rte_get_hpet_hz %" PRIu64 "\n", lcore_id, rte_get_hpet_hz());

    uint64_t hpet_cycles_in_one_second = rte_get_hpet_hz();
    uint64_t tsc_cycles_in_one_second = rte_get_tsc_hz();

    while (true) {
        if (tsc_cycles_in_one_second != rte_get_tsc_hz()) {
            tsc_cycles_in_one_second = rte_get_tsc_hz();
            printf("#%d rte_get_tsc_hz %" PRIu64 "\n", lcore_id, tsc_cycles_in_one_second);
        }
        if (hpet_cycles_in_one_second != rte_get_hpet_hz()) {
            hpet_cycles_in_one_second = rte_get_hpet_hz();
            printf("#%d hpet_cycles_in_one_second %" PRIu64 "\n", lcore_id, hpet_cycles_in_one_second);
        }

        one_spin(lcore_id);
    }

    return 0;
}

int
main(int argc, char **argv) {
    int ret;
    unsigned lcore_id;

    ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_panic("Cannot init EAL\n");

    ret = rte_eal_hpet_init(false);
    if (ret < 0)
        rte_panic("Cannot init HPET\n");


    RTE_LCORE_FOREACH_SLAVE(lcore_id) {
//        if (lcore_id < 4)
        rte_eal_remote_launch(lcore_hello, NULL, lcore_id);
    }

    /* call it on master lcore too */
    lcore_hello(NULL);

    rte_eal_mp_wait_lcore();
    return 0;
}

