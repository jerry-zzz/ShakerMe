#include "Filter.h"
#include <QDebug>
#include<qfile.h>

FilterFunc::FilterFunc()
{

}

#include <stdlib.h>
double pi = 3.1415926535897932384626433832795L;
#if 0
#include "complex.h"
#else
typedef struct _C_double_complex
{
    double _Val[2];
} _C_double_complex;

_C_double_complex cexp(_C_double_complex x)
{
    _C_double_complex r;
    double p;
    p = exp(x._Val[0]);
    r._Val[0] = p*cos(x._Val[1]);
    r._Val[1] = p*sin(x._Val[1]);
    return r;
}

_C_double_complex conj(_C_double_complex x)
{
    _C_double_complex r;
    r._Val[0] = x._Val[0];
    r._Val[1] = -x._Val[1];
    return r;
}

_C_double_complex _Cmulcr(_C_double_complex x, double y)
{
    _C_double_complex r;
    r._Val[0] = x._Val[0] * y;
    r._Val[1] = x._Val[1] * y;
    return r;
}

_C_double_complex _Cmulcc(_C_double_complex x, _C_double_complex y)
{
    _C_double_complex r;
    double p, q, s;
    p = x._Val[0] * y._Val[0];
    q = x._Val[1] * y._Val[1];
    s = (x._Val[0] + x._Val[1])*(y._Val[0] + y._Val[1]);
    r._Val[0] = p - q;
    r._Val[1] = s - p - q;
    return r;
}

double creal(_C_double_complex x)
{
    return x._Val[0];
}

double cabs(_C_double_complex x)
{
    return sqrt(x._Val[0] * x._Val[0] + x._Val[1] * x._Val[1]);
}

#endif






/*************************************************************************
* @brief   :  Butterworth filter prototype, matlab函数
* @inparam :  n		阶数
*
* @outparam:  p		复矩阵
*			  k
* @author  :  wang zhe qi
* @date    :  2017/10/18 15:40
* @version :  ver 1.0
*************************************************************************/
static void mybuttap(int n, _C_double_complex* p, double *k)
{
    int i = 0, j = 0, m = 0;
    int size = (n - 1) / 2;
    int isodd = n % 2;

    _C_double_complex temp;

    for (i = 1, j = 0; i < n; j++)
    {
        temp._Val[0] = 0;
        temp._Val[1] = pi * i / (2 * n) + pi / 2;
        p[j * 2] = cexp(temp);
        i += 2;
    }

    for (m = 1, i = 0; i < j; i++)
    {
        p[m] = conj(p[m - 1]);
        m += 2;
    }

    if (isodd)
    {
        p[size * 2]._Val[0] = -1.0;
        p[size * 2]._Val[1] = 0.0;
    }

    _C_double_complex a = _Cmulcc(_Cmulcr(p[0], -1), _Cmulcr(p[1], -1));
    for (m = 2; m < size * 2 + isodd; m++)
    {
        a = _Cmulcc(a, _Cmulcr(p[m], -1));
    }

    *k = creal(a);
}

/*************************************************************************
* @brief   :  Characteristic polynomial or polynomial with specified roots, matlab函数
* @inparam :  p		复矩阵
*			  np	复矩阵的大小
* @outparam:  d		返回复矩阵
* @author  :  wang zhe qi
* @date    :  2017/10/18 15:40
* @version :  ver 1.0
*************************************************************************/
static void mypoly(_C_double_complex* p, int np, _C_double_complex *d)
{
    int i = 0, j = 0;
    _C_double_complex *c = (_C_double_complex *)malloc((np + 1) * sizeof(_C_double_complex));
    c[0]._Val[0] = 1.0;
    c[0]._Val[1] = 0.0;
    d[0]._Val[0] = 1.0;
    d[0]._Val[1] = 0.0;
    for (i = 1; i<np + 1; i++)
    {
        c[i]._Val[0] = 0.0;
        c[i]._Val[1] = 0.0;
        d[i]._Val[0] = 0.0;
        d[i]._Val[1] = 0.0;
    }

    _C_double_complex temp;
    for (i = 0; i < np; i++)
    {
        for (j = 1; j <= i + 1; j++)
        {
            temp = _Cmulcc(p[i], d[j - 1]);
            c[j]._Val[0] = d[j]._Val[0] - temp._Val[0];
            c[j]._Val[1] = d[j]._Val[1] - temp._Val[1];
        }
        for (j = 1; j <= i + 1; j++)
        {
            d[j]._Val[0] = c[j]._Val[0];
            d[j]._Val[1] = c[j]._Val[1];
        }
    }
    if (c != 0)	free(c);
}

/*************************************************************************
* @brief   :  实数矩阵相乘
* @inparam :  a		矩阵A
*			  b		矩阵B
*			  m		矩阵A与乘积矩阵C的行数
*			  n		矩阵A的行数,矩阵B的列数
*			  k		矩阵B与乘积矩阵C的列数
* @outparam:  c		乘积矩阵 C=AB
* @author  :  wang zhe qi
* @date    :  2017/10/18 15:40
* @version :  ver 1.0
*************************************************************************/
static void mytrmul(double a[], double b[], int m, int n, int k, double c[])
{
    int i, j, l, u;
    for (i = 0; i <= m - 1; i++)
        for (j = 0; j <= k - 1; j++)
        {
            u = i*k + j; c[u] = 0.0;
            for (l = 0; l <= n - 1; l++)
                c[u] = c[u] + a[i*n + l] * b[l*k + j];
        }
}

/*************************************************************************
* @brief   :  矩阵求逆
* @inparam :  a		矩阵A
*			  n		矩阵A的阶数
* @outparam:  a		逆矩阵
* @author  :  wang zhe qi
* @date    :  2017/10/18 15:40
* @version :  ver 1.0
*************************************************************************/
static void myrinv(double a[], int n)
{
    int *is, *js, i, j, k, l, u, v;
    double d, p;
    is = (int *)malloc(n * sizeof(int));
    js = (int *)malloc(n * sizeof(int));

    for (k = 0; k <= n - 1; k++)
    {
        d = 0.0;
        for (i = k; i <= n - 1; i++)
            for (j = k; j <= n - 1; j++)
            {
                l = i*n + j; p = fabs(a[l]);
                if (p>d) { d = p; is[k] = i; js[k] = j; }
            }
        if (d + 1.0 == 1.0)
        {
            if (is != 0)	free(is);
            if (js != 0)	free(js);
            return;
        }
        if (is[k] != k)
            for (j = 0; j <= n - 1; j++)
            {
                u = k*n + j; v = is[k] * n + j;
                p = a[u]; a[u] = a[v]; a[v] = p;
            }
        if (js[k] != k)
            for (i = 0; i <= n - 1; i++)
            {
                u = i*n + k; v = i*n + js[k];
                p = a[u]; a[u] = a[v]; a[v] = p;
            }
        l = k*n + k;
        a[l] = 1.0 / a[l];
        for (j = 0; j <= n - 1; j++)
            if (j != k)
            {
                u = k*n + j; a[u] = a[u] * a[l];
            }
        for (i = 0; i <= n - 1; i++)
            if (i != k)
                for (j = 0; j <= n - 1; j++)
                    if (j != k)
                    {
                        u = i*n + j;
                        a[u] = a[u] - a[i*n + k] * a[k*n + j];
                    }
        for (i = 0; i <= n - 1; i++)
            if (i != k)
            {
                u = i*n + k; a[u] = -a[u] * a[l];
            }
    }
    for (k = n - 1; k >= 0; k--)
    {
        if (js[k] != k)
            for (j = 0; j <= n - 1; j++)
            {
                u = k*n + j; v = js[k] * n + j;
                p = a[u]; a[u] = a[v]; a[v] = p;
            }
        if (is[k] != k)
            for (i = 0; i <= n - 1; i++)
            {
                u = i*n + k; v = i*n + is[k];
                p = a[u]; a[u] = a[v]; a[v] = p;
            }
    }
    if (is != 0)	free(is);
    if (js != 0)	free(js);
}




/*************************************************************************
* @brief   :  对复数排序
* @inparam :  p			复矩阵
*			  n			复矩阵大小
* @outparam:  p
* @author  :  wang zhe qi
* @date    :  2017/10/18 15:40
* @version :  ver 1.0
*************************************************************************/
static void sort_complex(_C_double_complex *p, int n)
{
    _C_double_complex *pa, *pb, *k, temp;
    for (pa = p; pa<p + n - 1; pa++)
    {
        k = pa;
        for (pb = pa + 1; pb < p + n; pb++)
            if (creal(*k) > creal(*pb))
                k = pb;
        temp = *pa;
        *pa = *k;
        *k = temp;
    }
}

/*************************************************************************
* @brief   :  Convert zero-pole-gain filter parameters to state-space form,matlab函数
* @inparam :  np			复矩阵p的阶数
*			  p,k0
*			  a,b,c,d
* @outparam:  a,b,c,d
* @author  :  wang zhe qi
* @date    :  2017/10/18 15:40
* @version :  ver 1.0
*************************************************************************/
static void  myzp2ss(_C_double_complex* p, int np, double k0, double *a, double *b, double *c, double *d)
{
    int i, j, k, jj, kk;
    int np1 = np;
    for (i = 0; i<np*np; i++)
    {
        a[i] = 0.0;
    }
    for (i = 0; i<np; i++)
    {
        b[i] = 0.0;
        c[i] = 0.0;
    }
    *d = 1;

    //If odd number of poles only, convert the pole at the
    //end into state-space.
    //H(s) = 1/(s-p1) = 1/(s + den(2))
    if (np % 2)
    {
        a[0] = -1;
        b[0] = 1;
        c[0] = 1;
        *d = 0;
        np1 = np - 1;
    }
    sort_complex(p, np1);

    //Take care of any left over unmatched pole pairs.
    //H(s) = 1/(s^2+den(2)s+den(3))
    _C_double_complex p_temp[2];
    _C_double_complex c_temp[3];
    double den[3], wn;
    double t[2 * 2] = { 0.0, 0.0, 0.0, 0.0 };
    double t_rinv[2 * 2] = { 0.0, 0.0, 0.0, 0.0 };
    double tr_den[2 * 2] = { 0.0, 0.0, 0.0, 0.0 };

    double a1_temp[2 * 2] = { 0.0, 0.0, 0.0, 0.0 };
    double a1[2 * 2] = { 0.0, 0.0, 0.0, 0.0 };
    double b1[2 * 2] = { 0.0, 0.0, 0.0, 0.0 };
    double c1[2 * 2] = { 0.0, 0.0, 0.0, 0.0 };
    double d1 = 0;
    int c_a = np % 2;	//a的列数
    int ma1 = np % 2;	//a的行数

    for (i = 0; i < np1; i = i + 2)
    {
        p_temp[0] = p[i];
        p_temp[1] = p[i + 1];
        mypoly(p_temp, 2, c_temp);
        for (j = 0; j<3; j++)
        {
            den[j] = creal(c_temp[j]);
        }
        wn = sqrt(cabs(p_temp[0]) * cabs(p_temp[1]));
        //		wn = 1;
        if (wn == 0)
            wn = 1;
        //a1 = t\[-den(2) -den(3); 1 0]*t;
        t[0] = 1.0;				//t[0][0]
        t[1 * 2 + 1] = 1.0 / wn;	//t[1][1]
        t_rinv[0] = 1.0;
        t_rinv[1 * 2 + 1] = 1.0 / wn;
        myrinv(t_rinv, 2);
        tr_den[0] = -den[1];
        tr_den[0 * 2 + 1] = -den[2];
        tr_den[1 * 2 + 0] = 1.0;
        tr_den[1 * 2 + 1] = 0.0;
        mytrmul(t_rinv, tr_den, 2, 2, 2, a1_temp);
        mytrmul(a1_temp, t, 2, 2, 2, a1);
        //b1 = t\[1; 0];
        double tr_temp1[2 * 1] = { 1.0, 0.0 };
        mytrmul(t_rinv, tr_temp1, 2, 2, 1, b1);
        double tr_temp2[2] = { 0.0, 1.0 };
        mytrmul(tr_temp2, t_rinv, 1, 2, 2, c1);
        d1 = 0;

        //[a,b,c,d] = series(a,b,c,d,a1,b1,c1,d1);
        //Next lines perform series connection
        if (ma1 != 0)
        {
            //a = [a zeros(ma1,na2); b1*c a1];
            for (k = 0; k<ma1; k++)
            {
                for (j = c_a; j<c_a + 2; j++)
                {
                    a[k*np + j] = 0;
                }
            }
            a[ma1*np + (c_a - 1)] = 1;
            for (k = ma1, kk = 0; kk < 2; k++, kk++)
            {
                for (j = c_a, jj = 0; jj < 2; j++, jj++)
                {
                    a[k*np + j] = a1[kk * 2 + jj];
                }
            }
            //b = [b; b1*d];
            //c = [d1*c c1];
            for (k = 0; k<c_a + 2; k++)
            {
                c[k] = 0;
            }
            c[c_a + 1] = 1;
            (*d) = d1*(*d);
            ma1 += 2;
            c_a += 2;
        }
        if (ma1 == 0)
        {
            //a = [a zeros(ma1,na2); b1*c a1];
            for (k = 0; k<2; k++)
            {
                for (j = 0; j<2; j++)
                {
                    a[k*np + j] = a1[k * 2 + j];
                }
            }
            //b = [b; b1*d];
            for (k = 0; k<2; k++)
            {
                b[k] = b1[k];
            }
            //c = [d1*c c1];
            for (k = 0; k<2; k++)
            {
                c[k] = c1[k];
            }
            (*d) = d1*(*d);
            ma1 += 2;
            c_a += 2;
        }
    }

    for (i = 0; i<np; i++)
    {
        c[i] *= k0;
    }
    (*d) = k0*(*d);
}

/*************************************************************************
* @brief   :  Change cutoff frequency for lowpass analog filter,matlab函数
* @inparam :  n			矩阵A的阶数
*			  a,b
*			  wo
* @outparam:  a,b
* @author  :  wang zhe qi
* @date    :  2017/10/18 15:40
* @version :  ver 1.0
*************************************************************************/
static void mylp2lp(int n, double *a, double *b, double wo)
{
    int i;
    for (i = 0; i < n*n; i++)
    {
        a[i] = wo * a[i];
    }
    for (i = 0; i < n; i++)
    {
        b[i] = wo * b[i];
    }
}

/*************************************************************************
* @brief   :  Transform lowpass analog filters to bandpass,matlab函数
* @inparam :  n			矩阵A的阶数
*			  a,b,c,d
*			  wo
*			  bw
* @outparam:  a,b,c,d
* @author  :  wang zhe qi
* @date    :  2017/10/18 15:40
* @version :  ver 1.0
*************************************************************************/
static void mylp2bp(int n, double **a, double **b, double **c, double *d, double wo, double bw)
{
    int i, j;
    double q = wo / bw;
    double *at = (double *)malloc(sizeof(double)*(2 * n)*(2 * n));
    double *bt = (double *)malloc(sizeof(double)*(2 * n));
    double *ct = (double *)malloc(sizeof(double)*(2 * n));

    for (i = 0; i < 2 * n; i++)
    {
        for (j = 0; j < 2 * n; j++)
        {
            if (i < n && j < n)
                at[i * 2 * n + j] = (*a)[+i*n + j] / q * wo;
            else if (i < n && j >= n)
            {
                if (i == j - n)
                    at[i * 2 * n + j] = 1 * wo;
                else
                    at[i * 2 * n + j] = 0;
            }
            else if (i >= n && j < n)
            {
                if (i - n == j)
                    at[i * 2 * n + j] = -1 * wo;
                else
                    at[i * 2 * n + j] = 0;
            }
            else if (i >= n && j >= n)
                at[i * 2 * n + j] = 0;
        }
    }
    bt[0] = (*b)[0] * wo;
    for (i = 1; i < 2 * n; i++)
    {
        bt[i] = 0;
    }
    for (i = 0; i < 2 * n; i++)
    {
        if (i < n)
            ct[i] = (*c)[i];
        else
            ct[i] = 0;
    }

    *a = (double*)realloc(*a, (2 * n)*(2 * n) * sizeof(double));
    *b = (double*)realloc(*b, (2 * n) * sizeof(double));
    *c = (double*)realloc(*c, (2 * n) * sizeof(double));
    for (i = 0; i < 2 * n * 2 * n; i++)
        (*a)[i] = at[i];
    for (i = 0; i < 2 * n; i++)
    {
        (*b)[i] = bt[i];
        (*c)[i] = ct[i];
    }

    if (at != 0)	free(at);
    if (bt != 0)	free(bt);
    if (ct != 0)	free(ct);
}

/*************************************************************************
* @brief   :  用于模数转换的双线性变换方法,matlab函数
* @inparam :  n			矩阵A的阶数
*			  a,b,c,d
*			  fs
* @outparam:  a,b,c,d
* @author  :  wang zhe qi
* @date    :  2017/10/18 15:40
* @version :  ver 1.0
*************************************************************************/
static void mybilinear(int n, double *a, double *b, double *c, double *d, double fs)
{
    int i, j;
    double t = 1 / fs;
    double r = sqrt(t);
    double *eye_a = (double *)malloc(n*n * sizeof(double));
    double *t1 = (double *)malloc(n*n * sizeof(double));
    double *t2 = (double *)malloc(n*n * sizeof(double));
    double *t2_rinv = (double *)malloc(n*n * sizeof(double));
    double *ad = (double *)malloc(n*n * sizeof(double));
    double *bd = (double *)malloc(n*n * sizeof(double));
    double *cd = (double *)malloc(n*n * sizeof(double));
    double *dd = (double *)malloc(n*n * sizeof(double));
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            if (i == j)
                eye_a[i*n + j] = 1;
            else
                eye_a[i*n + j] = 0;
            t1[i*n + j] = eye_a[i*n + j] + a[i*n + j] * t / 2;
            t2[i*n + j] = eye_a[i*n + j] - a[i*n + j] * t / 2;
            t2_rinv[i*n + j] = eye_a[i*n + j] - a[i*n + j] * t / 2;
        }
    }
    myrinv(t2_rinv, n);
    mytrmul(t2_rinv, t1, n, n, n, ad);
    mytrmul(t2_rinv, b, n, n, 1, bd);
    mytrmul(c, t2_rinv, 1, n, n, cd);
    mytrmul(cd, b, 1, n, 1, dd);
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            bd[i*n + j] = bd[i*n + j] * t / r;
            cd[i*n + j] = cd[i*n + j] * r;
            dd[i*n + j] = dd[i*n + j] * t / 2 + *d;

            a[i*n + j] = ad[i*n + j];
        }
    }
    for (i = 0; i < n; i++)
    {
        b[i] = bd[i*n];
        c[i] = cd[i];
    }
    *d = dd[0];

    if (eye_a != 0)	free(eye_a);
    if (t1 != 0)	free(t1);
    if (t2 != 0)	free(t2);
    if (t2_rinv != 0)	free(t2_rinv);
    if (ad != 0)	free(ad);
    if (bd != 0)	free(bd);
    if (cd != 0)	free(cd);
    if (dd != 0)	free(dd);

}

/*************************************************************************
* @brief   :  一般实矩阵约化为Hessenberg矩阵
* @inparam :  a		存放一般实矩阵A,返回上H矩阵
*			  n		矩阵的阶数
* @outparam:  a
* @author  :  wang zhe qi
* @date    :  2017/10/18 15:40
* @version :  ver 1.0
*************************************************************************/
static void myhhbg(double a[], int n)
{
    int i, j, k, u, v;
    double d, t;
    for (k = 1; k <= n - 2; k++)
    {
        d = 0.0;
        for (j = k; j <= n - 1; j++)
        {
            u = j*n + k - 1; t = a[u];
            if (fabs(t)>fabs(d))
            {
                d = t; i = j;
            }
        }
        if (fabs(d) + 1.0 != 1.0)
        {
            if (i != k)
            {
                for (j = k - 1; j <= n - 1; j++)
                {
                    u = i*n + j; v = k*n + j;
                    t = a[u]; a[u] = a[v]; a[v] = t;
                }
                for (j = 0; j <= n - 1; j++)
                {
                    u = j*n + i; v = j*n + k;
                    t = a[u]; a[u] = a[v]; a[v] = t;
                }
            }
            for (i = k + 1; i <= n - 1; i++)
            {
                u = i*n + k - 1; t = a[u] / d; a[u] = 0.0;
                for (j = k; j <= n - 1; j++)
                {
                    v = i*n + j;
                    a[v] = a[v] - t*a[k*n + j];
                }
                for (j = 0; j <= n - 1; j++)
                {
                    v = j*n + k;
                    a[v] = a[v] + t*a[j*n + i];
                }
            }
        }
    }
    return;
}

/*************************************************************************
* @brief   :  用带原点位移的双重步QR方法计算实上H矩阵的全部特征值
* @inparam :  a		存放上H矩阵A
*			  n		上H矩阵A的阶数
*			  eps	控制精度要求
*			  jt	控制最大迭代次数
* @outparam:  u		返回n个特征值的实部
*			  v		返回n个特征值的虚部
* @author  :  wang zhe qi
* @date    :  2017/10/18 15:40
* @version :  ver 1.0
*************************************************************************/
static int myhhqr(double a[], int n, double eps, int jt, double *u, double *v)
{
    int m, it, i, j, k, l, ii, jj, kk, ll;
    double b, c, w, g, xy, p, q, r, x, s, e, f, z, y;
    it = 0; m = n;
    while (m != 0)
    {
        l = m - 1;
        while ((l>0) && (fabs(a[l*n + l - 1])>eps*(fabs(a[(l - 1)*n + l - 1]) + fabs(a[l*n + l])))) l = l - 1;
        ii = (m - 1)*n + m - 1; jj = (m - 1)*n + m - 2;
        kk = (m - 2)*n + m - 1; ll = (m - 2)*n + m - 2;
        if (l == m - 1)
        {
            u[m - 1] = a[(m - 1)*n + m - 1]; v[m - 1] = 0.0;
            m = m - 1; it = 0;
        }
        else if (l == m - 2)
        {
            b = -(a[ii] + a[ll]);
            c = a[ii] * a[ll] - a[jj] * a[kk];
            w = b*b - 4.0*c;
            y = sqrt(fabs(w));
            if (w>0.0)
            {
                xy = 1.0;
                if (b<0.0) xy = -1.0;
                u[m - 1] = (-b - xy*y) / 2.0;
                u[m - 2] = c / u[m - 1];
                v[m - 1] = 0.0; v[m - 2] = 0.0;
            }
            else
            {
                u[m - 1] = -b / 2.0; u[m - 2] = u[m - 1];
                v[m - 1] = y / 2.0; v[m - 2] = -v[m - 1];
            }
            m = m - 2; it = 0;
        }
        else
        {
            if (it >= jt)
            {
                return(-1);
            }
            it = it + 1;
            for (j = l + 2; j <= m - 1; j++)
                a[j*n + j - 2] = 0.0;
            for (j = l + 3; j <= m - 1; j++)
                a[j*n + j - 3] = 0.0;
            for (k = l; k <= m - 2; k++)
            {
                if (k != l)
                {
                    p = a[k*n + k - 1]; q = a[(k + 1)*n + k - 1];
                    r = 0.0;
                    if (k != m - 2) r = a[(k + 2)*n + k - 1];
                }
                else
                {
                    x = a[ii] + a[ll];
                    y = a[ll] * a[ii] - a[kk] * a[jj];
                    ii = l*n + l; jj = l*n + l + 1;
                    kk = (l + 1)*n + l; ll = (l + 1)*n + l + 1;
                    p = a[ii] * (a[ii] - x) + a[jj] * a[kk] + y;
                    q = a[kk] * (a[ii] + a[ll] - x);
                    r = a[kk] * a[(l + 2)*n + l + 1];
                }
                if ((fabs(p) + fabs(q) + fabs(r)) != 0.0)
                {
                    xy = 1.0;
                    if (p<0.0) xy = -1.0;
                    s = xy*sqrt(p*p + q*q + r*r);
                    if (k != l) a[k*n + k - 1] = -s;
                    e = -q / s; f = -r / s; x = -p / s;
                    y = -x - f*r / (p + s);
                    g = e*r / (p + s);
                    z = -x - e*q / (p + s);
                    for (j = k; j <= m - 1; j++)
                    {
                        ii = k*n + j; jj = (k + 1)*n + j;
                        p = x*a[ii] + e*a[jj];
                        q = e*a[ii] + y*a[jj];
                        r = f*a[ii] + g*a[jj];
                        if (k != m - 2)
                        {
                            kk = (k + 2)*n + j;
                            p = p + f*a[kk];
                            q = q + g*a[kk];
                            r = r + z*a[kk]; a[kk] = r;
                        }
                        a[jj] = q; a[ii] = p;
                    }
                    j = k + 3;
                    if (j >= m - 1) j = m - 1;
                    for (i = l; i <= j; i++)
                    {
                        ii = i*n + k; jj = i*n + k + 1;
                        p = x*a[ii] + e*a[jj];
                        q = e*a[ii] + y*a[jj];
                        r = f*a[ii] + g*a[jj];
                        if (k != m - 2)
                        {
                            kk = i*n + k + 2;
                            p = p + f*a[kk];
                            q = q + g*a[kk];
                            r = r + z*a[kk]; a[kk] = r;
                        }
                        a[jj] = q; a[ii] = p;
                    }
                }
            }
        }
    }
    return(1);
}

/*************************************************************************
* @brief   :  复数除法
* @inparam :  a,b	表示复数a+jb
*			  c,d	表示复数c+jd
* @outparam:  *e,*f	指向返回的复数商 e+jf = (a+jb) / (c+jd)
* @author  :  wang zhe qi
* @date    :  2017/10/18 15:40
* @version :  ver 1.0
*************************************************************************/
static void mycdiv(double a, double b, double c, double d, double *e, double *f)
{
    double p, q, s, w;
    p = a*c; q = -b*d; s = (a + b)*(c - d);
    w = c*c + d*d;
    if (w + 1.0 == 1.0)
    {
        *e = 1.0e+35*a / fabs(a);
        *f = 1.0e+35*b / fabs(b);
    }
    else
    {
        *e = (p - q) / w; *f = (s - p - q) / w;
    }
    return;
}


/*************************************************************************
* @brief   :  求巴特沃斯滤波器系数 matlab对应函数 butter,精度大约在小数点后10~16位
* @inparam :  n		滤波器阶数
*			  Wn[2]	Wn在[0.0, 1.0]之间,与1/2采样率对应
*			  type	1 = "low" 		低通滤波器
*					2 = "bandpass" 	带通滤波器
*					3 = "high" 		高通滤波器		注:没写
*					4 = "stop" 		带阻滤波器		注:没写
*			  analog	0 = digital
*						1 = analog
* @outparam:  ab	长度为 n+1
*			  bb	长度为 n+1 或 2n+1(带通)
* @author  :  wang zhe qi
* @date    : 2017/10/18 15:40
* @version :  ver 1.0
*************************************************************************/
void FilterFunc::mybutter(int n, double Wn[], int type, int analog, double *ab, double *bb)
{

    int i, j;
    double fs = 2;
    double u[2] = { 0.0, 0.0 };
    //step 1: get analog, pre-warped frequencies
    if (!analog)
    {
        if (type == 1 || type == 3)
        {
            fs = 2;
            u[0] = 2 * fs*tan(pi*Wn[0] / fs);
        }
        else
        {
            fs = 2;
            u[0] = 2 * fs*tan(pi*Wn[0] / fs);
            u[1] = 2 * fs*tan(pi*Wn[1] / fs);
        }
    }
    else if (type == 2 || type == 4)
    {
        if (type == 1 || type == 3)
        {
            u[0] = Wn[0];
        }
        else
        {
            u[1] = Wn[1];
        }
    }

    //step 2: convert to low-pass prototype estimate
    double Bw = 0.0;
    if (type == 1 || type == 3)
    {
        Wn = u;
    }
    else if (type == 2 || type == 4)
    {
        Bw = u[1] - u[0];
        Wn[0] = sqrt(u[0] * u[1]);		//center
        Wn[1] = 0.0;
    }

    //step 3: Get N-th order Butterworth analog lowpass prototype
    _C_double_complex* p = (_C_double_complex*)malloc(n * sizeof(_C_double_complex));
    double k = 0;
    mybuttap(n, p, &k);

    //Transform to state-space
    int a_size = n;
    double *a = (double *)malloc(sizeof(double) * n * n);
    double *b = (double *)malloc(sizeof(double) * n);
    double *c = (double *)malloc(sizeof(double) * n);
    double d;

    myzp2ss(p, n, k, a, b, c, &d);

    if (type == 1)						// Lowpass
        mylp2lp(n, a, b, Wn[0]);
    else if (type == 2)					// Bandpass
    {
        mylp2bp(n, &a, &b, &c, &d, Wn[0], Bw);
        a_size = 2 * n;
    }
    else
    {
        if (p != 0)	free(p);
        if (a != 0)	free(a);
        if (b != 0)	free(b);
        if (c != 0)	free(c);
        return;
    }

    if (!analog)
    {
        mybilinear(a_size, a, b, c, &d, fs);
    }

    myhhbg(a, a_size);

    double *u_real = (double *)malloc(sizeof(double) *a_size);
    double *v_imag = (double *)malloc(sizeof(double) *a_size);
    double eps = 0.00000000000000000001L;
    int jt = 60;
    myhhqr(a, a_size, eps, jt, u_real, v_imag);

    _C_double_complex* p1 = (_C_double_complex*)malloc(a_size * sizeof(_C_double_complex));
    _C_double_complex* ctemp = (_C_double_complex*)malloc((a_size + 1) * sizeof(_C_double_complex));

    for (i = 0; i < a_size; i++)
    {
        p1[i]._Val[0] = u_real[i];
        p1[i]._Val[1] = v_imag[i];
    }

    mypoly(p1, a_size, ctemp);

    for (j = 0; j < a_size + 1; j++)
    {
        ab[j] = creal(ctemp[j]);
    }

    int r_lenth = 0;
    if (type == 1) r_lenth = n;
    else if (type == 2) r_lenth = n * 2;
    else if (type == 3) r_lenth = n;
    else if (type == 4) r_lenth = n;			//这里大小不清楚,待定
    _C_double_complex *r = (_C_double_complex *)malloc(sizeof(_C_double_complex) * r_lenth);
    double w = 0.0;
    Wn[0] = 2 * atan2(Wn[0], 4);
    switch (type)
    {
    case 1:
        for (i = 0; i < r_lenth; i++)
        {
            r[i]._Val[0] = -1;
            r[i]._Val[1] = 0;
        }
        w = 0;
        break;
    case 2:
        for (i = 0; i < r_lenth; i++)
        {
            if (i < n)
            {
                r[i]._Val[0] = 1;
                r[i]._Val[1] = 0;
            }
            else
            {
                r[i]._Val[0] = -1;
                r[i]._Val[1] = 0;
            }
        }
        w = Wn[0];
        break;
    case 3:
        for (i = 0; i < r_lenth; i++)
        {
            r[i]._Val[0] = 1;
            r[i]._Val[1] = 0;
        }
        w = pi;
        break;
    default:
        return;
    }

    _C_double_complex *r_temp = (_C_double_complex *)malloc(sizeof(_C_double_complex) * (r_lenth + 1));
    _C_double_complex *kern = (_C_double_complex *)malloc(sizeof(_C_double_complex) * (r_lenth + 1));
    mypoly(r, r_lenth, r_temp);

    for (j = 0; j < r_lenth + 1; j++)
    {
        bb[j] = creal(r_temp[j]);
    }

    _C_double_complex temp;
    for (i = 0; i < r_lenth + 1; i++)
    {
        temp._Val[0] = 0;
        temp._Val[1] = -1 * w * i;
        kern[i] = cexp(temp);
    }

    _C_double_complex c_temp1, c_temp2, c_temp3;
    c_temp3._Val[0] = 0;
    c_temp3._Val[1] = 0;
    for (i = 0; i < r_lenth + 1; i++)
    {
        c_temp1 = _Cmulcr(kern[i], ab[i]);
        c_temp3._Val[0] += c_temp1._Val[0];
        c_temp3._Val[1] += c_temp1._Val[1];
    }

    c_temp2._Val[0] = 0;
    c_temp2._Val[1] = 0;
    for (i = 0; i < r_lenth + 1; i++)
    {
        r_temp[i] = _Cmulcr(c_temp3, bb[i]);

        c_temp1 = _Cmulcr(kern[i], bb[i]);
        c_temp2._Val[0] += c_temp1._Val[0];
        c_temp2._Val[1] += c_temp1._Val[1];
    }
    for (i = 0; i < r_lenth + 1; i++)
    {
        mycdiv(r_temp[i]._Val[0], r_temp[i]._Val[1], c_temp2._Val[0], c_temp2._Val[1], &c_temp1._Val[0], &c_temp1._Val[1]);
        bb[i] = creal(c_temp1);
    }
    if (p != 0)	free(p);
    if (a != 0)	free(a);
    if (b != 0)	free(b);
    if (c != 0)	free(c);
    if (u_real != 0)	free(u_real);
    if (v_imag != 0)	free(v_imag);
    if (p1 != 0)	free(p1);
    if (ctemp != 0)	free(ctemp);
    if (r != 0)	free(r);
    if (r_temp != 0)	free(r_temp);
    if (kern != 0)	free(kern);
}


//求逆矩阵，当返回值为0时成功，a变为逆矩阵
int FilterFunc::rinv(double *a, int n)//逆矩阵
{
    int *is, *js, i, j, k, l, u, v;
    double d, p;
    is = (int *)malloc(n * sizeof(int));
    js = (int *)malloc(n * sizeof(int));
    for (k = 0; k <= n - 1; k++)
    {
        d = 0.0;
        for (i = k; i <= n - 1; i++)
            for (j = k; j <= n - 1; j++)
            {
                l = i*n + j; p = fabs(a[l]);
                if (p>d) { d = p; is[k] = i; js[k] = j; }
            }
        if (d + 1.0 == 1.0)
        {
            free(is); free(js);
            //printf("err**not invn");
            return(0);
        }
        if (is[k] != k)
            for (j = 0; j <= n - 1; j++)
            {
                u = k*n + j; v = is[k] * n + j;
                p = a[u]; a[u] = a[v]; a[v] = p;
            }
        if (js[k] != k)
            for (i = 0; i <= n - 1; i++)
            {
                u = i*n + k; v = i*n + js[k];
                p = a[u]; a[u] = a[v]; a[v] = p;
            }
        l = k*n + k;
        a[l] = 1.0 / a[l];
        for (j = 0; j <= n - 1; j++)
            if (j != k)
            {
                u = k*n + j; a[u] = a[u] * a[l];
            }
        for (i = 0; i <= n - 1; i++)
            if (i != k)
                for (j = 0; j <= n - 1; j++)
                    if (j != k)
                    {
                        u = i*n + j;
                        a[u] = a[u] - a[i*n + k] * a[k*n + j];
                    }
        for (i = 0; i <= n - 1; i++)
            if (i != k)
            {
                u = i*n + k; a[u] = -a[u] * a[l];
            }
    }
    for (k = n - 1; k >= 0; k--)
    {
        if (js[k] != k)
            for (j = 0; j <= n - 1; j++)
            {
                u = k*n + j; v = js[k] * n + j;
                p = a[u]; a[u] = a[v]; a[v] = p;
            }
        if (is[k] != k)
            for (i = 0; i <= n - 1; i++)
            {
                u = i*n + k; v = i*n + is[k];
                p = a[u]; a[u] = a[v]; a[v] = p;
            }
    }
    free(is);
    free(js);
    return(0);
}

//矩阵乘法
void FilterFunc::trmul(double *a, double *b, double *c, int m, int n, int k)//矩阵乘法  m为a的行数，n为a的列数数，k为b的行数，第一个矩阵列数必须要和第二个矩阵的行数相同
{
    int i, j, l, u;
    for (i = 0; i <= m - 1; i++)
        for (j = 0; j <= k - 1; j++)
        {
            u = i*k + j; c[u] = 0.0;
            for (l = 0; l <= n - 1; l++)
                c[u] = c[u] + a[i*n + l] * b[l*k + j];
        }
    return;
}

#define EPS 0.000001
void FilterFunc::filter(const double* x, double* y, int xlen, double* a, double* b, int nfilt, double* zi)//nfilt为系数数组长度
{
    double tmp;
    int i, j;


    //normalization
    if ((*a - 1.0>EPS) || (*a - 1.0<-EPS))
    {
        tmp = *a;
        for (i = 0; i<nfilt; i++)
        {
            b[i] /= tmp;
            a[i] /= tmp;
        }
    }


    memset(y, 0, xlen * sizeof(double));//将y清零，以双浮点为单位

//    qDebug() << "filter";
    a[0] = 0.0;
    for (i = 0; i<xlen; i++)
    {
        for (j = 0; i >= j&&j<nfilt; j++)
        {
            y[i] += (b[j] * x[i - j] - a[j] * y[i - j]);
//            qDebug() << "y[i]" << y[i] << "b[j]" << b[j] << "x[i - j]" << x[i - j] << "a[j]" << a[j] << "y[i - j]" << y[i - j];
        }
        if (zi&&i<nfilt - 1) y[i] += zi[i];
    }
    a[0] = 1.0;
}

//***********************************************************************************************
//*函数名 : filtfilt
//*函数功能描述 : filtfilt滤波算法
//*函数参数 : src —— 采集数据源     dest —— 源数据滤波  xlen —— 数据长度
//*          a —— 滤波因子a       b —— 滤波因子b      nfilt —— 英子长度
//*函数返回值 :
//*作者 :
//*函数创建日期 : 2017.8.15
//*函数修改日期 : 尚未修改
//*修改人 ：尚未修改
//*修改原因 :  尚未修改
//*版本 : 1.0
//*历史版本 : 无
//***********************************************************************************************/
int FilterFunc::filtfilt(double* x, double* y, int xlen, double* a, double* b, int nfilt)
{
    int nfact;
    int tlen;    //length of tx
    int i;
    double *tx, *tx1, *p, *t, *end;
    double *sp, *tvec, *zi;
    double tmp, tmp1;


    nfact = nfilt - 1;    //3*nfact: length of edge transients

    if (xlen <= 3 * nfact || nfilt<2) return -1;
    //too short input x or a,b
    //Extrapolate beginning and end of data sequence using a "reflection
    //method". Slopes of original and extrapolated sequences match at
    //the end points.
    //This reduces end effects.
    tlen = 6 * nfact + xlen;
    tx = (double *)malloc(tlen * sizeof(double));
    tx1 = (double *)malloc(tlen * sizeof(double));


    sp = (double *)malloc(sizeof(double) * nfact * nfact);
    tvec = (double *)malloc(sizeof(double) * nfact);
    zi = (double *)malloc(sizeof(double) * nfact);


    if (!tx || !tx1 || !sp || !tvec || !zi) {
        free(tx);
        free(tx1);
        free(sp);
        free(tvec);
        free(zi);
        return 1;
    }

    tmp = x[0];
    for (p = x + 3 * nfact, t = tx; p>x; --p, ++t)
        *t = 2.0*tmp - *p;
    for (end = x + xlen; p<end; ++p, ++t)
        *t = *p;
    tmp = x[xlen - 1];
    for (end = tx + tlen, p -= 2; t<end; --p, ++t)
        *t = 2.0*tmp - *p;
    //now tx is ok.


    end = sp + nfact*nfact;
    p = sp;
    while (p<end) *p++ = 0.0L; //clear sp
    sp[0] = 1.0 + a[1];
    for (i = 1; i<nfact; i++)
    {
        sp[i*nfact] = a[i + 1];
        sp[i*nfact + i] = 1.0L;
        sp[(i - 1)*nfact + i] = -1.0L;
    }


    for (i = 0; i<nfact; i++)
    {
        tvec[i] = b[i + 1] - a[i + 1] * b[0];
    }


    if (rinv(sp, nfact)) {
        free(zi);
        zi = NULL;
    }
    else {
        trmul(sp, tvec, zi, nfact, nfact, 1);
    }//zi is ok


    free(sp); free(tvec);


    //filtering tx, save it in tx1
    tmp1 = tx[0];
    if (zi)
        for (p = zi, end = zi + nfact; p<end;) *(p++) *= tmp1;
    filter(tx, tx1, tlen, a, b, nfilt, zi);


    //reverse tx1
    for (p = tx1, end = tx1 + tlen - 1; p<end; p++, end--) {
        tmp = *p;
        *p = *end;
        *end = tmp;
    }


    //filter again
    tmp1 = (*tx1) / tmp1;
    if (zi)
        for (p = zi, end = zi + nfact; p<end;) *(p++) *= tmp1;
    filter(tx1, tx, tlen, a, b, nfilt, zi);

    //reverse to y
    end = y + xlen;
    p = tx + 3 * nfact + xlen - 1;
    while (y<end) {
        *y++ = *p--;
    }


    free(zi);
    free(tx);
    free(tx1);


    return 0;
}


QList<double> FilterFunc::my_filter(QList<double> array, int is_band_pass, double lower, double upper)
{
    // 数据滤波
    const int nj = 3;
    int n = 0;
    double *ab;
    double *bb;

    if(is_band_pass)
    {
        double wn[2] = { lower / (1000.0L / 2), upper / (1000.0L / 2.0) };
        ab = new double[2 * nj + 1];
        bb = new double[2 * nj + 1];
        n = 2 * nj + 1;
//        qDebug() << "wn[0]=" << wn[0] << "wn[1]=" << wn[1];
        mybutter(nj, wn, 2, 0, ab, bb);

    }
    else
    {
        double wn[2] = { upper / (1000.0L / 2), 0.0 };
        ab = new double[nj + 1];
        bb = new double[nj + 1];
        n = nj + 1;
        mybutter(nj, wn, 1, 0, ab, bb);
    }


    for(int i=0;i<n;i++)
    {
//        qDebug() << "i:" << i << "ab " << ab[i] << "bb" << bb[i];
    }

    double *i_filter_data = new double[array.size()];
    double *o_filter_data = new double[array.size()];
    for(int i=0;i<array.size();i++)
    {
        i_filter_data[i] = array[i];
    }
    filtfilt(i_filter_data, o_filter_data, array.size(), ab, bb, n);
//    qDebug() << "calc result:";
    QList<double> out_array;
    for(int i=0;i<array.size();i++)
    {
        out_array.append(o_filter_data[i]);
//        qDebug() << o_filter_data[i];
    }

    delete [] ab;
    delete [] bb;
    delete [] i_filter_data;
    delete [] o_filter_data;

    return out_array;
}


#include <QDebug>
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//测试用例
int FilterFunc::test_main(void)
{

#if 0
    //低通
    double wn[2] = { 8.15214L / (88.658L / 2), 0.0 };
    double ab[4];
    double bb[4];
    int n = 4;
    mybutter(3, wn, 1, 0, ab, bb);
#else
    //带通
    double wn[2] = { 8.15214L / (88.658L / 2), 26.25876L / (88.658L / 2) };
    double ab[2 * 4 + 1];
    double bb[2 * 4 + 1];
    int n = 2 * 4 + 1;
    mybutter(4, wn, 2, 0, ab, bb);
    for(int i=0;i<9;i++)
        qDebug() << "ab: " << ab[i];

    for(int i=0;i<9;i++)
        qDebug() << "bb: " << bb[i];
#endif;

//    system("pause");
    return 0;
}
