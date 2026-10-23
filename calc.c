// Tokhirbek Akramov 251ADB079
// https://github.com/tohir-cs/calc-lab-home
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: Gemini ai -- link: https://share.gemini.google/5HAMwiYzEvIU

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#define STUDENT_NAME "tokhirbek"
#define STUDENT_LASTNAME "akramov"
#define STUDENT_ID "251ADB079"

//skip spaces, tabs
static const char *skip_spaces(const char *p) {
    while (*p == ' ' || *p == '\t')
    {
    p++;
    }
    return p;
}

// simple integer evaluator for + and (Grade 4)
static long long evaluate_line(const char *line) {
    const char *p = skip_spaces(line);
    long long total = 0;

//read first number
    char *endptr;
    total = strtoll(p, &endptr, 10);
    p = skip_spaces(endptr);

//read subsequent operator and numbers
    while (*p == '+' || *p == '-')
    {
    char op = *p++;
    p = skip_spaces(p);
    long long next_val = strtoll(p, &endptr, 10);
    p = skip_spaces(endptr);
        
    if (op == '+') {
        total += next_val;
    } else
    {
        total -= next_val;
    }
    }
    return total;
}



//AI; prompt "how do I extract filename without directory path and without .txt extension from a string in C"
// link: https://share.gemini.google/5HAMwiYzEvIU
//used strrchr and bounds checked slice copy to strip path and .txt extension

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

    FILE *fin = fopen(infile, "r");
    if (!fin)

    {
        fprintf(stderr, "Cannot open %s\n", infile);
        return 1;
    }


    FILE *fout = fopen(outpath, "w");
    if (!fout)
    {
        fclose(fin);
        return 1;
    }


    char buf[1024];
    while (fgets(buf, sizeof(buf), fin)) 
    {
//skip completely empty lines
        const char *p = skip_spaces(buf);
        if (*p == '\n' || *p == '\0') continue;

        long long res = evaluate_line(buf);
        fprintf(fout, "%lld\n", res);
    }
    fclose(fin);
    fclose(fout);
    return 0;
}