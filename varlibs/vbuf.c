/*
 * varlib: tiny variable memory-management structures
 *
 * inherited from inebriated, C edition
 * -eta
 */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <err.h>
#include "vbuf.h"
/**
 * Initialises a dynamic pointer array object.
 * Returns a pointer to said object, or NULL if there was an error.
 */
extern DPA *DPA_init(void) {
    DPA *dpa = malloc(sizeof(DPA));
    if (dpa == NULL) err(EXIT_FAILURE, "varlibs DPA_init(): malloc failed");
    dpa->keys = calloc(DPA_START_SIZE, sizeof(void *));
    if (dpa->keys == NULL) err(EXIT_FAILURE, "varlibs DPA_init(): malloc failed");
    dpa->used = 0;
    dpa->size = DPA_START_SIZE;
    return dpa;
}
/**
 * Removes obj from dpa, by swapping the last object of dpa->keys into the slot where
 * obj was, and setting that object to NULL whilst decrementing dpa->used.
 *
 * Does NOT make any attempt to free() obj, do this yourself. Returns false if object
 * does not exist.
 */
extern bool *DPA_rem(DPA *dpa, void *obj) {
    int i = 0, j = -1;
    for (void *tbr = dpa->keys[i]; i < dpa->used; tbr = dpa->keys[++i])
        if (tbr == obj) j = i;
    if (j == -1) return false;
    dpa->keys[j] = dpa->keys[i - 1];
    dpa->keys[i - 1] = NULL;
    dpa->used--;
    return true;
}
/**
 * Stores obj in dpa. Returns a pointer to obj if successful, or NULL if there was an error.
 */
extern void *DPA_store(DPA *dpa, void *obj) {
    if ((dpa->size - dpa->used) < 2) {
        // allocate more space
        void **ptr = realloc(dpa->keys, sizeof(void *) * (dpa->size + DPA_REFILL_SIZE));
        if (ptr == NULL) err(EXIT_FAILURE, "varlibs DPA_store(): realloc failed");
        dpa->keys = ptr;
        dpa->size += DPA_REFILL_SIZE;
    }
    (dpa->keys)[(dpa->used++)] = obj;
    return obj;
}

extern void DPA_free(DPA *dpa) {
    free(dpa->keys);
    free(dpa);
}

/**
 * Initialises a variable string object. Returns pointer on success, NULL on failure.
 */
extern struct varstr *varstr_init(void) {
    struct varstr *vs = malloc(sizeof(struct varstr));
    if (vs == NULL) err(EXIT_FAILURE, "varlibs varstr_init(): malloc failed");
    vs->str = calloc(VARSTR_START_SIZE, sizeof(char));
    if (vs->str == NULL) err(EXIT_FAILURE, "varlibs varstr_init(): malloc failed");
    vs->used = 0;
    vs->size = VARSTR_START_SIZE;
    return vs;
}
/**
 * (internal function) Refill a varstr if space left is less than/equal to iu.
 */
static struct varstr *varstr_refill_if_needed(struct varstr *vs, int iu) {
    if ((vs->size - vs->used) <= iu) {
        char *ptr = realloc(vs->str, sizeof(char) * (vs->size + iu + VARSTR_REFILL_SIZE));
        if (ptr == NULL) err(EXIT_FAILURE, "varlibs varstr_refill_if_needed(): realloc failed");
        memset(ptr + vs->size, L'\0', iu + VARSTR_REFILL_SIZE);
        vs->str = ptr;
        vs->size += iu;
        vs->size += VARSTR_REFILL_SIZE;
    }
    return vs;
}
/**
 * Appends a to b using strcat(), allocating more space if needed. Returns pointer to varstr object on success, NULL on failure.
 */
extern struct varstr *varstr_cat(struct varstr *vs, char *str) {
    vs = varstr_refill_if_needed(vs, (strlen(str) + 1));
    vs->used += (strlen(str) + 1);
    strcat(vs->str, str);
    return vs;
}
/**
 * Appends a to b using strncat(), allocating more space if needed. Returns pointer to varstr object on success, NULL on failure.
 */
extern struct varstr *varstr_ncat(struct varstr *vs, char *str, size_t count) {
    vs = varstr_refill_if_needed(vs, count + 1);
    vs->used += (count + 1);
    strncat(vs->str, str, count);
    return vs;
}
/**
 * Append a single char a to b, allocating more space if needed. Returns pointer to varstr object on success, NULL on failure.
 */
extern struct varstr *varstr_pushc(struct varstr *vs, char c) {
    vs = varstr_refill_if_needed(vs, 2);
    (vs->str)[(vs->used)++] = c;
    return vs;
}
/**
 * Free unused memory in a variable string & convert it to just a regular string.
 * Returns pointer to regular string, NULL on failure.
 *
 * Remember to free() the string!
 */
extern char *varstr_pack(struct varstr *vs) {
    char *ptr = malloc(sizeof(char) * (vs->used + 1));
    if (ptr == NULL) err(EXIT_FAILURE, "varlibs varstr_pack(): malloc failed");
    strcpy(ptr, (const char *) vs->str);
    free(vs->str);
    free(vs);
    return ptr;
};

extern void varstr_free(struct varstr *vs) {
    free(vs->str);
    free(vs);
};
