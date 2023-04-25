#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include "sudoku.h"
#include <time.h>
int unique;

Grid_T sudoku_read(void){
    int i = 0, j = 0;
    Grid_T g;
    char c;

    while((i < 9) && ((c = getchar()) != EOF)){
        if((c >= 48) && (c <= 57)){
            if((i < 9) && (j < 9)){
                grid_update_value(&g, i, j, c - '0');
                j++;
            }
            else{
                printf("error1\n");
                exit(-1);
            }
        }
        else if(c == 10){
            if(j < 9){
                printf("error2\n");
                exit(-2);
            }
            else{
                i++;
                j = 0;
            }
        }
    }
    if(i != 9){
        printf("error4\n");
        exit(-4);
    }
    return g;
}

void sudoku_print(FILE *s, Grid_T g){
    int i, j;

    for(i=0; i<9; i++){
        for(j=0; j<9; j++){
            fprintf(s, "%d", grid_read_value(g, i, j));
            if(j != 8){
                fprintf(s, " ");
            }
        }
        fprintf(s, "\n");
    }
}

void sudoku_print_errors(Grid_T g){
    int i, j, k, f, x;

    for(i=0; i<9; i++){
        for(j=0; j<9; j++){
            x = grid_read_value(g, i, j);
            for(k=0; k<9; k++){
                if((x == grid_read_value(g, i, k) && (j != k))){
                    printf("Number %d positioned in (%d, %d) same as (%d, %d).\n", x, i, j, i, k);
                }
                if((x == grid_read_value(g, k, j)) && (i != k)){
                    printf("Number %d positioned in (%d, %d) same as (%d, %d).\n", x, i, j, k, j);
                }
            }
            k = i - i%3;
            f = j - j%3;
            while(k%3){
                while(f%3){
                    if((x == grid_read_value(g, k, f)) && (i != k) && (j != f)){
                        printf("Number %d positioned in (%d, %d) same as (%d, %d).\n", x, i, j, k, f);
                    }
                    f++;
                }
                f = j - j%3;;
                k++;
            }
        }
    }    
}

int sudoku_is_correct(Grid_T g){
    int i, j, k, f, x;

    for(i=0; i<9; i++){
        for(j=0; j<9; j++){
            x = grid_read_value(g, i, j);
            for(k=0; k<9; k++){
                if((x == grid_read_value(g, i, k) && (j != k))){
                    return 0;
                }
                if((x == grid_read_value(g, k, j)) && (i != k)){
                    return 0;
                }
            }
            k = i - i%3;
            f = j - j%3;
            do{
                do{
                    if((x == grid_read_value(g, k, f)) && (i != k) && (j != f)){
                        return 0;
                    }
                    f++;
                }while(f%3);
                f = j - j%3;;
                k++;
            }while(k%3);
        }
    }
    return 1;
}

static void sudoku_init_choices(Grid_T *g){
    int i, j, k, f;

    assert(g);
    for(i=0; i<9; i++){
        for(j=0; j<9; j++){
            for(k=0; k<10; k++){
                grid_set_choice(g, i, j, k);
            }
            grid_set_count(g, i, j);
        }
    }
    for(i=0; i<9; i++){
        for(j=0; j<9; j++){
            if(!grid_read_value(*g, i, j)){
                for(k=0; k<9; k++){
                    if(grid_read_value(*g, i, k)){
                        grid_remove_choice(g, i, j, grid_read_value(*g, i, k));
                    }
                    if(grid_read_value(*g, k, j)){
                        grid_remove_choice(g, i, j, grid_read_value(*g, k, j));
                    }
                }
                k = i - i%3;
                f = j - j%3;
                do{
                    do{
                        if(grid_read_value(*g, k, f)){
                            grid_remove_choice(g, i, j, grid_read_value(*g, k, f));
                        }
                        f++;
                    }while(f%3);
                    f = j - j%3;
                    k++;
                }while(k%3);
            }
        }
    }
}

static int sudoku_try_next(Grid_T g, int *row, int *col){
    int valid, count = 0, i = 1, c;

    srand(time(NULL) + getpid());
    c = grid_read_count(g, *row, *col);
    if(!c){
        return 0;
    }
    valid = rand() % c + 1;
    while(count < valid){
        if(grid_choice_is_valid(g, *row, *col, i)){
            count++;
        }  
        i++;
    }
    return i -1;
}

static int sudoku_update_choice(Grid_T *g, int i, int j, int n){

    grid_remove_choice(g, i, j, n);
    return grid_read_count(*g, i, j);
}

static void sudoku_eliminate_choice(Grid_T *g, int r, int c, int n){
    int i, j;

    for(i=0; i<9; i++){
        grid_remove_choice(g, r, i, n);
        grid_remove_choice(g, i, c, n);
    }
    i = r - r%3;
    j = c - c%3;
    do{
        do{
            grid_remove_choice(g, i, j, n);
            j++;
        }while(j%3);
        j = c - c%3;
        i++;
    }while(i%3);
}

int grid_EmptyItems(Grid_T g){
    int i, j, count=0;

    for(i=0; i<9; i++){
        for(j=0; j<9; j++){
            if(!grid_read_value(g, i, j)){
                count++;
            }
        }
    }
    return count;
}

int grid_GetFirstValid(Grid_T g, int i, int j){
    int k;

    for(k=1; k<=9; k++){
        if(g.elts[i][j].choices.num[k] == 1){
            return k;
        }
    }
    return 0;
}

void sudoku_solveOneChoiceItems(Grid_T *g){
    int count, i, j, v;
    
    do{
        count=0;
        for(i=0; i<9; i++){
            for(j=0; j<9; j++){
                if(!grid_read_value(*g, i, j) && (grid_read_count(*g, i, j) == 1)){
                    v = grid_GetFirstValid(*g, i,j);
                    grid_update_value(g, i, j, v);
                    sudoku_update_choice(g, i, j, v);
                    sudoku_eliminate_choice(g, i, j, v);
                    count++;
                }
            }
        }
    } while(count);
}
    
Grid_T sudoku_solve(Grid_T g){
    int i,j,k, valid, gs;
    Grid_T tmp, sol, savedState;
    
    gs = grid_read_unique(g);
    if(gs > 1){
        return g;
    }
    if(grid_EmptyItems(g)){
        sudoku_solveOneChoiceItems(&g); 
    }
    if (!grid_EmptyItems(g)) {
        if (sudoku_is_correct(g)){
            grid_set_unique(&g);
        }
        return g;
    }
    grid_clear_unique(&sol);
    for(i=0; i<9; i++){
        for( j=0;j<9; j++){
            if(!grid_read_value(g, i, j)){ 
                tmp = g;
                do{
                    valid = sudoku_try_next(tmp, &i, &j);
                    if(!valid){
                        grid_clear_unique(&tmp);
                        return tmp;
                    }
                    savedState = tmp;
                    grid_update_value(&tmp, i, j, valid);
                    sudoku_update_choice(&tmp, i, j, valid);
                    sudoku_eliminate_choice(&tmp, i, j, valid);
                    tmp = sudoku_solve(tmp);
                    if((grid_read_unique(tmp) > gs) && !grid_read_unique(sol)){
                        sol = tmp;
                        for(k=1; k< grid_read_unique(tmp) - gs; k++){
                            grid_set_unique(&sol);
                        }                   
                    }
                    tmp = savedState;
                    grid_remove_choice(&tmp, i, j, valid);
                }while(grid_read_count(tmp, i, j) && (grid_read_unique(sol) <= 1));
                if(grid_read_unique(sol) > 1){
                    return sol;
                }
                if(grid_read_unique(sol) > gs){
                    return sol;
                }
                else{
                    return g;
                }
            }
        }
    }
    if(grid_read_unique(sol) > gs){
        return sol;
    }
    else{
        return g;
    }
}

int sudoku_solution_is_unique(Grid_T g){
    int i, j;

    for(i=0; i<9; i++){
        for(j=0; j<9; j++){
            if(grid_read_count(g, i, j)){
                return 0;
            }
        }
    }
    return 1;
}

static Grid_T sudoku_generate_complete(void){
    Grid_T g;
    int tmp[9][9] = {
        {1, 2, 6, 4, 5, 3, 8, 7, 9},
        {5, 7, 9, 2, 8, 6, 1, 3, 4},
        {3, 4, 8, 1, 7, 9, 5, 2, 6},
        {7, 5, 1, 9, 4, 8, 3, 6, 2},
        {6, 3, 2, 7, 1, 5, 4, 9, 8},
        {9, 8, 4, 6, 3, 2, 7, 5, 1},
        {4, 6, 5, 3, 2, 1, 9, 8, 7},
        {2, 1, 3, 8, 9, 7, 6, 4, 5},
        {8, 9, 7, 5, 6, 4, 2, 1, 3}
    };
    int i, j, valid, correct = 0, w = 0, k;

    for(i=0; i<=9; i++){
        for(j=0; j<=9; j++){
            grid_update_value(&g, i, j, 0);
        }
    }
    sudoku_init_choices(&g);
    while((w<10) && !correct){
        for(i=0; i<9; i++){
            for(j=0; j<9; j++){
                valid = sudoku_try_next(g, &i, &j);
                if(!valid){
                    srand(getpid());
                    valid = rand() % 10 + 1;
                }
                grid_update_value(&g, i, j, valid);
                sudoku_update_choice(&g, i, j, valid);
                sudoku_eliminate_choice(&g, i, j, valid);
            }
        }
        correct = sudoku_is_correct(g);
        w++;
    }
    if(correct != 1){
        for(i=0; i<9; i++){
            for(j=0; j<9; j++){
                grid_update_value(&g, i, j, tmp[i][j]);
                grid_clear_count(&g, i, j);
                for(k=0; k<=9; k++){
                    grid_clear_choice(&g, i , j, k);
                }
            }
        }
        grid_set_unique(&g);
    }
    return g;
}

int check_unique(Grid_T g, int i, int j){
    int k, f;

    if(!unique){
        return 1;
    }
    else{
        for(k=0; k<9; k++){
            if(!grid_read_value(g, i, k)){
                return 0;
            }
            if(!grid_read_value(g, k, j)){
                return 0;
            }
        }
        k = i - i%3;
        f = j - j%3;
        while(k%3){
            while(f%3){
                if(!grid_read_value(g, k, f)){
                    return 0;
                }
                f++;
            }
            f = j - j%3;
            k++;
        }
    }
    return 1;
}

Grid_T sudoku_generate(int nelts){
    int i, j, k;
    Grid_T g;

    srand(getpid());
    g = sudoku_generate_complete();
    for(k=0; k<81-nelts; k++){
        do{
            i = rand() % 9;
            j = rand() % 9;
        }while(!grid_read_value(g, i, j) && (check_unique(g, i, j)));
        grid_update_value(&g, i, j, 0);
    }
    return g;
}

int main(int argc, char *argv[]){
    Grid_T g;
    int i;

    if(argc == 1){
        g = sudoku_read();
        sudoku_print(stderr, g);
        sudoku_init_choices(&g);
        grid_clear_unique(&g);
        g = sudoku_solve(g);
        i = grid_read_unique(g);
        if(i == 1){
            fprintf(stderr, "unique solution\n");
            sudoku_print(stdout, g);
        }
        else if(i > 1){
            fprintf(stderr, "not unique solution\n");
            sudoku_print(stdout, g);
        }
        else{
            fprintf(stderr, "No solution\n");
            sudoku_print_errors(g);
        }
    }
    else if(argc == 2){
        g = sudoku_read();
        sudoku_print(stderr, g);
        if(!sudoku_is_correct(g)){
            fprintf(stderr, "sudoku is not correct\n");
            sudoku_print_errors(g);
        }
        else{
            fprintf(stderr, "sudoku is correct\n");
        }
    }
    else if(argc == 3){
        unique = 0;     /*I declare the unique as global variable because the TA said not to change sudoku.h*/
        g = sudoku_generate(atoi(argv[argc - 1]));
        sudoku_print(stdout, g);
    }
    else{
        unique = 1; 
        g = sudoku_generate(atoi(argv[argc - 1]));
        sudoku_print(stdout, g);
    }
    return 0;
}