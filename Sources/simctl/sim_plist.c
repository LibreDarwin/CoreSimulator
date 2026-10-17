/*
 * sim_plist - minimal property list reader for simctl.
 *
 * Copyright (c) 2026 Sunneva N. Mariu <sunnevanattsol@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Supports the binary property list format (bplist00) and the XML
 * property list format. The implementation is written from the public
 * format descriptions and contains no code derived from Apple sources.
 */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sim_plist.h"

/* ------------------------------------------------------------------ */
/* Allocation helpers                                                  */
/* ------------------------------------------------------------------ */

static void *
xmalloc(size_t n)
{
	return malloc(n == 0 ? 1 : n);
}

static plist *
plist_new(plist_type type)
{
	plist *p = xmalloc(sizeof *p);

	memset(p, 0, sizeof *p);
	p->type = type;
	return p;
}

static char *
xstrndup(const char *s, size_t n)
{
	char *r = xmalloc(n + 1);

	memcpy(r, s, n);
	r[n] = '\0';
	return r;
}

void
plist_free(plist *p)
{
	if (p == NULL)
		return;

	if (p->type == PLIST_ARRAY) {
		for (size_t i = 0; i < p->count; i++)
			plist_free(p->items[i]);
		free(p->items);
	} else if (p->type == PLIST_DICT) {
		for (size_t i = 0; i < p->count; i++) {
			free(p->keys[i]);
			plist_free(p->vals[i]);
		}
		free(p->keys);
		free(p->vals);
	}

	free(p->str);
	free(p->data);
	free(p);
}

plist *
plist_dict_get(const plist *dict, const char *key)
{
	if (dict == NULL || dict->type != PLIST_DICT)
		return NULL;
	for (size_t i = 0; i < dict->count; i++) {
		if (strcmp(dict->keys[i], key) == 0)
			return dict->vals[i];
	}
	return NULL;
}

const char *
plist_as_string(const plist *p)
{
	if (p != NULL && p->type == PLIST_STRING)
		return p->str;
	return NULL;
}

long long
plist_as_int(const plist *p, long long def)
{
	if (p == NULL)
		return def;
	switch (p->type) {
	case PLIST_INT:
		return p->inum;
	case PLIST_BOOL:
		return p->boolean;
	case PLIST_REAL:
		return (long long)p->rnum;
	case PLIST_STRING:
		return strtoll(p->str, NULL, 10);
	default:
		return def;
	}
}

double
plist_as_real(const plist *p, double def)
{
	if (p == NULL)
		return def;
	switch (p->type) {
	case PLIST_REAL:
		return p->rnum;
	case PLIST_INT:
		return (double)p->inum;
	case PLIST_BOOL:
		return (double)p->boolean;
	default:
		return def;
	}
}

int
plist_as_bool(const plist *p, int def)
{
	if (p != NULL && p->type == PLIST_BOOL)
		return p->boolean;
	return def;
}

/* ------------------------------------------------------------------ */
/* Binary property list (bplist00)                                     */
/* ------------------------------------------------------------------ */

static uint64_t
be_read(const unsigned char *p, unsigned n)
{
	uint64_t v = 0;

	for (unsigned i = 0; i < n; i++)
		v = (v << 8) | p[i];
	return v;
}

struct bctx {
	const unsigned char *data;
	size_t size;
	uint64_t offset_table;
	unsigned offset_size;
	unsigned ref_size;
	uint64_t num_objects;
};

static plist *bobj(struct bctx *c, uint64_t index, int depth);

static uint64_t
bcount(struct bctx *c, const unsigned char *p, unsigned lo, size_t *hdr)
{
	if (lo != 0xf) {
		*hdr = 1;
		return lo;
	}
	{
		unsigned marker = p[1];
		unsigned bytes;

		if ((marker >> 4) != 0x1)
			return UINT64_MAX;
		bytes = 1u << (marker & 0xf);
		if ((size_t)(p - c->data) + 2 + bytes > c->size)
			return UINT64_MAX;
		*hdr = 2 + bytes;
		return be_read(p + 2, bytes);
	}
}

static char *
utf16_to_utf8(const unsigned char *p, uint64_t units)
{
	char *out = xmalloc(units * 3 + 1);
	size_t o = 0;

	for (uint64_t i = 0; i < units; i++) {
		uint32_t cp = ((uint32_t)p[i * 2] << 8) | p[i * 2 + 1];

		if (cp >= 0xd800 && cp <= 0xdbff && i + 1 < units) {
			uint32_t lo = ((uint32_t)p[(i + 1) * 2] << 8) |
			    p[(i + 1) * 2 + 1];
			if (lo >= 0xdc00 && lo <= 0xdfff) {
				cp = 0x10000 + ((cp - 0xd800) << 10) +
				    (lo - 0xdc00);
				i++;
			}
		}
		if (cp < 0x80) {
			out[o++] = (char)cp;
		} else if (cp < 0x800) {
			out[o++] = (char)(0xc0 | (cp >> 6));
			out[o++] = (char)(0x80 | (cp & 0x3f));
		} else if (cp < 0x10000) {
			out[o++] = (char)(0xe0 | (cp >> 12));
			out[o++] = (char)(0x80 | ((cp >> 6) & 0x3f));
			out[o++] = (char)(0x80 | (cp & 0x3f));
		} else {
			out[o++] = (char)(0xf0 | (cp >> 18));
			out[o++] = (char)(0x80 | ((cp >> 12) & 0x3f));
			out[o++] = (char)(0x80 | ((cp >> 6) & 0x3f));
			out[o++] = (char)(0x80 | (cp & 0x3f));
		}
	}
	out[o] = '\0';
	return out;
}

static plist *
bobj(struct bctx *c, uint64_t index, int depth)
{
	const unsigned char *p;
	uint64_t off;
	unsigned marker, hi, lo;
	size_t hdr = 0;
	uint64_t count;

	if (depth > 64 || index >= c->num_objects)
		return NULL;

	off = be_read(c->data + c->offset_table + index * c->offset_size,
	    c->offset_size);
	if (off >= c->size)
		return NULL;
	p = c->data + off;
	marker = p[0];
	hi = marker >> 4;
	lo = marker & 0xf;

	switch (hi) {
	case 0x0: {
		plist *v = plist_new(PLIST_NULL);

		if (lo == 0x8) {
			v->type = PLIST_BOOL;
			v->boolean = 0;
		} else if (lo == 0x9) {
			v->type = PLIST_BOOL;
			v->boolean = 1;
		}
		return v;
	}
	case 0x1: {
		unsigned bytes = 1u << lo;
		uint64_t uv;
		plist *v;

		if ((size_t)(p - c->data) + 1 + bytes > c->size)
			return NULL;
		uv = be_read(p + 1, bytes);
		if (bytes < 8 && (uv & (1ULL << (bytes * 8 - 1))))
			uv |= ~((1ULL << (bytes * 8)) - 1);
		v = plist_new(PLIST_INT);
		v->inum = (long long)uv;
		return v;
	}
	case 0x2: {
		unsigned bytes = 1u << lo;
		plist *v = plist_new(PLIST_REAL);

		if ((size_t)(p - c->data) + 1 + bytes > c->size)
			return NULL;
		if (bytes == 4) {
			uint32_t bits = (uint32_t)be_read(p + 1, 4);
			float f;

			memcpy(&f, &bits, sizeof f);
			v->rnum = f;
		} else {
			uint64_t bits = be_read(p + 1, 8);

			memcpy(&v->rnum, &bits, sizeof v->rnum);
		}
		return v;
	}
	case 0x3: {
		uint64_t bits;
		plist *v;

		if ((size_t)(p - c->data) + 9 > c->size)
			return NULL;
		bits = be_read(p + 1, 8);
		v = plist_new(PLIST_DATE);
		memcpy(&v->rnum, &bits, sizeof v->rnum);
		return v;
	}
	case 0x4: {
		plist *v;

		count = bcount(c, p, lo, &hdr);
		if (count == UINT64_MAX ||
		    (size_t)(p - c->data) + hdr + count > c->size)
			return NULL;
		v = plist_new(PLIST_DATA);
		v->data = xmalloc(count);
		memcpy(v->data, p + hdr, count);
		v->data_len = count;
		return v;
	}
	case 0x5: {
		plist *v;

		count = bcount(c, p, lo, &hdr);
		if (count == UINT64_MAX ||
		    (size_t)(p - c->data) + hdr + count > c->size)
			return NULL;
		v = plist_new(PLIST_STRING);
		v->str = xstrndup((const char *)p + hdr, count);
		return v;
	}
	case 0x6: {
		plist *v;

		count = bcount(c, p, lo, &hdr);
		if (count == UINT64_MAX ||
		    (size_t)(p - c->data) + hdr + count * 2 > c->size)
			return NULL;
		v = plist_new(PLIST_STRING);
		v->str = utf16_to_utf8(p + hdr, count);
		return v;
	}
	case 0x8: {
		/* UID: stored as an unsigned integer. */
		unsigned bytes = lo + 1;
		plist *v;

		if ((size_t)(p - c->data) + 1 + bytes > c->size)
			return NULL;
		v = plist_new(PLIST_INT);
		v->inum = (long long)be_read(p + 1, bytes);
		return v;
	}
	case 0xa:
	case 0xc: {
		plist *v;

		count = bcount(c, p, lo, &hdr);
		if (count == UINT64_MAX ||
		    (size_t)(p - c->data) + hdr + count * c->ref_size >
		    c->size)
			return NULL;
		v = plist_new(PLIST_ARRAY);
		v->count = count;
		v->items = xmalloc(count * sizeof *v->items);
		for (uint64_t i = 0; i < count; i++) {
			uint64_t ref = be_read(p + hdr + i * c->ref_size,
			    c->ref_size);

			v->items[i] = bobj(c, ref, depth + 1);
		}
		return v;
	}
	case 0xd: {
		plist *v;

		count = bcount(c, p, lo, &hdr);
		if (count == UINT64_MAX ||
		    (size_t)(p - c->data) + hdr + count * c->ref_size * 2 >
		    c->size)
			return NULL;
		v = plist_new(PLIST_DICT);
		v->count = count;
		v->keys = xmalloc(count * sizeof *v->keys);
		v->vals = xmalloc(count * sizeof *v->vals);
		for (uint64_t i = 0; i < count; i++) {
			uint64_t kref = be_read(
			    p + hdr + i * c->ref_size, c->ref_size);
			plist *k = bobj(c, kref, depth + 1);

			v->keys[i] = (k != NULL && k->type == PLIST_STRING)
			    ? strdup(k->str) : strdup("");
			plist_free(k);
		}
		for (uint64_t i = 0; i < count; i++) {
			uint64_t vref = be_read(
			    p + hdr + (count + i) * c->ref_size, c->ref_size);

			v->vals[i] = bobj(c, vref, depth + 1);
		}
		return v;
	}
	default:
		return plist_new(PLIST_NULL);
	}
}

static plist *
parse_binary(const unsigned char *d, size_t n)
{
	struct bctx c;
	const unsigned char *tr;
	uint64_t top;

	if (n < 40 || memcmp(d, "bplist00", 8) != 0)
		return NULL;

	tr = d + n - 32;
	c.data = d;
	c.size = n;
	c.offset_size = tr[6];
	c.ref_size = tr[7];
	c.num_objects = be_read(tr + 8, 8);
	top = be_read(tr + 16, 8);
	c.offset_table = be_read(tr + 24, 8);

	if (c.offset_size < 1 || c.offset_size > 8 ||
	    c.ref_size < 1 || c.ref_size > 8 ||
	    c.num_objects == 0 || c.offset_table >= n)
		return NULL;

	return bobj(&c, top, 0);
}

/* ------------------------------------------------------------------ */
/* XML property list                                                   */
/* ------------------------------------------------------------------ */

static const char *
skip_ws(const char *p, const char *end)
{
	while (p < end && isspace((unsigned char)*p))
		p++;
	return p;
}

static const char *
skip_misc(const char *p, const char *end)
{
	for (;;) {
		p = skip_ws(p, end);
		if (p + 3 < end && p[0] == '<' && p[1] == '!' &&
		    p[2] == '-' && p[3] == '-') {
			const char *e = NULL;

			for (const char *q = p + 4; q + 2 < end; q++) {
				if (q[0] == '-' && q[1] == '-' &&
				    q[2] == '>') {
					e = q + 3;
					break;
				}
			}
			p = e != NULL ? e : end;
			continue;
		}
		if (p + 1 < end && p[0] == '<' && p[1] == '?') {
			const char *e = NULL;

			for (const char *q = p + 2; q + 1 < end; q++) {
				if (q[0] == '?' && q[1] == '>') {
					e = q + 2;
					break;
				}
			}
			p = e != NULL ? e : end;
			continue;
		}
		if (p + 8 < end && strncmp(p, "<!DOCTYPE", 9) == 0) {
			const char *e = NULL;

			for (const char *q = p; q < end; q++) {
				if (*q == '>') {
					e = q + 1;
					break;
				}
			}
			p = e != NULL ? e : end;
			continue;
		}
		break;
	}
	return p;
}

/*
 * Read an XML entity or literal run into out. Returns the number of
 * source bytes consumed into *consumed.
 */
static size_t
decode_entities(const char *s, const char *end, char *out, size_t cap)
{
	size_t o = 0;

	while (s < end) {
		if (*s == '&') {
			const char *semi = NULL;

			for (const char *q = s + 1; q < end && q < s + 12;
			    q++) {
				if (*q == ';') {
					semi = q;
					break;
				}
			}
			if (semi != NULL) {
				size_t n = (size_t)(semi - (s + 1));
				char enc[12];

				if (n < sizeof enc) {
					memcpy(enc, s + 1, n);
					enc[n] = '\0';
					if (strcmp(enc, "amp") == 0)
						out[o++] = '&';
					else if (strcmp(enc, "lt") == 0)
						out[o++] = '<';
					else if (strcmp(enc, "gt") == 0)
						out[o++] = '>';
					else if (strcmp(enc, "quot") == 0)
						out[o++] = '"';
					else if (strcmp(enc, "apos") == 0)
						out[o++] = '\'';
					else if (enc[0] == '#') {
						long cp = (enc[1] == 'x' ||
						    enc[1] == 'X')
						    ? strtol(enc + 2, NULL,
						    16)
						    : strtol(enc + 1, NULL,
						    10);

						if (cp < 0x80) {
							out[o++] = (char)cp;
						} else if (cp < 0x800) {
							out[o++] = (char)(0xc0 |
							    (cp >> 6));
							out[o++] = (char)(0x80 |
							    (cp & 0x3f));
						} else {
							out[o++] = (char)(0xe0 |
							    (cp >> 12));
							out[o++] = (char)(0x80 |
							    ((cp >> 6) & 0x3f));
							out[o++] = (char)(0x80 |
							    (cp & 0x3f));
						}
					} else {
						out[o++] = *s;
					}
					s = semi + 1;
					continue;
				}
			}
		}
		if (o + 1 < cap)
			out[o++] = *s;
		s++;
	}
	out[o] = '\0';
	return o;
}

static plist *xml_value(const char **pp, const char *end, int depth);

/*
 * Locate the content and closing tag of the element starting at p (which
 * points just past the opening tag's '>'). Sets *content to the first
 * byte of the element text and *content_end to the '<' of the closing
 * tag (equal to *content when the tag is self-closing), and returns a
 * pointer just past the closing tag.
 */
static const char *
element_body(const char *p, const char *end, const char *name, size_t nlen,
    int selfclose, const char **content, const char **content_end)
{
	*content = p;
	if (selfclose) {
		*content_end = p;
		return p;
	}

	for (const char *q = p; q + 1 < end; q++) {
		if (q[0] == '<' && q[1] == '/') {
			if ((size_t)(end - q) > nlen + 2 &&
			    strncmp(q + 2, name, nlen) == 0) {
				const char *after = q + 2 + nlen;

				if (after < end && *after == '>') {
					*content_end = q;
					return after + 1;
				}
			}
		}
	}
	*content_end = end;
	return end;
}

static plist *
xml_value(const char **pp, const char *end, int depth)
{
	const char *p = skip_misc(*pp, end);
	const char *name;
	const char *content, *content_end, *next;
	size_t nlen;
	int selfclose = 0;
	plist *v = NULL;

	if (depth > 128 || p >= end || *p != '<')
		return NULL;
	p++;
	name = p;
	while (p < end && *p != '>' && !isspace((unsigned char)*p) &&
	    *p != '/')
		p++;
	nlen = (size_t)(p - name);
	while (p < end && *p != '>') {
		if (*p == '/')
			selfclose = 1;
		p++;
	}
	if (p < end)
		p++;
	next = element_body(p, end, name, nlen, selfclose, &content,
	    &content_end);

	if (nlen == 4 && strncmp(name, "dict", 4) == 0) {
		v = plist_new(PLIST_DICT);
		v->count = 0;
		v->keys = NULL;
		v->vals = NULL;
		const char *q = content;

		for (;;) {
			q = skip_misc(q, content_end);
			if (q >= content_end || (q[0] == '<' && q[1] == '/'))
				break;
			if (!(q + 4 < content_end &&
			    strncmp(q + 1, "key", 3) == 0 &&
			    (q[4] == '>' || isspace((unsigned char)q[4])))) {
				/* Unexpected token; skip it. */
				plist *junk = xml_value(&q, content_end,
				    depth + 1);

				plist_free(junk);
				continue;
			}
			{
				const char *kn = q + 1;
				const char *kp = kn;
				size_t knl;
				int ksc = 0;
				const char *kb, *kc, *kce, *knext;
				char *key;

				while (kp < content_end && *kp != '>' &&
				    !isspace((unsigned char)*kp) &&
				    *kp != '/')
					kp++;
				knl = (size_t)(kp - kn);
				while (kp < content_end && *kp != '>') {
					if (*kp == '/')
						ksc = 1;
					kp++;
				}
				if (kp < content_end)
					kp++;
				kb = kp;
				knext = element_body(kb, content_end, kn, knl, ksc,
				    &kc, &kce);
				key = xmalloc((size_t)(kce - kc) + 1);
				decode_entities(kc, kce, key,
				    (size_t)(kce - kc) + 1);
				q = skip_misc(knext, content_end);

				plist *val = xml_value(&q, content_end,
				    depth + 1);
				if (val == NULL)
					val = plist_new(PLIST_NULL);

				size_t nn = v->count + 1;

				v->keys = realloc(v->keys, nn * sizeof *v->keys);
				v->vals = realloc(v->vals, nn * sizeof *v->vals);
				v->keys[v->count] = key;
				v->vals[v->count] = val;
				v->count = nn;
			}
		}
	} else if (nlen == 5 && strncmp(name, "array", 5) == 0) {
		v = plist_new(PLIST_ARRAY);
		v->count = 0;
		v->items = NULL;
		const char *q = content;

		for (;;) {
			q = skip_misc(q, content_end);
			if (q >= content_end || (q[0] == '<' && q[1] == '/'))
				break;
			plist *val = xml_value(&q, content_end, depth + 1);

			if (val == NULL)
				break;
			size_t nn = v->count + 1;

			v->items = realloc(v->items, nn * sizeof *v->items);
			v->items[v->count] = val;
			v->count = nn;
		}
	} else if (nlen == 6 && strncmp(name, "string", 6) == 0) {
		v = plist_new(PLIST_STRING);
		v->str = xmalloc((size_t)(content_end - content) + 1);
		decode_entities(content, content_end, v->str,
		    (size_t)(content_end - content) + 1);
	} else if (nlen == 7 && strncmp(name, "integer", 7) == 0) {
		char tmp[64];
		size_t n = (size_t)(content_end - content);

		if (n >= sizeof tmp)
			n = sizeof tmp - 1;
		memcpy(tmp, content, n);
		tmp[n] = '\0';
		v = plist_new(PLIST_INT);
		v->inum = strtoll(tmp, NULL, 10);
	} else if (nlen == 4 && strncmp(name, "real", 4) == 0) {
		char tmp[64];
		size_t n = (size_t)(content_end - content);

		if (n >= sizeof tmp)
			n = sizeof tmp - 1;
		memcpy(tmp, content, n);
		tmp[n] = '\0';
		v = plist_new(PLIST_REAL);
		v->rnum = strtod(tmp, NULL);
	} else if (nlen == 4 && strncmp(name, "true", 4) == 0) {
		v = plist_new(PLIST_BOOL);
		v->boolean = 1;
	} else if (nlen == 5 && strncmp(name, "false", 5) == 0) {
		v = plist_new(PLIST_BOOL);
		v->boolean = 0;
	} else if (nlen == 4 && strncmp(name, "data", 4) == 0) {
		v = plist_new(PLIST_DATA);
		v->data = xmalloc((size_t)(content_end - content) + 1);
		v->data_len = decode_entities(content, content_end,
		    (char *)v->data, (size_t)(content_end - content) + 1);
	} else if (nlen == 4 && strncmp(name, "date", 4) == 0) {
		v = plist_new(PLIST_DATE);
		v->str = xmalloc((size_t)(content_end - content) + 1);
		decode_entities(content, content_end, v->str,
		    (size_t)(content_end - content) + 1);
	} else if (nlen == 5 && strncmp(name, "plist", 5) == 0) {
		const char *q = skip_misc(content, content_end);

		v = xml_value(&q, content_end, depth + 1);
	} else {
		v = plist_new(PLIST_NULL);
	}

	*pp = next;
	return v;
}

static plist *
parse_xml(const char *d, size_t n)
{
	const char *p = d;
	const char *end = d + n;
	plist *v;

	p = skip_misc(p, end);
	if (p >= end || *p != '<')
		return NULL;
	v = xml_value(&p, end, 0);
	return v;
}

/* ------------------------------------------------------------------ */
/* Public entry points                                                 */
/* ------------------------------------------------------------------ */

plist *
plist_load_file(const char *path)
{
	FILE *fp;
	char *buf = NULL;
	size_t cap = 0, len = 0;
	plist *p;

	fp = fopen(path, "rb");
	if (fp == NULL)
		return NULL;

	for (;;) {
		size_t got;

		if (len == cap) {
			cap = cap == 0 ? 65536 : cap * 2;
			buf = realloc(buf, cap);
			if (buf == NULL) {
				fclose(fp);
				return NULL;
			}
		}
		got = fread(buf + len, 1, cap - len, fp);
		len += got;
		if (got == 0)
			break;
	}
	fclose(fp);

	if (len == 0) {
		free(buf);
		return NULL;
	}

	if (len >= 8 && memcmp(buf, "bplist00", 8) == 0)
		p = parse_binary((const unsigned char *)buf, len);
	else
		p = parse_xml(buf, len);

	free(buf);
	return p;
}

char *
plist_read_string(const char *path, const char *key)
{
	plist *root = plist_load_file(path);
	plist *v;
	char *r = NULL;

	if (root == NULL)
		return NULL;
	v = plist_dict_get(root, key);
	if (v != NULL && v->type == PLIST_STRING)
		r = strdup(v->str);
	plist_free(root);
	return r;
}

int
plist_read_int(const char *path, const char *key)
{
	plist *root = plist_load_file(path);
	plist *v;
	int r = -1;

	if (root == NULL)
		return -1;
	v = plist_dict_get(root, key);
	if (v != NULL)
		r = (int)plist_as_int(v, -1);
	plist_free(root);
	return r;
}

int
plist_read_bool(const char *path, const char *key)
{
	plist *root = plist_load_file(path);
	plist *v;
	int r = -1;

	if (root == NULL)
		return -1;
	v = plist_dict_get(root, key);
	if (v != NULL && v->type == PLIST_BOOL)
		r = v->boolean;
	plist_free(root);
	return r;
}

int
plist_read_family_id(const char *path)
{
	plist *root = plist_load_file(path);
	plist *arr, *first;
	int r = -1;

	if (root == NULL)
		return -1;
	arr = plist_dict_get(root, "supportedProductFamilyIDs");
	if (arr != NULL && arr->type == PLIST_ARRAY && arr->count > 0) {
		first = arr->items[0];
		if (first != NULL)
			r = (int)plist_as_int(first, -1);
	}
	plist_free(root);
	return r;
}
