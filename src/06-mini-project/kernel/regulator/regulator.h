// workspace/src/06-mini-project/kernel/regulator/regulator.h
#ifndef REGULATOR_H
#define REGULATOR_H

#include <linux/types.h>

/* Callbacks for adjusting period and getting temperature */
struct regulator_callbacks {
    void (*adjust_period)(int new_period_ms);
    uint32_t (*get_temperature)(void);
};


/* Initialize the regulator logic, background thread, and sysfs. Register the callbacks with other parts */
int regulator_init(struct regulator_callbacks *cbs);

/* Stop the regulator logic and clean up */
void regulator_exit(void);

#endif /* REGULATOR_H */
