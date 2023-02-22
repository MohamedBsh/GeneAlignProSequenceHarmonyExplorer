#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    int idx;
    int idy;
    double value;
} cell;

typedef struct
{
	double val_max;
	int count;
} cell_max;

struct path
{
    cell e;
    struct path* next;
};
typedef struct path path;

typedef struct
{
    path* tete;
} cell_liste;

double** allocation(int l1, int l2); // dynamic allocation of our notation matrix.
double similarite (char s1, char s2); // returns a similarity coefficient between two characters.
double find_maxQuatrecoeff (double a, double b, double c, double d); // returns the maximum coefficient between 4 values (according to the Smith-Waterman calculation method).
double find_maximum_K (int i, int j, int n, int m, double** mat); // returns the max among the left row of mat[i][j], according to some deletion coefficient.
double find_maximum_L (int i, int j, int n, int m, double** mat); // returns the max among the top column of mat[i][j], according to a certain deletion coefficient.
void FillMatrix (int n, int m, char s1[n], char s2[m], double** mat); // fills the similarity matrix.
void ShowMatrix (int n, int m, char s1[n], char s2[m], double** mat); // displays the similarity matrix.

cell indicesMaxK (int i ,int j, int n, int m, double** mat); // retrieve the indices (i-r,j) of the max & its value from the row on the left of mat[i][j], according to a certain deletion coefficient.
cell indicesMaxL (int i, int j, int n, int m, double** mat); // retrieve the indices (i,j-r) of the max & its value from the top column of mat[i][j], according to a certain deletion coefficient.
cell_max get_max(int n, int m,double** mat); // retrieve the maximum and count its number of occurrences. We store this information in cell_max.
cell precell (int i, int j, int n, int m, char s1[n], char s2[m], double** mat); // calculate the original/parent cell.
cell_liste traceback (cell c, int n, int m, char s1[n], char s2[m], double** mat); // return the traceback.
void stack(cell_liste *pile, cell p); // add a cell in the stack which contains the traceback.
cell depiler (cell_liste *pile); // delete a cell in the stack that contains the traceback.
void alignment (cell_liste *chaine, int n, int m, char s1[n], char s2[m]); // display the alignments with respect to our stack.

int main(int argc, char *argv[])
{
    if(argc<=1)
    {
        printf("No file.\n");
        return -1;
    }
    FILE* pf = fopen(argv[1],"rt");
    if(pf==NULL)
    {
        printf("Error.\n");
        exit(1);
    }

    char c;
    int l1=0,l2=0;
    while((c=fgetc(pf))!='\n' && c!=EOF)
        l1++;
    while((c=fgetc(pf))!='\n' && c!=EOF)
        l2++;

    rewind(pf);

    char s1[l1+1];
    int i=0;
    while((c=fgetc(pf))!='\n' && c!=EOF)
    {
        s1[i]=c;
        i++;
    }
    s1[l1]='\0';

    char s2[l2+1];
    i=0;
    while((c=fgetc(pf))!='\n' && c!=EOF)
    {
        s2[i]=c;
        i++;
    }
    s2[l2]='\0';

    if(strcmp(s1,"")==0 || strcmp(s2,"")==0)
    {
        printf("Chain is empty\n");
        exit(1);
    }

    printf("\n");
    printf("Input :");
    printf("\nSequence 1 (%d) : %s\nSequence 2 (%d) : %s\n", l1,s1, l2,s2);

    fclose(pf);

    double ** mat;
    mat = allocation(l1+1,l2+1);

    for(int i=0; i<l1+1; i++)
    {
        mat[i][0]=0.0;
    }
    for(int j=0; j<l2+1; j++)
    {
        mat[0][j]=0.0;
    }
    FillMatrix(l1+1,l2+1,s1,s2,mat);
    printf("\nNotation Matrix - mat[i][j] : \n\n");
    ShowMatrix(l1+1,l2+1,s1,s2,mat);

    cell_max mc = get_max(l1+1,l2+1,mat);
    printf("\nMaximal similarity degree  %.1f . Number of occurences = %d.\n",mc.val_max,mc.count);
    printf("\n");

/********************************** Traceback ******************************/

	cell cMax;
	for (int i=1; i<l1+1; i++)
		for (int j=1; j<l2+1; j++)
			if (mat[i][j] == mc.val_max)
			{
				cMax.idx = i; cMax.idy = j;
				cell_liste chaine = traceback(cMax,l1+1,l2+1,s1,s2,mat);
				printf("\n");
				alignment(&chaine,l1+1,l2+1,s1,s2);
			}

    for (int i=0; i<l1+1; i++) {
        if(l2>0)
            free(mat[i]);
    }
    if(l1>0)
        free(mat);

    return 0;
}
/*************************************** Notation Matrix ***************************************/

double** allocation(int l1, int l2)
{
    double ** mat;
    mat = (double**) malloc ((l1+1)*sizeof(double*));
    if (mat == NULL)
    {
        printf("Error\n");
        exit(1);
    }
    for (int i=0; i<l1+1; i++) {
        mat[i] = (double*) malloc ((l2+1)*sizeof(double));
      if (mat[i] == NULL)
      {
          printf("Error\n");
          exit(1);
      }
    }
    return mat;
}

double similarite (char s1, char s2)
{
    if (s1==s2)
        return 1.0;
    return -0.33;
}

double find_maxQuatrecoeff (double a, double b, double c, double d)
{
    if(a>=b && a>=c && a>=d)
    {
        return a;
    }
    if(b>=a && b>=c && b>=d)
    {
        return b;
    }
    if(c>=a && c>=b && c>=d)
    {
        return c;
    }
    return d;
}

double find_maximum_K (int i, int j, int n, int m, double** mat)
{
    double val, max = 0;
    for (int k=1; k<i; k++)
    {
        val=mat[i-k][j] - 1 - 0.33*k;
        if(max<val)
            max = val;
    }
    return max;
}

double find_maximum_L (int i, int j, int n, int m, double** mat)
{
    double val, max = 0;
    for (int l=1; l<j; l++)
    {
        val=mat[i][j-l] - 1 - 0.33*l;
        if(max<val)
            max = val;
    }
    return max;
}


void FillMatrix (int n, int m, char s1[n], char s2[m], double** mat)
{
    double a,b,c,d=0;
    for (int i=1; i<n+1; i++)
    {
        for (int j=1; j<m+1; j++)
        {
            a = mat[i-1][j-1] + similarite (s1[i-1],s2[j-1]);
            b = find_maximum_K(i,j,n,m,mat);
            c = find_maximum_L(i,j,n,m,mat);
            mat[i][j] = find_maxQuatrecoeff(a,b,c,d);
        }
    }
}

void ShowMatrix (int n, int m, char s1[n], char s2[m], double** mat)
{
    printf("       ");
    for (int j=1; j<m; j++) printf("%c   ",s2[j-1]);
		printf("\n");

    for (int i=0; i<n; i++)
    {
        for (int j=0; j<m; j++)
        {
					if(j==0 && i>0)
						printf("%c ",s1[i-1]);
					else if(j==0 && i==0)
						printf("  ");

		      printf("%.1f ", mat[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

cell_max get_max(int n , int m, double** mat)
{
    cell_max mc;
    int i, j;
    mc.count= 0;
    double max=mat[0][0];

    for(i=1; i<n; i++)
    {
        for(j=1; j<m; j++)
        {
            if(mat[i][j]>max)
            {
                max = mat[i][j];
            }
        }
    }
    mc.val_max = max;


    for(i=1; i<n; i++)
    {
        for(j=1; j<m; j++)
        {
            if(mat[i][j]==mc.val_max)
            {
                mc.count++;
            }
        }
    }

    return mc;
}

cell indicesMaxK (int i,int j, int n,int m, double** mat)
{
    cell indices_maxK;
    int r=1;
    double max=0;
    double val;

    for (int k=1; k<i; k++)
    {
        val = mat[i-k][j] - 1 - 0.33*k;
        if (max<val)
        {
            max = val;
            r = k;
        }
    }

    indices_maxK.idx = i-r;
    indices_maxK.idy = j;
    indices_maxK.value = max;

    return indices_maxK;
}

cell indicesMaxL (int i,int j, int n,int m, double** mat)
{
    cell indices_maxL;
    int r=1;
    double max=0;
    double val;
    for (int l=1; l<j; l++)
    {
        val = mat[i][j-l] - 1 - 0.33*l;
        if (max<val)
        {
            max = val;
            r = l;
        }
    }

    indices_maxL.idx = i;
    indices_maxL.idy = j-r;
    indices_maxL.value = max;

    return indices_maxL;
}

cell precell (int i,int j, int n,int m, char s1[n], char s2[m], double** mat)
{
    cell origine;

    double a = mat[i-1][j-1] + similarite(s1[i-1], s2[j-1]);
    double d = 0;

    cell bb = indicesMaxK(i,j, n,m,mat);
    cell cc = indicesMaxL(i,j, n,m,mat);

    double prevscore = find_maxQuatrecoeff(a, bb.value, cc.value, d);

    if (prevscore == a)
    {
        origine.idx = i-1;
        origine.idy = j-1;
        origine.value = mat[origine.idx][origine.idy];
    }
    else if (prevscore == bb.value)
    {
        origine.idx = bb.idx;
        origine.idy = bb.idy;
        origine.value = mat[origine.idx][origine.idy];
    }
    else if (prevscore == cc.value)
    {
        origine.idx = cc.idx;
        origine.idy = cc.idy;
        origine.value = mat[origine.idx][origine.idy];
    }
    else
    {
        origine.idx = 0;
        origine.idy = 0;
        origine.value = 0;
    }

    return origine;
}

void empiler(cell_liste *pile, cell p)
{
    path* c = (path*) malloc(sizeof(path));
    if(c==NULL)
    {
        printf("Error\n");
        exit(1);
    }
    c->e = p;
    c->next = pile->tete;
    pile->tete = c;
}

cell_liste traceback (cell c, int n,int m, char s1[n], char s2[m], double** mat)
{
    cell_liste chaine;
    chaine.tete = NULL;

    while (c.value != 0)
    {
        empiler(&chaine, c);
        c = precell(c.idx,c.idy, n,m,s1,s2,mat);
    }

    return chaine;
}

cell depiler (cell_liste *pile)
{

    path* supp_cell = pile->tete;
    if(pile == NULL || pile->tete ==NULL)
    {
        printf("Error.\n");
        exit(1);
    }
    cell p = supp_cell->e;
    pile->tete = supp_cell->next;
    free(supp_cell);

    return p;
}

void alignment(cell_liste *chaine,int n, int m, char s1[n],char s2[m])
{

    char c1[n+m];
    char c2[n+m];

    strcpy(c1 , "");  strcpy(c2 , "");

    int d1 = chaine->tete->e.idx;
    int d2 = chaine->tete->e.idy;

    while(chaine->tete!=NULL)
    {
        cell d = depiler(chaine);
        while(d1<d.idx)
        {
            char a[2] = {s1[d1-1]};
            strcat(c1,a);
            strcat(c2,"-");
            d1++;
        }

        while(d2<d.idy)
        {
            char b[2] = {s2[d2-1]};
            strcat(c2, b);
            strcat(c1,"-");
            d2++;
        }

        if(d1==d.idx && d2==d.idy)
        {
            char a[2] = {s1[d1-1]};
            char b[2] = {s2[d2-1]};
            strcat(c1,a);
            strcat(c2,b);
            d1++; d2++;
        }
    }

    printf("Output :\n");
    printf("%s\n%s\n", c1,c2);
}