#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int idx;
    int idy;
    double value;
} cell;

typedef struct {
    double val_max;
    int count;
} cell_max;

struct path {
    cell e;
    struct path* next;
};
typedef struct path path;

typedef struct {
    path* tete;
} cell_liste;

typedef struct {
    char* c1;
    char* c2;
    double similarity;
    int occurrences;
} AlignmentResult;


EMSCRIPTEN_KEEPALIVE double** allocation(int l1, int l2);
EMSCRIPTEN_KEEPALIVE void FillMatrix(int n, int m, const char* s1, const char* s2, double** mat);
EMSCRIPTEN_KEEPALIVE cell_max get_max(int n, int m, double** mat);
EMSCRIPTEN_KEEPALIVE cell precell(int i, int j, int n, int m, const char* s1, const char* s2, double** mat);
EMSCRIPTEN_KEEPALIVE cell_liste traceback(cell c, int n, int m, const char* s1, const char* s2, double** mat);
EMSCRIPTEN_KEEPALIVE AlignmentResult* alignment(cell_liste *chaine, int n, int m, const char* s1, const char* s2);
EMSCRIPTEN_KEEPALIVE void free_alignment_result(AlignmentResult* result);


EMSCRIPTEN_KEEPALIVE AlignmentResult* align_sequences(const char* s1, const char* s2);
EMSCRIPTEN_KEEPALIVE void free_matrix(double** mat, int rows);



double similarite(char s1, char s2) {
    return (s1 == s2) ? 1.0 : -0.33;
}

double find_maxQuatrecoeff(double a, double b, double c, double d) {
    double max = a;
    if (b > max) max = b;
    if (c > max) max = c;
    if (d > max) max = d;
    return max;
}

double find_maximum_K(int i, int j, int n, int m, double** mat) {
    double max = 0;
    for (int k = 1; k < i; k++) {
        double val = mat[i-k][j] - 1 - 0.33*k;
        if (val > max) max = val;
    }
    return max;
}

double find_maximum_L(int i, int j, int n, int m, double** mat) {
    double max = 0;
    for (int l = 1; l < j; l++) {
        double val = mat[i][j-l] - 1 - 0.33*l;
        if (val > max) max = val;
    }
    return max;
}

EMSCRIPTEN_KEEPALIVE
double** allocation(int l1, int l2) {
    double** mat = (double**)malloc((l1+1) * sizeof(double*));
    if (mat == NULL) {
        printf("Error allocating matrix\n");
        return NULL;
    }
    for (int i = 0; i < l1+1; i++) {
        mat[i] = (double*)malloc((l2+1) * sizeof(double));
        if (mat[i] == NULL) {
            printf("Error allocating matrix row\n");
            for (int j = 0; j < i; j++) free(mat[j]);
            free(mat);
            return NULL;
        }
    }
    return mat;
}

EMSCRIPTEN_KEEPALIVE
void FillMatrix(int n, int m, const char* s1, const char* s2, double** mat) {
    for (int i = 0; i < n+1; i++) mat[i][0] = 0.0;
    for (int j = 0; j < m+1; j++) mat[0][j] = 0.0;

    for (int i = 1; i < n+1; i++) {
        for (int j = 1; j < m+1; j++) {
            double a = mat[i-1][j-1] + similarite(s1[i-1], s2[j-1]);
            double b = find_maximum_K(i, j, n, m, mat);
            double c = find_maximum_L(i, j, n, m, mat);
            mat[i][j] = find_maxQuatrecoeff(a, b, c, 0);
        }
    }
}

EMSCRIPTEN_KEEPALIVE
cell_max get_max(int n, int m, double** mat) {
    cell_max mc = {0, 0};
    for (int i = 1; i < n; i++) {
        for (int j = 1; j < m; j++) {
            if (mat[i][j] > mc.val_max) {
                mc.val_max = mat[i][j];
                mc.count = 1;
            } else if (mat[i][j] == mc.val_max) {
                mc.count++;
            }
        }
    }
    return mc;
}

cell indicesMaxK(int i, int j, int n, int m, double** mat) {
    cell indices_maxK = {i, j, 0};
    for (int k = 1; k < i; k++) {
        double val = mat[i-k][j] - 1 - 0.33*k;
        if (val > indices_maxK.value) {
            indices_maxK.value = val;
            indices_maxK.idx = i-k;
        }
    }
    return indices_maxK;
}

cell indicesMaxL(int i, int j, int n, int m, double** mat) {
    cell indices_maxL = {i, j, 0};
    for (int l = 1; l < j; l++) {
        double val = mat[i][j-l] - 1 - 0.33*l;
        if (val > indices_maxL.value) {
            indices_maxL.value = val;
            indices_maxL.idy = j-l;
        }
    }
    return indices_maxL;
}

EMSCRIPTEN_KEEPALIVE
cell precell(int i, int j, int n, int m, const char* s1, const char* s2, double** mat) {
    double a = mat[i-1][j-1] + similarite(s1[i-1], s2[j-1]);
    cell bb = indicesMaxK(i, j, n, m, mat);
    cell cc = indicesMaxL(i, j, n, m, mat);
    double prevscore = find_maxQuatrecoeff(a, bb.value, cc.value, 0);

    cell origine = {0, 0, 0};
    if (prevscore == a) {
        origine.idx = i-1;
        origine.idy = j-1;
    } else if (prevscore == bb.value) {
        origine = bb;
    } else if (prevscore == cc.value) {
        origine = cc;
    }
    origine.value = mat[origine.idx][origine.idy];
    return origine;
}

void empiler(cell_liste *pile, cell p) {
    path* c = (path*)malloc(sizeof(path));
    if (c == NULL) {
        printf("Error allocating path\n");
        return;
    }
    c->e = p;
    c->next = pile->tete;
    pile->tete = c;
}

EMSCRIPTEN_KEEPALIVE
cell_liste traceback(cell c, int n, int m, const char* s1, const char* s2, double** mat) {
    cell_liste chaine = {NULL};
    while (c.value != 0) {
        empiler(&chaine, c);
        c = precell(c.idx, c.idy, n, m, s1, s2, mat);
    }
    return chaine;
}

cell depiler(cell_liste *pile) {
    if (pile == NULL || pile->tete == NULL) {
        printf("Error: Empty stack\n");
        return (cell){0, 0, 0};
    }
    path* supp_cell = pile->tete;
    cell p = supp_cell->e;
    pile->tete = supp_cell->next;
    free(supp_cell);
    return p;
}

EMSCRIPTEN_KEEPALIVE
AlignmentResult* alignment(cell_liste *chaine, int n, int m, const char* s1, const char* s2) {
    char* c1 = (char*)malloc((n+m+1) * sizeof(char));
    char* c2 = (char*)malloc((n+m+1) * sizeof(char));
    if (c1 == NULL || c2 == NULL) {
        printf("Error allocating alignment result\n");
        free(c1);
        free(c2);
        return NULL;
    }

    c1[0] = c2[0] = '\0';
    int d1 = chaine->tete->e.idx;
    int d2 = chaine->tete->e.idy;

    while (chaine->tete != NULL) {
        cell d = depiler(chaine);
        while (d1 < d.idx) {
            char a[2] = {s1[d1-1], '\0'};
            strcat(c1, a);
            strcat(c2, "-");
            d1++;
        }
        while (d2 < d.idy) {
            char b[2] = {s2[d2-1], '\0'};
            strcat(c2, b);
            strcat(c1, "-");
            d2++;
        }
        if (d1 == d.idx && d2 == d.idy) {
            char a[2] = {s1[d1-1], '\0'};
            char b[2] = {s2[d2-1], '\0'};
            strcat(c1, a);
            strcat(c2, b);
            d1++; d2++;
        }
    }

    AlignmentResult* result = (AlignmentResult*)malloc(sizeof(AlignmentResult));
    if (result == NULL) {
        printf("Error allocating AlignmentResult\n");
        free(c1);
        free(c2);
        return NULL;
    }
    result->c1 = c1;
    result->c2 = c2;

    return result;
}

EMSCRIPTEN_KEEPALIVE
void free_alignment_result(AlignmentResult* result) {
    if (result) {
        free(result->c1);
        free(result->c2);
        free(result);
    }
}

EMSCRIPTEN_KEEPALIVE
void free_matrix(double** mat, int rows) {
    for (int i = 0; i < rows; i++) {
        free(mat[i]);
    }
    free(mat);
}

EMSCRIPTEN_KEEPALIVE
AlignmentResult* align_sequences(const char* s1, const char* s2) {
    int l1 = strlen(s1);
    int l2 = strlen(s2);

    double** mat = allocation(l1, l2);
    if (mat == NULL) return NULL;

    FillMatrix(l1, l2, s1, s2, mat);

    cell_max mc = get_max(l1+1, l2+1, mat);

    cell cMax = {0, 0, mc.val_max};
    for (int i = 1; i < l1+1; i++) {
        for (int j = 1; j < l2+1; j++) {
            if (mat[i][j] == mc.val_max) {
                cMax.idx = i;
                cMax.idy = j;
                cell_liste chaine = traceback(cMax, l1+1, l2+1, s1, s2, mat);
                AlignmentResult* result = alignment(&chaine, l1+1, l2+1, s1, s2);
                if (result) {
                    result->similarity = mc.val_max;
                    result->occurrences = mc.count;
                }
                free_matrix(mat, l1+1);
                return result;
            }
        }
    }

    free_matrix(mat, l1+1);
    return NULL;
}