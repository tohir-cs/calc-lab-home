// Tokhirbek Akramov 251ADB079
// https://github.com/tohir-cs/calc-lab-home
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: Gemini AI -- https://share.gemini.google/5HAMwiYzEvIU

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#define STUDENT_NAME "tokhirbek"
#define STUDENT_LASTNAME "akramov"
#define STUDENT_ID "251ADB079"

typedef enum {
TOK_INT,
TOK_PLUS,
TOK_MINUS,
TOK_MUL,
TOK_DIV,
TOK_EOL,
TOK_INVALID
} TokenType;

typedef struct 
{
TokenType type;
size_t pos;
long long val;
} Token;

typedef struct
{
const char *src;
size_t len;
size_t idx;
Token cur;
size_t err_pos;
} Parser;

static void set_error(Parser *p, size_t pos) {
    if (p->err_pos == 0)
    {
    p->err_pos = pos;
    }
}

//tokenizer tracking 1based character position in entire files
static void next_token(Parser *p) {
    while (p->idx < p->len && (p->src[p->idx] == ' ' || p->src[p->idx] == '\t'))
    {
    p->idx++;
    }

    if (p->idx >= p->len)
    {
    p->cur.type = TOK_EOL;
    p->cur.pos = p->len + 1;
    return;
    }

char c = p->src[p->idx];
size_t start_pos = p->idx + 1;

    if (c == '\n')
    {
    p->cur.type = TOK_EOL;
    p->cur.pos = start_pos;
    return;
    }

    if (c == '+')
    {
    p->idx++;
    p->cur.type = TOK_PLUS;
    p->cur.pos = start_pos;
    return;
    }
    if (c == '-')
    {
    p->idx++;
    p->cur.type = TOK_MINUS;
    p->cur.pos = start_pos;
    return;
    }
    if (c == '*')
    {
    p->idx++;
    p->cur.type = TOK_MUL;
    p->cur.pos = start_pos;
    return;
    }

    if (c == '/')
    {
    p->idx++;
    p->cur.type = TOK_DIV;
    p->cur.pos = start_pos;
    return;
    }

    if (isdigit((unsigned char)c))
    {
    long long val = 0;
        while (p->idx < p->len && isdigit((unsigned char)p->src[p->idx])) {
        val = val * 10 + (p->src[p->idx] - '0');
        p->idx++;
        }
    p->cur.type = TOK_INT;
    p->cur.pos = start_pos;
    p->cur.val = val;
        return;
    }

    p->cur.type = TOK_INVALID;
    p->cur.pos = start_pos;
}

static long long parse_primary(Parser *p) {
    if (p->err_pos != 0) return 0;

    if (p->cur.type == TOK_INT)
    {
    long long val = p->cur.val;
    next_token(p);
return val;
    }

    set_error(p, p->cur.pos);
return 0;
}

//AI: prompt "how to parse * and / with higher precedence than + and - and track character position in C without external libraries"
//link: https://share.gemini.google/5HAMwiYzEvIU
//used recursive descent hierarchy: parse_term handles * and / before returning to parse_expr (+ and -)

static long long parse_term(Parser *p) {
    long long left = parse_primary(p);
    if (p->err_pos != 0) return left;

    while (p->cur.type == TOK_MUL || p->cur.type == TOK_DIV)
    {
    TokenType op = p->cur.type;
    next_token(p);

    size_t rhs_pos = p->cur.pos;
    long long right = parse_primary(p);
    if (p->err_pos != 0) return left;

    if (op == TOK_MUL) {
        left *= right;
        } else {
    if (right == 0) {
        set_error(p, rhs_pos);
        return left;
            }
    left /= right;
        }
    }
    return left;
}

static long long parse_expr(Parser *p) {
    long long left = parse_term(p);
    if (p->err_pos != 0) return left;

while (p->cur.type == TOK_PLUS || p->cur.type == TOK_MINUS) {
    TokenType op = p->cur.type;
    next_token(p);

    long long right = parse_term(p);
if (p->err_pos != 0) return left;

    if (op == TOK_PLUS) {
        left += right;
        } else {
    left -= right;
        }
    }
    return left;
}


//AI: prompt "how do i extract filename without directory path and without .txt extension from a string in C"
//link: https://share.gemini.google/5HAMwiYzEvIU


static void get_basename_no_ext(const char *path, char *base, size_t sz) {
    const char *slash = strrchr(path, '/');
    const char *fname = slash ? slash + 1 : path;
    size_t len = strlen(fname);
    if (len >= 4 && strcmp(fname + len - 4, ".txt") == 0)
    {
        len -= 4;
    }
    if (len >= sz) len = sz - 1;
    strncpy(base, fname, len);
    base[len] = '\0';
}

static char *read_all(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc(sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t n = fread(buf, 1, sz, f);
    buf[n] = '\0';
    fclose(f);
    *out_len = n;
    return buf;
}

int main(int argc, char **argv) {
    const char *outdir = NULL;
    const char *infile = NULL;

    for (int i = 1; i < argc; i++)
    {
if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output-dir") == 0) {
    if (i + 1 < argc) outdir = argv[++i];
        } else if (argv[i][0] != '-') {
        infile = argv[i];
        }
    }

    if (!infile)
    {
        fprintf(stderr, "Usage: %s [-o OUTDIR] input.txt\n", argv[0]);
        return 1;
    }

    size_t file_len = 0;
    char *src = read_all(infile, &file_len);
    if (!src) {
        fprintf(stderr, "Cannot open %s\n", infile);
        return 1;
    }

    char base[256];
    get_basename_no_ext(infile, base, sizeof(base));

    char def_outdir[512];
    if (!outdir) {
        snprintf(def_outdir, sizeof(def_outdir), "%s_%s_%s", base, STUDENT_NAME, STUDENT_ID);
        outdir = def_outdir;
    }

    mkdir(outdir, 0755);

    char outpath[1024];
    snprintf(outpath, sizeof(outpath), "%s/%s_%s_%s_%s.txt",
             outdir, base, STUDENT_NAME, STUDENT_LASTNAME, STUDENT_ID);
    Parser p;
    p.src = src;
    p.len = file_len;
    p.idx = 0;
    p.err_pos = 0;

    char *out_buf = malloc(file_len * 2 + 1024);
    size_t out_len = 0;

    while (p.idx < p.len)
    {
     while (p.idx < p.len && (p.src[p.idx] == ' ' || p.src[p.idx] == '\t')) {
            p.idx++;
        }

    if (p.idx < p.len && p.src[p.idx] == '\n') {
        p.idx++;
        continue;
        }

    if (p.idx >= p.len) break;

    next_token(&p);
    long long val = parse_expr(&p);

    if (p.err_pos == 0 && p.cur.type != TOK_EOL) {
        set_error(&p, p.cur.pos);
        }

    if (p.err_pos != 0) break;

    char temp[64];
    int w = snprintf(temp, sizeof(temp), "%lld\n", val);
    memcpy(out_buf + out_len, temp, w);
    out_len += w;

    if (p.idx < p.len && p.src[p.idx] == '\n') {
        p.idx++;
        }
    }

    FILE *fout = fopen(outpath, "w");
    if (!fout) {
        free(src);
        free(out_buf);
        return 1;
    }

    if (p.err_pos != 0)
    {
        fprintf(fout, "ERROR:%zu\n", p.err_pos);
    } else if (out_len > 0)
    {
        fwrite(out_buf, 1, out_len, fout);
    }

    fclose(fout);
    free(src);
    free(out_buf);
    return 0;
}