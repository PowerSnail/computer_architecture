/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * Please fill in the following team struct 
 */
team_t team = {
    "PowerSnail",              /* Team name */

    "Han Jin",     /* First member full name */
    "hj5fb@virginia.edu",  /* First member email address */

    "",                   /* Second member full name (leave blank if none) */
    ""                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

char rotate_u2_des[] = "rotate: unroll_2";
void rotate_u2(int dim, pixel *src, pixel *dst)
{
    int i, j;

    for (i = 0; i < dim; i++)
		{
				for (j = 0; j < dim - 1; j += 2)
				{
						dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
						dst[RIDX(dim-2-j, i, dim)] = src[RIDX(i, j + 1, dim)];
				}
		}
}


char rotate_u4_des[] = "rotate: unroll_4";
void rotate_u4(int dim, pixel *src, pixel *dst)
{
    int i, j;

    for (i = 0; i < dim; i++)
		{
				for (j = 0; j < dim; j += 4)
				{
						dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
						dst[RIDX(dim-2-j, i, dim)] = src[RIDX(i, j + 1, dim)];
						dst[RIDX(dim-3-j, i, dim)] = src[RIDX(i, j + 2, dim)];
						dst[RIDX(dim-4-j, i, dim)] = src[RIDX(i, j + 3, dim)];
				}
		}
}

char rotate_with_ptr_des[] = "rotate: with ptr 4x4";
inline void rotate_with_ptr(int dim, pixel *src, pixel *dst)
{
		*(dst + 3 * dim) = *(src);
		*(dst + 2 * dim) = *(src + 1);
		*(dst + 1 * dim) = *(src + 2);
		*(dst) = *(src + 3);

		src += dim;
		dst++;
		*(dst + 3 * dim) = *(src);
		*(dst + 2 * dim) = *(src + 1);
		*(dst + 1 * dim) = *(src + 2);
		*(dst) = *(src + 3);

		src += dim;
		dst++;
		*(dst + 3 * dim) = *(src);
		*(dst + 2 * dim) = *(src + 1);
		*(dst + 1 * dim) = *(src + 2);
		*(dst) = *(src + 3);

		src += dim;
		dst++;
		*(dst + 3 * dim) = *(src);
		*(dst + 2 * dim) = *(src + 1);
		*(dst + 1 * dim) = *(src + 2);
		*(dst) = *(src + 3);
}




/* 
 * rotate - Another version of rotate
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
		int dim2 = dim * 2;
		int dim3 = dim * 3;
		int dim_bound = dim * dim - dim;
		int i, j;
		for (i = 0; i < dim - 3; i += 4)
				for (j = 0; j < dim - 3; j += 4)
				{
						int d = dim_bound - j * dim + i;
						int s = i * dim + j; 
				
						dst[d] = src[s];
						dst[d - dim] = src[s + 1];
						dst[d - dim2] = src[s + 2];
						dst[d - dim3] = src[s + 3];

						d++; s += dim;
						dst[d] = src[s];
						dst[d - dim] = src[s + 1];
						dst[d - dim2] = src[s + 2];
						dst[d - dim3] = src[s + 3];

						d++; s += dim;
						dst[d] = src[s];
						dst[d - dim] = src[s + 1];
						dst[d - dim2] = src[s + 2];
						dst[d - dim3] = src[s + 3];

						d++; s += dim;
						dst[d] = src[s];
						dst[d - dim] = src[s + 1];
						dst[d - dim2] = src[s + 2];
						dst[d - dim3] = src[s + 3];
				}



}

/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{
    add_rotate_function(&naive_rotate, naive_rotate_descr);  
		add_rotate_function(&rotate_u4, rotate_u4_des);	
    add_rotate_function(&rotate, rotate_descr);   
    /* ... Register additional test functions here */
}


/***************
 * SMOOTH KERNEL
 **************/

/***************************************************************
 * Various typedefs and helper functions for the smooth function
 * You may modify these any way you like.
 **************************************************************/

/* A struct used to compute averaged pixel value */
typedef struct {
    int red;
    int green;
    int blue;
    int num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
static int min(int a, int b) { return (a < b ? a : b); }
static int max(int a, int b) { return (a > b ? a : b); }

/* 
 * initialize_pixel_sum - Initializes all fields of sum to 0 
 */
static void initialize_pixel_sum(pixel_sum *sum) 
{
    sum->red = sum->green = sum->blue = 0;
    sum->num = 0;
    return;
}

/* 
 * accumulate_sum - Accumulates field values of p in corresponding 
 * fields of sum 
 */
static void accumulate_sum(pixel_sum *sum, pixel p) 
{
    sum->red += (int) p.red;
    sum->green += (int) p.green;
    sum->blue += (int) p.blue;
    sum->num++;
    return;
}

/* 
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel 
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum) 
{
    current_pixel->red = (unsigned short) (sum.red/sum.num);
    current_pixel->green = (unsigned short) (sum.green/sum.num);
    current_pixel->blue = (unsigned short) (sum.blue/sum.num);
    return;
}

/* 
 * avg - Returns averaged pixel value at (i,j) 
 */
static pixel avg(int dim, int i, int j, pixel *src) 
{
    int ii, jj;
    pixel_sum sum;
    pixel current_pixel;

    initialize_pixel_sum(&sum);
    for(ii = max(i-1, 0); ii <= min(i+1, dim-1); ii++) 
	for(jj = max(j-1, 0); jj <= min(j+1, dim-1); jj++) 
	    accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);

    assign_sum_to_pixel(&current_pixel, sum);
    return current_pixel;
}

/******************************************************
 * Your different versions of the smooth kernel go here
 ******************************************************/

/*
 * naive_smooth - The naive baseline version of smooth 
 */
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(i, j, dim)] = avg(dim, i, j, src);
}

/*
 * smooth - Another version of smooth. 
 */
char smooth_descr[] = "smooth: Current working version";
void smooth(int dim, pixel *src, pixel *dst) 
{
		int i = 0, j = 0;	
		pixel_sum sum;

		i = 0; j = 0;
		{
						initialize_pixel_sum(&sum);
						accumulate_sum(&sum, src[RIDX(i + 1, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i + 1, j + 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j + 1, dim)]);				
						assign_sum_to_pixel(&(dst[RIDX(i, j, dim)]), sum);
		}
		for (j = 1; j < dim - 1; ++j)
		{
						initialize_pixel_sum(&sum);
						accumulate_sum(&sum, src[RIDX(i + 1, j - 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i + 1, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i + 1, j + 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j - 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j + 1, dim)]);					
						assign_sum_to_pixel(&(dst[RIDX(i, j, dim)]), sum);
		}
		j = dim - 1;
		{
						initialize_pixel_sum(&sum);
						accumulate_sum(&sum, src[RIDX(i + 1, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i + 1, j - 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j - 1, dim)]);					
						assign_sum_to_pixel(&(dst[RIDX(i, j, dim)]), sum);
		}


		for (i = 1; i < dim - 1; ++i)
		{
				j = 0;
				{
						initialize_pixel_sum(&sum);
						accumulate_sum(&sum, src[RIDX(i - 1, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i - 1, j + 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i + 1, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i + 1, j + 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j + 1, dim)]);					
						assign_sum_to_pixel(&(dst[RIDX(i, j, dim)]), sum);
				}
				for (j = 1; j < dim - 1; ++j)
				{
						initialize_pixel_sum(&sum);
						accumulate_sum(&sum, src[RIDX(i - 1, j - 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i - 1, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i - 1, j + 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i + 1, j - 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i + 1, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i + 1, j + 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j - 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j + 1, dim)]);					
						assign_sum_to_pixel(&(dst[RIDX(i, j, dim)]), sum);
				}
				j = dim - 1;
				{
						initialize_pixel_sum(&sum);
						accumulate_sum(&sum, src[RIDX(i - 1, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i - 1, j - 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i + 1, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i + 1, j - 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j - 1, dim)]);					
						assign_sum_to_pixel(&(dst[RIDX(i, j, dim)]), sum);
				}
		}

		i = dim - 1; j = 0;
		{
						initialize_pixel_sum(&sum);
						accumulate_sum(&sum, src[RIDX(i - 1, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i - 1, j + 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j + 1, dim)]);					
						assign_sum_to_pixel(&(dst[RIDX(i, j, dim)]), sum);
		}
		for (j = 1; j < dim - 1; ++j)
		{
						initialize_pixel_sum(&sum);
						accumulate_sum(&sum, src[RIDX(i - 1, j - 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i - 1, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i - 1, j + 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j - 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j + 1, dim)]);					
						assign_sum_to_pixel(&(dst[RIDX(i, j, dim)]), sum);
		}
		i = dim - 1; j = dim - 1;  
		{
						initialize_pixel_sum(&sum);
						accumulate_sum(&sum, src[RIDX(i - 1, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i - 1, j - 1, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j, dim)]);					
						accumulate_sum(&sum, src[RIDX(i, j - 1, dim)]);					
						assign_sum_to_pixel(&(dst[RIDX(i, j, dim)]), sum);
		}



}


/********************************************************************* 
 * register_smooth_functions - Register all of your different versions
 *     of the smooth kernel with the driver by calling the
 *     add_smooth_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_smooth_functions() {
    add_smooth_function(&smooth, smooth_descr);
    add_smooth_function(&naive_smooth, naive_smooth_descr);
    /* ... Register additional test functions here */
}

