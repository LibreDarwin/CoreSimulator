/*
 * sim_plist - minimal property list reader for simctl.
 *
 * Copyright (c) 2026 Sunneva N. Mariu <sunnevanattsol@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * A small, self-contained reader for the two property list encodings
 * CoreSimulator uses on disk: the binary format (bplist00) and the XML
 * format. It exists so the read-only commands can inspect CoreSimulator
 * plists in-process instead of forking plutil(8) for every field.
 */

#ifndef SIM_PLIST_H
#define SIM_PLIST_H

#include <stddef.h>

typedef enum {
	PLIST_NULL,
	PLIST_BOOL,
	PLIST_INT,
	PLIST_REAL,
	PLIST_STRING,
	PLIST_DATA,
	PLIST_DATE,
	PLIST_ARRAY,
	PLIST_DICT
} plist_type;

typedef struct plist plist;

struct plist {
	plist_type type;

	int boolean;
	long long inum;
	double rnum;

	char *str;		/* PLIST_STRING, PLIST_DATE */
	unsigned char *data;	/* PLIST_DATA */
	size_t data_len;

	plist **items;		/* PLIST_ARRAY */
	size_t count;

	char **keys;		/* PLIST_DICT */
	plist **vals;
};

/*
 * Load and parse a property list file. Returns NULL if the file cannot
 * be read or is not a recognizable property list.
 */
plist *plist_load_file(const char *path);

/* Recursively release a parsed property list. */
void plist_free(plist *p);

/* Look up a key in a dictionary. Returns NULL if absent or not a dict. */
plist *plist_dict_get(const plist *dict, const char *key);

/* Value accessors. */
const char *plist_as_string(const plist *p);
long long plist_as_int(const plist *p, long long def);
double plist_as_real(const plist *p, double def);
int plist_as_bool(const plist *p, int def);

/*
 * Convenience accessors that mirror the field reads previously performed
 * through plutil(8). Each loads the named plist and returns the value of
 * a top-level key.
 *
 *   plist_read_string : returns a newly allocated string, or NULL if the
 *                       key is absent or its value is not a string.
 *   plist_read_int    : returns the integer value, or -1 if absent.
 *   plist_read_bool   : returns 1/0 for a boolean, or -1 if absent.
 *   plist_read_family_id : returns the first element of the
 *                       supportedProductFamilyIDs array, or -1 if absent.
 */
char *plist_read_string(const char *path, const char *key);
int plist_read_int(const char *path, const char *key);
int plist_read_bool(const char *path, const char *key);
int plist_read_family_id(const char *path);

#endif /* SIM_PLIST_H */
