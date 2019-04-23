/*
 * File      : http_client_ota.h
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-03-22     Murphy       the first version
 */
#define HTTP_OTA_DEBUG 0

/* error level log */
#ifdef  log_e
#undef  log_e
#endif
#define log_e(...)                     rt_kprintf("\033[31;22m[E/HTTP_OTA] (%s:%d) ", __FUNCTION__, __LINE__);rt_kprintf(__VA_ARGS__);rt_kprintf("\033[0m\n")

/* info level log */
#ifdef  log_i
#undef  log_i
#endif
#define log_i(...)                     rt_kprintf("\033[36;22m[I/HTTP_OTA] ");                                rt_kprintf(__VA_ARGS__);rt_kprintf("\033[0m\n")

#if HTTP_OTA_DEBUG

/* debug level log */
#ifdef  log_d
#undef  log_d
#endif
#define log_d(...)                     rt_kprintf("[D/HTTP_OTA] (%s:%d) ", __FUNCTION__, __LINE__);           rt_kprintf(__VA_ARGS__);rt_kprintf("\n")

#else

/* debug level log */
#ifdef  log_d
#undef  log_d
#endif
#define log_d(...)

#endif /* HTTP_OTA_DEBUG */
