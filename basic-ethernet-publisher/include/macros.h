#ifndef MACROS_H_
#define MACROS_H_

#define RC_CHECK(fn)                  \
  {                                   \
    rcl_ret_t temp_rc = fn;           \
    if ((temp_rc != RCL_RET_OK)) {    \
      HandleReturnCodeError(temp_rc); \
    }                                 \
  }

#define EXECUTE_EVERY_N_MS(MS, X)      \
  do {                                 \
    static volatile int64_t init = -1; \
    if (init == -1) {                  \
      init = uxr_millis();             \
    }                                  \
    if (uxr_millis() - init > MS) {    \
      X;                               \
      init = uxr_millis();             \
    }                                  \
  } while (0)

#endif /* MACROS_H_ */
