/* Dedispersion code for lofasm. 
Copy right : Center for Advanced Radio Astronomy
Author: Jing Luo, Nan Yan, Teviet, Fredrick A. Jenet */


#include<stdio.h>
#include<string.h>
#include<math.h>
#include<stdlib.h>
//#include"dedisperion.h"

typedef struct {
    int NumChnl;
    int NumTimeSamp;
    int poolClumN;
    int resFltClumN;
    int resDMTClumN;
    int poolRowN;
    int resFltRowN;
    int resDMTRowN;
    double DMLmt[2];
    int NumDM;
    double fs;
}Parameter;

void read_parameters(Parameter *par,char filename[1024]);
void check_line(Parameter *par, char *str, FILE *fp);
void read_data_simple(double** dataPool, FILE *fp, Parameter par);
/******************** Functions under design will be moved to .h file***********
void read_parameters(par,);   // FIXME
void initialize_parameters(par);
void read_data(par);     // FIXME
doulbe do_dedispersion(**input, **resFlf, **resDMT); //FIXME
****************************************************************/
int main()
{
    Parameter par;
    char parameterFileNm[1024];
    double **inputDataPool, **resultFltBank,**resultDM_Time;  // 2D arrays
    double *freqArray, *timeArray, *DMArray;
    double *DMarray; double DMstep;     //test varible
    double fsBsBn;
    fsBsBn = 1.0/pow(2,14)*4096.0;
    printf("reading parameter files\n");
/* Read parameter file*/
    sprintf(parameterFileNm, "dedispersionPar.dat");
    read_parameters(&par,parameterFileNm);
/* Initalize config and check parameters setting */
    if(par.poolClumN <= par.resFltClumN || par.poolRowN <= par.resFltRowN)
    {
        printf("data pool's column or row should bigger than result's column or\
                row.\n");
        exit(1);   // FIXME std error output
    }
    printf("%d",par.NumDM);
    DMstep = (par.DMLmt[1]-par.DMLmt[0])/par.NumDM;
    par.resDMTRowN = par.NumDM;
    par.resDMTClumN = par.resFltClumN;
/* Initating data pool and result storage*/    
    printf("Initialize the data pool and result storage\n");
    if(!((inputDataPool = malloc(par.poolRowN * sizeof(double)))&& \
         (resultFltBank = malloc(par.resFltRowN * sizeof(double)))&& \
         (resultDM_Time = malloc(par.resDMTRowN * sizeof(double)))))
        printf("Memory error\n");          // Need to be stander error output
    int i,j;
    for(i=0;i<par.poolRowN;i++)
    {
        if(!(inputDataPool[i]=malloc(par.poolClumN*sizeof(double))))
            printf("Memory error\n");      // Need to be stander error output
        for(j=0;j<par.poolClumN;j++)
        {
            inputDataPool[i][j] = 0.0; 
        }
    }
    /*
    for(i=0;i<poolClumN;i++)
    {
        for(j=0;j<poolRowN;j++)
        {
            printf("%lf ",inputDataPool[i][j]);
        }
        printf("\n");
    }
    */
    for(i=0;i<par.resFltRowN;i++)
    {            
        if(!(resultFltBank[i]=malloc(par.resFltClumN*sizeof(double))))
            printf("Memory error\n");      // Need to be stander error output
    }        
    for(i=0;i<par.resFltRowN;i++)
    {
        if(!(resultDM_Time[i]=malloc(par.resDMTClumN*sizeof(double))))
            printf("Memory error\n");      // Need to be stander error output 
    }
    printf("Initialization data pool and result storage completed\n");

/* Initalize time axis and frequency axis*/
    printf("Initialize time axis and frequency axis\n");

    if(!((freqArray = malloc(par.NumChnl*sizeof(double)))&& \
        (freqArray = malloc(par.NumChnl*sizeof(double)))))
        printf("Memory error\n");
        
    printf("Initialization time axis and frequency axis complete\n");


/* Read data*/
    FILE *dataFile;
    dataFile = fopen("/Users/jingluo/Research_codes/LoFASM/Program4/BaseBandDate.dat","r"); // FIXME not hard coded.
    printf("Reading data\n");
    read_data_simple(inputDataPool, dataFile, par);
    fclose(dataFile);// FXIME does it need to be in the reading file function?
    
    
    
    /* Write the result to a file
    FILE *output;
    output = fopen("dedisfltB.dat","w");
    for(i=0;i<par.poolRowN;i++)
    {
        for(j=0;j<par.poolClumN;j++)
        {
            fprintf(output,"%lf ",inputDataPool[i][j]);
        }
        fprintf(output, "\n");
    }
    fclose(output);*/
/* Do dedispersion and write to a file*/
    /* Need changed here, This is only a test. */
    FILE *freqFile;
    freqFile = fopen("/Users/jingluo/Research_codes/LoFASM/Program4/FreqAxis.dat","r");
    double shiftIndex;
    int k;
    for(j=0;j<4096;j++)
    {
        fscanf(freqFile,"%lf",&freqArray[j]);
        //      printf("%lf",freqAxis[j]);
    }
     
    fclose(freqFile);   
    /* Above reading the frequency array*/
    /* assign the DM trail value array*/
    DMarray = malloc(par.NumDM*sizeof(double));
    for(i=0;i<par.NumDM;i++)
    {
        DMarray[i] = par.DMLmt[0]+i*DMstep;    
    }
    /* Do dedisperion*/
    double DMvalue = 0.5e-9;
    double *timeDelay;
    
    if(!(timeDelay = malloc(par.NumChnl*sizeof(double))))
        printf("Memory error\n");      // Need to be stander error output
    double minTimeDelay;
    FILE *outputFlt;
    outputFlt = fopen("dedisfltB.dat","w");
    fclose(outputFlt);
    printf("%d\n",par.NumDM);
    for(k=0;k<par.NumDM;k++)
    {    
        printf("%d",k);
        for(i=0;i<par.NumChnl;i++)
        {
            timeDelay[i] = 4.15e3*(1.0/pow(freqArray[i]/1.0e6,2))*DMarray[k];
            if(i==0)
                minTimeDelay = timeDelay[i];  // Initial value for minmum time delay
            else
                if(timeDelay[i]<=minTimeDelay)
                    minTimeDelay = timeDelay[i];  // Find the minmum time delay 
        }
        printf("%lf\n", minTimeDelay);  
        for(i=0;i<par.NumChnl;i++)
        {
            shiftIndex = (timeDelay[i]-minTimeDelay)/fsBsBn;//Calculate shift index
            int shiftIndexI = (int) shiftIndex;
            printf("Frequency = %lf timedelay = %lf shiftIndex = %d\n",freqArray[i],\
            timeDelay[i],shiftIndexI);
            for(j=0;j<par.NumTimeSamp;j++)
            {    
                if(shiftIndexI>par.poolClumN || shiftIndexI<0)
                    resultFltBank[i][j] = 0;
                else
                    resultFltBank[i][j] = inputDataPool[i][j+shiftIndexI];     
                resultDM_Time[k][j] = resultDM_Time[k][j]+resultFltBank[i][j];
            }       
        }
   
    }
/* Need be careful about the Num of channel and num of time sample. Now it is
hard coded. But we need to know the relationship of all the parameters*/
    
    /*Write the result to a file*/
    FILE *outputDMT;
    outputDMT = fopen("dedisfltBDMT.dat","w");
    for(i=0;i<par.resDMTRowN;i++)
    {
        for(j=0;j<par.resDMTClumN;j++)
        {
            fprintf(outputDMT,"%lf ",resultDM_Time[i][j]);
        }
        fprintf(outputDMT, "\n");
    }
    fclose(outputDMT);
    return 0;

}

void read_parameters(Parameter *par,char filename[1024])
{
    char *temp;
    char str[1024];
    int nline;
    FILE *fp;
    /* Reading parameter files */
    fp = fopen(filename,"r");
    if(fp == NULL)
    {
        printf("File %c is not exist, please check your input", filename);
        exit(1);
    }
    /* Read lines */
    while (!feof(fp))
    {
        nline = fscanf(fp,"%s",str);
        if(nline==1)
        {    
            check_line(par,str,fp);
           // printf("%c %d\n",str[0],nline);
        }
    }
    fclose(fp);
}

void check_line(Parameter *par, char *str, FILE *fp)
{   
    if(str[0]=='#')   /* Read comment line */
        fgets(str,1000,fp);
    else if(strcasecmp(str,"POOL_CLNM") == 0)  // Data pool colunm number
        fscanf(fp,"%d",&par->poolClumN);
    else if(strcasecmp(str,"POOL_ROW") == 0)   // Data pool row number
        fscanf(fp,"%d",&par->poolRowN);      
    else if(strcasecmp(str,"RES_FLT_CLNM") == 0) // Result filter bank column # 
        fscanf(fp,"%d",&par->resFltClumN);
    else if(strcasecmp(str,"RES_FLT_ROW") == 0) // Result filter bank row #   
        fscanf(fp,"%d",&par->resFltRowN);
    else if(strcasecmp(str,"RES_DMT_CLNM") == 0) // Result DM_time column #
        fscanf(fp,"%d",&par->resDMTClumN);
    else if(strcasecmp(str,"RES_DMT_ROW") == 0) // Result DM_time row #
        fscanf(fp,"%d",&par->resDMTRowN);
    else if(strcasecmp(str,"NUM_CHNL") == 0) // number of frequency channel
        fscanf(fp,"%d",&par->NumChnl);
    else if(strcasecmp(str,"NUM_TIME_SAMP") == 0) // number of time sample 
        fscanf(fp,"%d",&par->NumTimeSamp);
    else if(strcasecmp(str,"DM_LMT") == 0) // DM lower limit and upper limit
        fscanf(fp,"%lf %lf",&par->DMLmt[0],&par->DMLmt[1]);
    else if(strcasecmp(str,"DM_NUM") == 0) // DM steps
        fscanf(fp,"%d",&par->NumDM);    
}



void read_data_simple(double** dataPool, FILE *fp, Parameter par)
{
    int i,j;

    for(i=0;i<par.resFltClumN;i++)// Need to be careful about the ClumN or row. 
    {
        for(j=0;j<par.resFltRowN;j++)
        {
            fscanf(fp,"%lf",&dataPool[j][i]);
            //printf("%lf ",dataPool[i][j]);
        }
    }
}   
