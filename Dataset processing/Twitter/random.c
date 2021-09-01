//random.c

#include <string>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cstring>

using namespace std;

const int LEN = 4098;

//返回属于[p,q)的随机整数 
int rand(int p, int q)
{
     int size = q-p+1;
     return  p+ rand()%size;
}

//交换两个元素值 
void swap(int& a , int& b)
{
     int temp = a;
     a = b;
     b = temp;
}

//打印数组值
void print(int *v, int n)
{
        for(int i=0; i < n ; i++)
        {
                printf("%u\n", v[i]);
        }
}
        
//给数组a[n], 随机不重复赋值[1,n]之间的数
void randomize(int *v, int n)
{       
        //initialize
        for(int i=0; i < n; i++)
        {       
                v[i] = i+1;
        }

        for(int i=n-1; i>0; i--)
        {
                int r = rand(0,i+1);
                swap(v[r], v[i]);
        }
}

//删除换行符
int chomp(char *str)
{
    int len = strlen(str);
    while(len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r'))
    {
        str[len - 1] = 0;
        len--;
    }
    return len;
}


//主函数
int main(int argc, char *argv[])
{       
        int line_num = atoi(argv[1]);
        printf("%u\n",line_num);
        int *value = (int*)malloc((line_num) * sizeof(int));
        printf("%u\n",line_num);
        randomize(value, line_num);
        //print(value, N);

        
        FILE* infile = fopen(argv[2], "r");
        if( infile == NULL )
        {       
                printf("Cann't open file %s.", argv[1]);
                return 0;
        }
        
        FILE* outfile = fopen(argv[3], "w");
        if( outfile == NULL)
        {       
                printf("Cann't open file %s to write.", argv[2]);
                return 0;
        }
        
        int i=0;
        char str[LEN];
        str[0] = 0;  
        str[LEN-1] = 0;

        while( !feof(infile) )
        {
                if( !fgets(str, sizeof(str),infile))
                {
                        break;
                }
                
                str[LEN- 1] = 0;
                chomp(str);
                
                fprintf(outfile, "%s\t%u\n", str, value[i]);
                i++;
        }
        fclose(infile);
        fclose(outfile);
        return 0;
}
