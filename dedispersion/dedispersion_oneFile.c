//
//  File.c
//  
//
//  Created by Jing Luo on 9/26/13.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define KDM 2.41e-16  // dispersion constant


typedef struct {
    int algorithm;   // When algorithm=1 (nonzero) the program adopt summation algorithm. When algorithm=0 the program adopt select algorithm.
    int powerOutput; // When poweroutput=1 (nonzero) the program will output time series (sum up power of each column of filter bank).
    int RsmplVal;   //the resampling number.
    int lenArray;  //the column length the array.
    int clmnFltBank; //the number of columns of the filter bank.
    int numChannel;  //number of channels.    2048
    int dedisVal;  // The number of columns of de-dispersed filter bank.
    int DMTrailNum;  // The number of trail when we are searching the true DM. It depends on DM search range and DM search resolution.
    double Fs;  // sample frequency
    double DMmin;  // The minimal DM of DM search range.
    double DMmax;  // The Maximal DM of DM search range.
    double DMresolution;   //The DM search resolution.
    double timeResolution;  // The time resolution of input filter bank.
    double freqLow;             // The low frequency cut off.
    char inputFile[1024];     // store the base format for file name of input filter bank.
    char outputFile[1024];     //store the base format for file name of output filter bank.
    char powerFile[1024];      //store the base format for file name of output time series.
    int inputStartNum;    // the number of the first input file
    int inputEndNum;       //the number of the last input file
    int outputStartNum;     //the number of the first output file
    int numRsmplUsr;                //the resampling number specified by user.
    double memoryLmt;             // memory limit.
    int lowFreqIndex;              // convert the low frequency cutoff to an index in the channels. It depends on flow and FS.
    double SNRcutoff;          // the minimal SNR we accept.
    int EXITNUMBER;            // to contral the program action when there is no input filter bank.  Value "0" denotes the program will be terminated when the input file cannot be found. Value "1" denotes the program will wait for new data when the input file cannot be found. Value "2" denotes the program will treat the nonexistent data as 0.0 when the input file cannot be found.
}Parameter;


/* Functions initalize*/
int countdis(int i, double c,Parameter *par);
double transform(int i, int j, double **fbk, int *displacement,Parameter *par);
void dedispersion(double c, int *displacement, double **fbk, double **result,Parameter *par);
double calculate_criterion(double **result,Parameter *par);
int max_in(double *a, int n);
void readconfig(Parameter *par);
void checkline(Parameter *par,char *temp,char *str);
void readfbk(double **temp, FILE *fp);
int resamplefbk(double **fbk, int nfbk);
int continuefbk(double **fbk,FILE *fp);
void calpower(double **result, double *power,Parameter *par);
double calave(double *power, int n);
double calsigma(double *power, int n);
void swap4(char *word); // Big Enddien for 4 bytes
void swapInt(int *word);// Big Enddien for INT type
void readData(FILE *fp,double **fltBank,Parameter par); // Read data from file
int calFileSize(FILE *fp);



int main(int argc, char *argv[])
{
    int count1 = 0, count2 = 0;
    int numFltBank;
    int *displacement;
    double duration;
    double  **fltBank, **temp, **result, *power;
    
    int data; // for reading the binary file
    int fileSize;
    double DM_trail;
    double computeTime;
    double *crtr, *DM_array;
    double ave, sigma, SNR, powerMax,pulseTime;
    char fname[1024];
    clock_t start, finish;
    FILE *fp, *outputFile;
    
    Parameter par;
    
    
    /*Read config file*/
    readconfig(&par);
    
    
    fp = fopen(par.inputFile,"rb");
    if(!fp)
    {
        printf("No file %s is found",fname);
        exit(1);
    }
    
    fileSize = calFileSize(fp);
    
    par.lenArray = (fileSize-80)/(2048+8);
    par.lenArray = 20;
    par.RsmplVal=1;
    par.dedisVal = par.lenArray/par.RsmplVal+1;
    
    printf("dedisVal %d",par.dedisVal);
    if(par.DMresolution==0)
		par.DMresolution=par.timeResolution*KDM*(par.Fs/4)*(par.Fs/4);
    
    par.DMTrailNum = (par.DMmax-par.DMmin)/par.DMresolution;
    
    
    


    printf("%d\n",par.lenArray);
    if(!(
         (fltBank = malloc( par.lenArray* sizeof(double)))&&
         (result = malloc( par.dedisVal * sizeof(double)))&&
         (temp = malloc( par.dedisVal * sizeof(double)))&&
         (crtr = malloc( par.DMTrailNum * sizeof(double)))&&
         (DM_array = malloc( par.DMTrailNum * sizeof(double)))&&
         (displacement = malloc( par.numChannel * sizeof(int)))
         )
       )
        printf("Memory Error\n");
    
    int i,j,m;  // loop control
    
    for(i=0;i<par.lenArray;i++)
        if(!(fltBank[i] = malloc( par.numChannel * sizeof(double))))
			fprintf(stdout,"memory error \n");
    for(i=0;i<par.dedisVal;i++)
        if( !(result[i] = malloc(par.numChannel* sizeof(double)))			)
            fprintf(stdout,"memory error \n");
    if( !(power = malloc( par.dedisVal * sizeof(double)))			)
        fprintf(stdout,"memory error \n");
    

    
    /*Read Data*/
    readData(fp,fltBank,par);    
    for(m=0; m<=par.DMTrailNum; m++)
    {
        DM_trail=par.DMmin+m*par.DMresolution;
        printf(" m =%d\n",m);
        dedispersion(DM_trail,displacement,fltBank,result,&par);
        DM_array[m]=DM_trail;  // Don't know why this is need.
        crtr[m]=calculate_criterion(result,&par);
        printf("criterion %lf\n",crtr[m]);
    }
    
    
    return 0;
}

void readconfig(Parameter *par)
{
    char *temp;
    FILE *fp;
    char *str;
    int nRead;
    
    if(!(temp = malloc(1024)))
        fprintf(stdout,"memory error \n");
    
    if(!(fp = fopen("dedisperser_config.txt", "r")))
    {
        fprintf(stdout," File dedisperser_config.txt could not be opened!");
        exit(EXIT_FAILURE);
    }
    
    fscanf(fp, "%s %d", temp, &par->clmnFltBank);
    //printf("%s , %d \n",temp, par->numClmn);
    fscanf(fp, "%s %d", temp, &par->numChannel);
    //printf("%s , %d \n ",temp, par->numChannel);
    fscanf(fp, "%s %lf", temp, &par->Fs);
    //printf("%s , %lf \n",temp, par->Fs);
    fscanf(fp, "%s %lf", temp, &par->timeResolution);
    //printf("%s , %lf \n",temp, par->timeResolution);
    fscanf(fp, "%s %lf", temp, &par->DMmin);
    //printf("%s , %lf \n",temp,par->DMmin);
    fscanf(fp, "%s %lf", temp, &par->DMmax);
    //printf("%s , %lf \n",temp,par->DMmax);
    fscanf(fp, "%s %lf", temp, &par->DMresolution);
    //printf("%s , %lf \n",temp,par->DMresolution);
    fscanf(fp, "%s %lf", temp, &par->freqLow);
    //printf("%s , %lf \n",temp,par->freqLow);
    fscanf(fp, "%s %s", temp, &par->inputFile);
    //printf("%s , %s \n",temp,par->inputFile);
    fscanf(fp, "%s %d", temp, &par->inputStartNum);
    //printf("%s , %d \n",temp,par->inputStartNum);
    fscanf(fp, "%s %d", temp, &par->inputEndNum);
    //printf("%s , %d \n",temp,par->inputEndNum);
    fscanf(fp, "%s %s", temp, &par->outputFile);
    //printf("%s , %s \n",temp,par->outputFile);
    fscanf(fp, "%s %d", temp, &par->outputStartNum);
    //printf("%s , %d \n",temp,par->outputStartNum);
    fscanf(fp, "%s %d", temp, &par->numRsmplUsr);
    //printf("%s , %d \n",temp,par->numRsmplUsr);
    fscanf(fp, "%s %lf", temp, &par->memoryLmt);
    //printf("%s , %lf \n",temp,par->memoryLmt);
    fscanf(fp, "%s %s", temp, &par->powerFile);
    //printf("%s , %s \n",temp,par->powerFile);
    fscanf(fp, "%s %lf", temp, &par->SNRcutoff);
    //printf("%s , %lf \n",temp,par->SNRcutoff);
    fscanf(fp, "%s %d", temp, &par->EXITNUMBER);
    //printf("%s , %d \n",temp,par->EXITNUMBER);
    
    /*
     while (!feof(fp))
     {
     fscanf(fp,"%s %s",temp,str);
     printf("%s %s\n",temp,str);
     checkline(par,temp,str);
     //     i++;
     }
     */
    
    fclose(fp);
    free(temp);
}
int calFileSize(FILE *fp)
{
    int sz;
    fseek(fp,0,SEEK_END);
    sz = ftell(fp);
    fseek(fp,0,SEEK_SET);
    return sz;
}

void readData(FILE *fp,double **fltBank,Parameter par)
{
    char buf[1024];
    int i,j;     // loop control
    int data;
    int count = 0;
    fread(buf,1,88,fp);   //Read headerfile
    

    for(i=0;i<par.lenArray;i++)
    {
        for(j=0;j<par.numChannel;j++)
        {

            fread(&data,4,1,fp);
            swapInt(&data);
            fltBank[i][j]=(double)data;
            count++;
        }
        fseek(fp,8,SEEK_CUR);
    }
    fclose(fp);
}
    
    
    
    
    
void swap4(char *word)
{
    char tmp;
    tmp = word[0]; word[0] = word[3]; word[3] = tmp;
    tmp = word[1]; word[1] = word[2]; word[2] = tmp;
}


void swapInt(int *word)
{
    swap4((char *)word);
}

void dedispersion(double c, int *displacement, double **fbk, double **result,Parameter *par)
{
    int i, j;
    
    /*find the displacement for each channel*/
    printf("DMval = %lf",c);
    for(i=0;i<par->numChannel;i++)
    {
        displacement[i]=countdis(i, c,par);
    }
    for(i=0;i<par->numChannel;i++)
        if(displacement[i]>par->lenArray||displacement[i]<0)
            displacement[i]=par->lenArray;
    
    
    /*de-dispersion*/
    for(i=0;i<par->lowFreqIndex;i++)
        for(j=0;j<par->dedisVal;j++)
            result[j][i]=0.0;
    
    for(i=par->lowFreqIndex;i<par->numChannel;i++)
    {
        for(j=0;j<par->dedisVal;j++)
            result[j][i]=transform(i, j, fbk, displacement,par);
        
    }
}


int countdis(int i, double c, Parameter *par)
{
    int displacement, difference;  //we convert time delay to an index of column (displacement) in filter bank. difference is used for time correction.
    double time, frequency;  // time is the time delay. frequency is corresponding to i.
    
    difference=par->clmnFltBank%par->RsmplVal;
    
    if(i>1)
        frequency=(i+0.5)*par->Fs/(par-> numChannel*2);
    else
        frequency=1.0;
    
    if(frequency-0.0<0.5&&frequency-0.0>-0.5)
        frequency=0.5;
    
    
    time=c/((frequency*frequency)*KDM);
    printf("time %lf\n",time);
    if(difference==0)
        displacement=time/par->timeResolution;
    else
        displacement=time/par->timeResolution+time/par->timeResolution/par->dedisVal*(par->RsmplVal-difference)/par->RsmplVal;
    return displacement;
}


double transform(int i, int j, double **fbk, int *displacement, Parameter *par)
{
    int m=0, differ=0, k=0;  //differ is the difference between two adjacent channels.
    double result=0.0;
    
    if((i!=0)&&(i<par->numChannel))
        differ=displacement[i-1]-displacement[i];
    else
        differ=0;
    
    if(differ<0)
        differ=0;
    if(par->algorithm==0)
        differ=0;
    
    for(m=0;m<differ+1;m++)
    {
        k=j+m+displacement[i];
        if((k>par->lenArray-1)||(k<0))
        {
            result=0.0;
            break;
        }
        
        result+=fbk[k][i];
        
    }
}
    
    
    
    
/*this function will calculate criterion of the filter bank*/    
double calculate_criterion(double **result,Parameter *par)
{
    int i, j, m;
    double *criterion, cri;
    //	FILE *fp;
    //	char file[FILENAME_MAX];  // name of data file
    
    
    if( !(criterion = malloc( par->dedisVal * sizeof(double))) )
        fprintf(stdout,"memory error \n");
    
    for(j=0;j<par->dedisVal;j++)
        criterion[j]=0.0;
    
    for(j=0;j<par->dedisVal;j++)
        for(i=par->lowFreqIndex;i<par->numChannel;i++)
            criterion[j]+=result[j][i];
    m=max_in(criterion, par->dedisVal);
    cri=criterion[m];
    //	fprintf(stdout,"\nOutput t-criterion\n   (if none, abort program): ");
    //	scanf("%s", file);
    /*	if(!(fp = fopen("tc.txt", "w")))
     {
     fprintf(stdout,"   File tc.txt could not be opened!");
     exit(EXIT_FAILURE);
     }
     
     for(j=0;j<NF/(NTAO*NT);j++)
     fprintf(fp, "%e %e\n", (j*1.0)*NT*NTAO/(FS*1.0), criterion[j]);
     
     
     fclose(fp);
     */
    
    free(criterion);
    return cri;
}

max_in(double *a, int n)
{
    int i, max_i;
    double t;
    t=a[0];                        /*initial */
    max_i=0;
    
    for(i=1;i<n;i++)
    {
        if (t<a[i])                  /*compare*/
        {
            t=a[i];
            max_i=i;
        }
    }
    
    return max_i;
}



