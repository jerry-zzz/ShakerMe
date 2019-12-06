#ifndef FILTERFUNC_H
#define FILTERFUNC_H

#include <QObject>

class FilterFunc
{
public:
    FilterFunc();


    void mybutter(int n, double Wn[], int type, int analog, double *ab, double *bb);


    int rinv(double *a, int n);
    void trmul(double *a, double *b, double *c, int m, int n, int k);
    void filter(const double* x, double* y, int xlen, double* a, double* b, int nfilt, double* zi);
    int filtfilt(double* x, double* y, int xlen, double* a, double* b, int nfilt);

    QList<double> my_filter(QList<double> array, int is_band_pass, double lower, double upper);

    int test_main(void);
};

#endif // FILTERFUNC_H
