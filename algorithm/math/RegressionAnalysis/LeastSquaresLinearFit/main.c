#include <stdio.h>

#include "../../../algutils.h"
#include "../../mathutils.h"
#include "LeastSquaresLinearFit.h"


int main()
{
    double height[] = { 1.47, 1.50, 1.52, 1.55, 1.57, 1.60, 1.63, 1.65, 1.68, 1.70, 1.73, 1.75, 1.78, 1.80, 1.83 };
    double mass[] = { 52.21, 53.12, 54.48, 55.84, 57.20, 58.57, 59.93, 61.29, 63.11, 64.47, 66.28, 68.10, 69.92, 72.19, 74.46 };
    double slope, intercept;

    double x[] = { -4.0, -3.0, -2.0, -1.5, -0.5, 1.0, 2.0, 3.5, 4.0 };
    double y[] = { -3.0, -1.0, -2.0, -0.5, 1.0, 0.0, 1.5, 1.0, 2.5 };

    double year[] = { 1980, 1982, 1984, 1986, 1988, 1990, 1992, 1994, 1996, 1998, 2000, 2002, 2004, 2006, 2008 };
    double CO2_level[] = { 338.7, 341.2, 344.4, 347.2, 351.5, 354.2, 356.3, 358.6, 362.4, 366.5, 369.4, 373.2, 377.5, 381.9, 385.6 };

    LeastSquaresLinearFit(height, ARRAY(mass), &slope, &intercept);
    printf("Slope: %f, Intercept: %f\n", slope, intercept);

    slope = covd(height, ARRAY(mass)) / vard(ARRAY(height));
    printf("Slope via Cov(X,Y)/Var(X): %f\n", slope);\

    LeastSquaresLinearFit(x, ARRAY(y), &slope, &intercept);
    printf("Slope: %f, Intercept: %f\n", slope, intercept);

    LeastSquaresLinearFit(year, ARRAY(CO2_level), &slope, &intercept);
    printf("Slope: %f, Intercept: %f\n", slope, intercept);

    return 0;
}
