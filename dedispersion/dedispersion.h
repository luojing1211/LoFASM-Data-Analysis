/* 
* Copyright (C), Jing Luo, Nan Yan, 
*
*
*
*/

#ifndef _DEDISPERSION_H
#define SEC_DAY 86400.0
#define DM_CONST 4.15e3

#endif

typedef struct {
    int NumChnl;       // Number of Channel
    int NumTimeSamp;   // Number of time sample
    int poolClumN;     // Number of data pool column 
    int resFltClumN;   // Number of result fliter bank column
    int resDMTClumN;   // Number of result DM vs time column
    int poolRowN;      // Number of data pool row
    int resFltRowN;    // Number of result fliter bank row
    int resDMTRowN;    // Number of result DM vs time row
    double fs;
}Parameter;


/* Read lines from parameter file*/            
void read_parameters(Parameter *par,char filename[1024]); 
/* Check parameter lines*/
void check_line(Parameter *par, char *str, FILE *fp);


//double read_data()

