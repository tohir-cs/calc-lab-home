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
#include <dirent.h>

#define STUDENT_NAME "tokhirbek"
#define STUDENT_LASTNAME "akramov"
#define STUDENT_ID "251ADB079"

typedef enum {
TOK_INT,
TOK_PLUS,
TOK_MINUS,
TOK_MUL,
TOK_DIV,
TOK_POW,
TOK_LPAREN,
TOK_RPAREN,
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

static void set_error(Parser *p, size_t pos) 
{
    if (p->err_pos == 0)
    {
    p->err_pos = pos;
    }
}

//tokenizer tracking 1based character position in entire files
static void next_token(Parser *p) 
{
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

    if (c == '\n' || c == '\r')
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
        if (p->idx + 1 < p->len && p->src[p->idx + 1] == '*')
        {
        p->idx += 2;
        p->cur.type = TOK_POW;
        p->cur.pos = start_pos;
        return;
        }
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

    if (c == '(')
    {
    p->idx++;
    p->cur.type = TOK_LPAREN;
    p->cur.pos = start_pos;
    return;
    }

    if (c == ')')
    {
    p->idx++;
    p->cur.type = TOK_RPAREN;
    p->cur.pos = start_pos;
    return;
    }

    if (isdigit((unsigned char)c))
    {
    long long val = 0;
        while (p->idx < p->len && isdigit((unsigned char)p->src[p->idx])) 
        {
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
    p->idx++;
}

static long long parse_expr(Parser *p);

//handle numbers and parenthesis
static long long parse_factor(Parser *p) 
{
    if (p->err_pos != 0) return 0;

    if (p->cur.type == TOK_INT)
    {
    long long val = p->cur.val;
    next_token(p);
    return val;
    }

    if (p->cur.type == TOK_LPAREN)
    {
    next_token(p);
    long long val = parse_expr(p);
    if (p->err_pos != 0) return 0;

        if (p->cur.type == TOK_RPAREN)
        {
        next_token(p);
        return val;
        } 
        else 
        {
        set_error(p, p->cur.pos);
        return 0;
        }
    }

    set_error(p, p->cur.pos);
    return 0;
}

//integer power loop
static long long ipow(long long base, long long exp) 
{
    long long res = 1;
    while (exp > 0)
    {
        if (exp % 2 == 1) res *= base;
        base *= base;
        exp /= 2;
    }
    return res;
}

//right associative power
static long long parse_power(Parser *p) 
{
    long long left = parse_factor(p);
    if (p->err_pos != 0) return left;

    if (p->cur.type == TOK_POW)
    {
    next_token(p);
    size_t rhs_pos = p->cur.pos;
    long long right = parse_power(p);
    if (p->err_pos != 0) return left;

        if (right < 0)
        {
        set_error(p, rhs_pos);
        return left;
        }
    left = ipow(left, right);
    }
    return left;
}

//AI: prompt "how to parse * and / with higher precedence than + and - and track character position in C without external libraries"
//link: https://share.gemini.google/5HAMwiYzEvIU
//used recursive descent hierarchy: parse_term handles * and / before returning to parse_expr (+ and -)

static long long parse_term(Parser *p) 
{
    long long left = parse_power(p);
    if (p->err_pos != 0) return left;

    while (p->cur.type == TOK_MUL || p->cur.type == TOK_DIV)
    {
    TokenType op = p->cur.type;
    next_token(p);

    size_t rhs_pos = p->cur.pos;
    long long right = parse_power(p);
    if (p->err_pos != 0) return left;

        if (op == TOK_MUL) 
        {
        left *= right;
        } 
        else 
        {
            if (right == 0) 
            {
            set_error(p, rhs_pos);
            return left;
            }
        left /= right;
        }
    }
    return left;
}

static long long parse_expr(Parser *p) 
{
    long long left = parse_term(p);
    if (p->err_pos != 0) return left;

    while (p->cur.type == TOK_PLUS || p->cur.type == TOK_MINUS) 
    {
    TokenType op = p->cur.type;
    next_token(p);

    long long right = parse_term(p);
    if (p->err_pos != 0) return left;

        if (op == TOK_PLUS) 
        {
        left += right;
        } 
        else 
        {
        left -= right;
        }
    }
    return left;
}

//AI: prompt "how do i extract filename without directory path and without .txt extension from a string in C"
//link: https://share.gemini.google/5HAMwiYzEvIU

static void get_basename_no_ext(const char *path, char *base, size_t sz) 
{
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

static char *read_all(const char *path, size_t *out_len) 
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc(sz + 1);
    if (!buf) 
    {
        fclose(f);
        return NULL;
    }
    size_t n = fread(buf, 1, sz, f);
    buf[n] = '\0';
    fclose(f);
    *out_len = n;
    return buf;
}

//process single input file and write output
static int process_single_file(const char *infile, const char *outdir)
{
    size_t file_len = 0;
    char *src = read_all(infile, &file_len);
    if (!src) 
    {
        fprintf(stderr, "Cannot open %s\n", infile);
        return 1;
    }

    char base[256];
    get_basename_no_ext(infile, base, sizeof(base));

    char def_outdir[512];
    if (!outdir) 
    {
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
        while (p.idx < p.len && (p.src[p.idx] == ' ' || p.src[p.idx] == '\t')) 
        {
            p.idx++;
        }

        if (p.idx < p.len && (p.src[p.idx] == '\n' || p.src[p.idx] == '\r')) 
        {
        p.idx++;
        continue;
        }

        //skip whole comment line starting with #
        if (p.idx < p.len && p.src[p.idx] == '#') 
        {
            while (p.idx < p.len && p.src[p.idx] != '\n') 
            {
                p.idx++;
            }
            if (p.idx < p.len && p.src[p.idx] == '\n') p.idx++;
            continue;
        }

        if (p.idx >= p.len) break;

        next_token(&p);
        long long val = parse_expr(&p);

        if (p.err_pos == 0 && p.cur.type != TOK_EOL) 
        {
            set_error(&p, p.cur.pos);
        }

        if (p.err_pos != 0) break;

        char temp[64];
        int w = snprintf(temp, sizeof(temp), "%lld\n", val);
        memcpy(out_buf + out_len, temp, w);
        out_len += w;

        while (p.idx < p.len && p.src[p.idx] != '\n') 
        {
            p.idx++;
        }
        if (p.idx < p.len && p.src[p.idx] == '\n') 
        {
            p.idx++;
        }
    }

    FILE *fout = fopen(outpath, "w");
    if (!fout) 
    {
        free(src);
        free(out_buf);
        return 1;
    }

    if (p.err_pos != 0)
    {
        fprintf(fout, "ERROR:%zu\n", p.err_pos);
    } 
    else if (out_len > 0)
    {
        fwrite(out_buf, 1, out_len, fout);
    }

    fclose(fout);
    free(src);
    free(out_buf);
    return 0;
}

//sort filenames
static int cmp_strings(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

//AI: prompt "how to read all .txt files from a directory and parse CLI options like -d, --dir, -o, --output-dir in C POSIX"
//link: https://share.gemini.google/5HAMwiYzEvIU
//scan directory, collect txt files and run each

static int process_directory(const char *indir, const char *outdir)
{
    DIR *d = opendir(indir);
    if (!d)
    {
        fprintf(stderr, "Cannot open directory %s\n", indir);
        return 1;
    }


    char *files[512];
    int count = 0;
    struct dirent *ent;

    while ((ent = readdir(d)) != NULL)
    {
        size_t n = strlen(ent->d_name);
        if (n >= 4 && strcmp(ent->d_name + n - 4, ".txt") == 0)
        {
    files[count] = strdup(ent->d_name);
         count++;
    if (count >= 512) break;
        }
    }

    closedir(d);

    qsort(files, count, sizeof(char *), cmp_strings);

    char def_outdir[512];
    if (!outdir) 
    {
    char dir_base[256];
    get_basename_no_ext(indir, dir_base, sizeof(dir_base));
    snprintf(def_outdir, sizeof(def_outdir), "%s_%s_%s", dir_base, STUDENT_NAME, STUDENT_ID);
    outdir = def_outdir;
    }

    for (int i = 0; i < count; i++) 
    {
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s", indir, files[i]);
    process_single_file(filepath, outdir);
    free(files[i]);
    }
    return 0;
}

int main(int argc, char **argv) 
{
    const char *outdir = NULL;
    const char *indir = NULL;
    const char *infile = NULL;

    for (int i = 1; i < argc; i++) 
    {
        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output-dir") == 0) 
        {
            if (i + 1 < argc) outdir = argv[++i];
        } 
        else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dir") == 0) 
        {
            if (i + 1 < argc) indir = argv[++i];
        } 
        else if (argv[i][0] != '-') 
        {
            infile = argv[i];
        }
    }

    if (!indir && !infile) 
    {
    fprintf(stderr, "Usage: %s [-o OUTDIR] [-d INDIR] [input.txt]\n", argv[0]);
    return 1;
    }

    if (indir) 
    {
    return process_directory(indir, outdir);
    }
    return process_single_file(infile, outdir);
}