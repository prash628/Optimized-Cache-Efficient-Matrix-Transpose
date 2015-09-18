/*
*  Name: Prashant Srinivasan
*  Andrew ID: psriniv1
*/
/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"
#include "contracts.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose_func64(int M, int N, int A[N][M], int B[M][N]);
void transpose_func32(int M, int N, int A[N][M], int B[M][N]);
void transpose_func6167(int M, int N, int A[N][M], int B[M][N]);
/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. The REQUIRES and ENSURES from 15-122 are included
 *     for your convenience. They can be removed if you like.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M==32)
    {
        transpose_func32(M,N,A,B);
    }
    if(M==64)
    {
        transpose_func64(M,N,A,B);
    }
    if(M==61)
    {
        transpose_func6167(M,N,A,B);
    }
}


/*
* Purpose: The function computes the transpose of a 32 by 32 matrix
*
* Procedure:
* To reduce the amount of conflict misses, the function virtually divides
* the 8 by 8 matrix into quadrants and works on them distinctly and
* appropriately so as to reduce conflicting cache lines that map to the
* same set. The complete working procedure is explained within the function.
*
* Legend:
* The order of the quadrants are as follows:
* 1 2
* 3 4
*/
void transpose_func32(int M, int N, int A[M][N], int B[N][M])
{
    int i, j;
    int p,q;
    int temp;

    for(i=0; i<M; i+=8)
    {
        for(j=0; j<N; j+=8)
        {
            //Step 1: A3 to B2
            for(p=i+7; (p>(i+3))&&p<M; p--)
            {
                for(q=j+3; (q>=j)&&q<N; q--)
                {
                    B[q][p]=A[p][q];
                }
            }
            //Step 2: A4 to B1 in requisite order
            for(p=i+7; (p>(i+3))&&p<M; p--)
            {
                for(q=j+4; (q<=(j+7))&&q<N; q++)
                {
                    B[q-4][p-4]=A[p][q];
                }
            }
            //Step 3: A2 to B3
            for(p=i; (p<=(i+3))&&p<M; p++)
            {
                for(q=j+4; (q<=(j+7))&&q<N; q++)
                {
                    B[q][p]=A[p][q];
                }
            }
            //Step 4: A1 to B4 in requisite order
            for(p=i; (p<=(i+3))&&p<M; p++)
            {
                for(q=j; (q<=(j+3))&&q<N; q++)
                {
                    B[q+4][p+4]=A[p][q];
                }
            }
            //Step 5: Swap B1 and B4-direct swap
            for(p=i; (p<=(i+3))&&p<M; p++)
            {
                for(q=j; (q<=(j+3))&&q<N; q++)
                {
                    temp=B[p+4+(j-i)][q+4+(i-j)];
                    B[p+4+(j-i)][q+4+(i-j)]=B[p+(j-i)][q+(i-j)];
                    B[p+(j-i)][q+(i-j)]=temp;
                }
            }
        }

    }
}


/*
* Purpose: The function computes the transpose of a 64 by 64 matrix
*
* Procedure:
* To reduce the amount of conflict misses, the function virtually divides
* the 8 by 8 matrix into quadrants and works on them distinctly and
* appropriately so as to reduce conflicting cache lines that map to the
* same set. The complete working procedure is explained within the function.
*
* Footnote: It may appear that a lot of similar code could be abstracted away
* into  small helper functions. But, I feel that doing that would
* reduce readability as some of the functions might require passing a lot
* of parameters and it would not fit within the 80 chars per line limit.
* Hence, the decision to keep it within this function itself was taken.
* Also, the main codes in question are just 1-5 lines long.
*
*/
void transpose_func64(int M, int N, int A[N][M], int B[N][M])
{
    int i,j,p,q;
    int xDash,yDash;  // These variables help for translating the matrix 
    int temp;

    for(i=0; i<M; i+=8)
    {
        for(j=0; j<N; j+=8)
        {
            xDash=j-i;
            yDash=i-j;
            //A's 1,2 rows to B's 3,4
            for(p=i; p<(i+2); p++)
            {
                for(q=j; q<(j+8); q++)
                {
                    B[p+2+xDash][q+yDash]=A[p][q];
                }
            }
            //A's 3,4 rows to B's 1,2
            for(p=i+2; p<(i+4); p++)
            {
                for(q=j; q<(j+8); q++)
                {
                    B[p-2+xDash][q+yDash]=A[p][q];
                }
            }
            //Now, re-translate in B
            //B rows 1,2 and 3,4
            for(p=i; p<(i+2); p++)
            {
                for(q=j; q<(j+8); q++)
                {
                    temp=B[p+xDash][q+yDash];
                    B[p+xDash][q+yDash]=B[p+2+xDash][q+yDash];
                    B[p+2+xDash][q+yDash]=temp;
                }
            }
            //Now transpose the first 4 by 4 quadrant of upper half
            for(p=i+xDash; p<(i+4+xDash); p++)
            {
                for(q=j+yDash; q<(j+4+yDash); q++)
                {
                    if((q-(j+yDash))<(p-(i+xDash)))
                    {
                        temp=B[p][q];
                        B[p][q]=B[q+xDash][p+yDash];
                        B[q+xDash][p+yDash]=temp;
                    }
                }
            }
            //Now transpose the second 4 by 4 quadrant of upper half
            for(p=i+xDash; p<(i+4+xDash); p++)
            {
                for(q=j+4+yDash; q<(j+8+yDash); q++)
                {
                    if((q-(j+4+yDash))<(p-(i+xDash)))
                    {
                        temp=B[p][q];
                        B[p][q]=B[q+xDash-4][p+yDash+4];
                        B[q+xDash-4][p+yDash+4]=temp;
                    }
                }
            }
            //Swap the second quadrant of B's 12 with 34
            //B rows 1,2 and 3,4
            for(p=i; p<(i+2); p++)
            {
                for(q=j+4; q<(j+8); q++)
                {
                    temp=B[p+xDash][q+yDash];
                    B[p+xDash][q+yDash]=B[p+2+xDash][q+yDash];
                    B[p+2+xDash][q+yDash]=temp;
                }
            }


            //Now repeat the same for the lower half
            //A's 5,6 rows to B's 7,8
            for(p=i+4; p<(i+6); p++)
            {
                for(q=j; q<(j+8); q++)
                {
                    B[p+2+xDash][q+yDash]=A[p][q];
                }
            }
            //A's 7,8 rows to B's 5,6
            for(p=i+6; p<(i+8); p++)
            {
                for(q=j; q<(j+8); q++)
                {
                    B[p-2+xDash][q+yDash]=A[p][q];
                }
            }
            //Now re translate in B
            //B rows 5,6 and 7,8
            for(p=i+4; p<(i+6); p++)
            {
                for(q=j; q<(j+8); q++)
                {
                    temp=B[p+xDash][q+yDash];
                    B[p+xDash][q+yDash]=B[p+2+xDash][q+yDash];
                    B[p+2+xDash][q+yDash]=temp;
                }
            }
            //Now transpose the first 4 by 4 quadrant of the lower half
            for(p=i+4+xDash; p<(i+8+xDash); p++)
            {
                for(q=j+yDash; q<(j+4+yDash); q++)
                {
                    if((q-(j+yDash))<(p-(i+4+xDash)))
                    {
                        temp=B[p][q];
                        B[p][q]=B[q+xDash+4][p+yDash-4];
                        B[q+xDash+4][p+yDash-4]=temp;
                    }
                }
            }
            //Now transpose the second 4 by 4 quadrant of lower half
            for(p=i+4+xDash; p<(i+8+xDash); p++)
            {
                for(q=j+4+yDash; q<(j+8+yDash); q++)
                {
                    if((q-(j+4+yDash))<(p-(i+4+xDash)))
                    {
                        temp=B[p][q];
                        B[p][q]=B[q+xDash][p+yDash];
                        B[q+xDash][p+yDash]=temp;
                    }
                }
            }
            //Swap the first quadrant of B's 56 with 78
            //B rows 5,6 and 7,8
            for(p=i+4; p<(i+6); p++)
            {
                for(q=j+0; q<(j+4); q++)
                {
                    temp=B[p+xDash][q+yDash];
                    B[p+xDash][q+yDash]=B[p+2+xDash][q+yDash];
                    B[p+2+xDash][q+yDash]=temp;
                }
            }


            /*
            *Consider Quadrants to be defined as :
            * 1 2
            * 3 4
            */
            /*Swap the:
            * upper half of the third quadrant  with the
            * lower half of the second quadrant
            */
            for(p=i+4; p<(i+6); p++)
            {
                for(q=j+0; q<(j+4); q++)
                {
                    temp=B[p+xDash][q+yDash];
                    B[p+xDash][q+yDash]=B[p-2+xDash][q+4+yDash];
                    B[p-2+xDash][q+4+yDash]=temp;
                }
            }
            /*Now, swap the:
            * lower half of the third quadrant with the
            * upper half of the second quadrant
            */
            for(p=i+6; p<(i+8); p++)
            {
                for(q=j+0; q<(j+4); q++)
                {
                    temp=B[p+xDash][q+yDash];
                    B[p+xDash][q+yDash]=B[p-6+xDash][q+4+yDash];
                    B[p-6+xDash][q+4+yDash]=temp;
                }
            }
        }
    }

}



/*The function transposes 61 by 67 matrix by saving the diagonal elements
 *in temp, which ensures that A and B's
 *cache lines are not accessed at the same time, thereby reducing the
 *number of misses. This is because they map to the same set.
 */
/*Having the block size 18 makes it within the  limit of 2000
 *The number was arrived at by trial and error
 *The random nature of the number is what probably helped.
*/
void transpose_func6167(int M, int N, int A[N][M], int B[M][N])
{
    int i,j,p,q;
    int temp;

    for(i=0; i<M; i+=18)
    {
        for(j=0; j<N; j+=18)
        {
            for(p=i; (p<(i+18)) && (p<M); p++)
            {
                for(q=j; (q<(j+18)) && q<N; q++)
                {
                    if((p)==(q))
                    {
                        // Only for diagonal elements, do we have the problem
                        temp=A[p][p];
                    }
                    else
                    {
                        B[p][q]=A[q][p];
                    }
                }
                if(i==j)
                {
                    /*Accessing B's set here is better because it reduces the
                     *number of conflict misses
                     */
                    B[p][p]=temp;
                }

            }

        }
    }

}


/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i,j,tmp;
    REQUIRES(M > 0);
    REQUIRES(N > 0);
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
    ENSURES(is_transpose(M, N, A, B));
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);

}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}




