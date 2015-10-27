#ifndef _ETA_VARLIB
#define _ETA_VARLIB
#define DPA_START_SIZE 4 /**< Default DPA start length */
#define DPA_REFILL_SIZE 1 /**< Default DPA refill length */
#define VARSTR_START_SIZE 15 /**< Default variable string start length */
#define VARSTR_REFILL_SIZE 5 /**< Default variable string refill length */
#define DPA_ENUMERATE(dpa) for (int DPA_N = 0; DPA_N < dpa->used; DPA_N++)
#define EITEM(dpa) dpa->keys[DPA_N]
#include <stdbool.h>
/**
 * \brief Dynamic Pointer Array: Used to store a set of pointers dynamically.
 */
typedef struct {
    void **keys; /**< Array of objects */
    int used; /**< Objects stored */
    int size; /**< Array size (in objects) */
} DPA;

extern DPA *DPA_init(void);
extern void *DPA_store(DPA *dpa, void *obj);
extern bool *DPA_rem(DPA *dpa, void *obj);
extern void DPA_free(DPA *dpa);

/**
 * \brief Expandable variable-length string.
 */
struct varstr {
    char *str; /**< string */
    int used; /**< chars used (not including null) */
    int size; /**< size of string (including null) */
};

extern struct varstr *varstr_init(void);
extern struct varstr *varstr_cat(struct varstr *vs, char *str);
extern struct varstr *varstr_ncat(struct varstr *vs, char *str, size_t count);
extern struct varstr *varstr_pushc(struct varstr *vs, char c);
extern char *varstr_pack(struct varstr *vs);
extern void varstr_free(struct varstr *vs);
#endif
