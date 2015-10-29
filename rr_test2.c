//4)	ROUND ROBIN:

#include<stdio.h> 
static int n;
int i,j,total_time=0;//time quantum
float avgtat=0,avgwt=0;
int queue[];
int time=0,front=0,rear=0,q_count=0,m=0,s=0;

struct data
{
int at,st,ct,tat,wt;
char pname[20];
float ntat;
}temp;
main()
{
printf("enter the no of process\n");
scanf("%d",&n);

struct data aa[n];
for(i=0;i<n;i++)
{
printf("enter the name of process %d,at,st\n",i);
scanf("%s",&aa[i].pname);
scanf("%d%d",&aa[i].at,&aa[i].st);
}

//sorting
for(i=0;i<n;i++)
{
for(j=i;j>=1;j--)
{
if(aa[j].at<aa[j-1].at)
{
temp=aa[j-1];
aa[j-1]=aa[j];
aa[j]=temp;
}
else if(aa[j].at==aa[j-1].at)
{
if(aa[j].st<aa[j-1].st)
{
temp=aa[j-1];
aa[j-1]=aa[j];
aa[j]=temp;
}
}
}
}
//finding the total time
total_time+=aa[0].at+aa[0].st;
for(i=1;i<n;i++)
{
if(aa[i].at>total_time)
total_time=aa[i].at;//coz if the next process arives after a while after the b4 process has //complted
total_time+=aa[i].st;
}

//finding the ct
finding(aa);

}

void finding(struct data a[])
{


int temp_st[n],flag=0,count=0,p_process;//present process, flag to denote if a process is selected
j=0;
int tq;
printf("enter the time quantum\n");
scanf("%d",&tq);
for(i=0;i<n;i++)
{
temp_st[i]=a[i].st;
}


time=a[0].at;


q_count=1;
push(j++);
while(time<=total_time)
{
if(flag==1||q_count!=0)//ie i enetrs if a process is already selected or a new process can be selected
{
if(flag==0&&count==0)
{

p_process=pop();//queue[front];

//p_process=i;
count=0;
flag=1;

}
temp_st[p_process]--;

if(temp_st[p_process]==0)
{
time++;
count=0;
a[p_process].ct=time;
flag=0;//a[i].st+=1000;
q_count--;
check(a);
continue;
}
count++;
//count=count%tq;
if(count==tq)
{
count=0;
time++;
check(a);
push(p_process);
flag=0;
}
else
{
time++;
check(a);
}
}
else
{
time++;
check(a);
}


}//end of while loop
display(a);
}
void push(int q)
{
queue[rear++]=q;
//rear%=n;
m++;
}
int pop()
{
s++;
int x;
x=queue[front++];

return x;
}

void check(struct data a[])
{
while(a[j].at<=time&&j<n)//coz all tthe process must b checked b4 pushing the presently executed process into the queue
{
q_count++;	
push(j++);
}
}
void display(struct data a[])
{
//n=5;

printf("n=%d	\n",n);

for(i=0;i<n;i++)
{
a[i].tat=a[i].ct-a[i].at;
a[i].wt=a[i].tat-a[i].st;
a[i].ntat=(float)a[i].tat/a[i].st;
}
for(i=0;i<n;i++)
{
avgtat+=a[i].tat;
avgwt+=a[i].wt;
}
avgtat=avgtat/n;
avgwt=avgwt/n;
printf("pname\tat\tst\tct\ttat\twt\tntat\n");
for(i=0;i<n;i++)
{
printf("%s\t%d\t%d\t%d\t%d\t%d\t%f\n",a[i].pname,a[i].at,a[i].st,a[i].ct,a[i].tat,a[i].wt,a[i].ntat);
}
printf("avgtat=%f\navgwt=%f\n",avgtat,avgwt);

}
/*
INPUT:
enter the no of process
5
enter the name of process 0,at,st
a 0 3
enter the name of process 1,at,st
b 2 6
enter the name of process 2,at,st
c 4 4
enter the name of process 3,at,st
d 6 5
enter the name of process 4,at,st
e 8 2
enter the time quantum
1
OUTPUT:
n=5
pname   at      st      ct      tat     wt      ntat
a       0       3       4       4       1       1.333333
b       2       6       18      16      10      2.666667
c       4       4       17      13      9       3.250000
d       6       5       20      14      9       2.800000
e       8       2       15      7       5       3.500000
avgtat=10.800000
avgwt=6.800000
*/