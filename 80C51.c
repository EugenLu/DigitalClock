#include <REGX52.H>

#define uint unsigned int
#define uchar unsigned char
    
sbit K1=P3^1;                       //模式设置按键
sbit K2=P3^0;                       //数码管定位按键
sbit K3=P3^2;                       //时分秒加一按键

sbit ALARM=P3^5;                    //闹铃控制针脚

uchar tst=1;                        //计时器状态标志
uchar s=0;                          //1秒计时信号
uchar ms=0;                         //10ms计时信号
uchar Mode=0;                       //显示模式（现时、暂时）
uchar Lct=0;                        //数码管定位位置
uchar Alm=0;                        //闹铃开始标志

uchar sec1=0;   uchar sec2=0;       //当前秒
uchar min1=0;   uchar min2=0;       //当前分
uchar hour1=0;  uchar hour2=0;      //当前时

uchar st1=0;    uchar st2=0;        //设置秒
uchar mt1=0;    uchar mt2=0;        //设置分
uchar ht1=0;    uchar ht2=0;        //设置时

uchar sa1=0;    uchar sa2=0;        //闹铃秒
uchar ma1=0;    uchar ma2=0;        //闹铃分
uchar ha1=0;    uchar ha2=0;        //闹铃时

uchar Number[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};     //显数0~9

void Delay(uchar ms);                //毫秒软件延时
void Timer0();                      //开定时器

void Clock();                       //用来实时整秒走时   

void msTimer();                     //用来计时器毫秒计时

void ShowTime();                    //数码管显示当前时间

void Scankey();                     //扫描按键

void Alarm();                       //判断是否开启闹铃

void main(){
    P0=0x00;
    P2=0x00;
    P3=0xFF;
    Timer0();                       //定时器初始化，开定时中断，开定时器
	while(1){
        if(Mode==1){ET0=0;}         //仅设置时间时关闭定时器0，停止走时
        else {ET0=1;}
        if(Mode==3){                //仅使用计时器时开定时器1，控制开始与停止
            if(tst==0){ET1=1;}
            else if(tst==1){ET1=0;}
        }
        else {ET1=0;}
        Clock();                    //定时器0后台计时整秒
        msTimer();                  //定时器1后台计时10ms
        ShowTime();                 //数码管实时显示当前的数
        Scankey();                  //单片机实时检测各按键状态
        Alarm();                    //判断是否已到闹铃时间
        if(Alm==1){                 //若已到时间则开始闹铃
            ALARM=0;Delay(500);
            ALARM=1;Alm=0;
        }
    }    
}

void Scankey(){
//设置模式Mode。Mode0正常走时，Mode1设置时间，Mode2设置闹钟，Mode3计时器
    if(K1==0){
        Delay(20);
        if(K1==0){
            Mode++;
            Mode%=4;
            if(Mode==1){            //设置时间的初始值              
                st1=sec1;st2=sec2;
                mt1=min1;mt2=min2;
                ht1=hour1;ht2=hour2;
            }
            else if(Mode==3){       //计时器的初始值
                st1=0;st2=0;mt1=0;mt2=0;ht1=0;ht2=0;
            }
            Lct=0;                  //数码管设置初始位置
        }
        while(!K1);
    }    
    
//定位数码管，按下按键K2，将设置的数码管往右移一位（仅在Mode1和Mode2下生效）
    if(K2==0&&(Mode==1||Mode==2)){  
        Delay(20);
        if(K2==0&&Mode!=0){Lct++;Lct%=8;}
        while(!K2);
    }        
 
//设置时间，按下按键K3，将设置的数码管显示的数值加一，并将设置好的时间赋值给当前时间
    if(K3==0&&Mode==1){
        Delay(20);
        if(K3==0&&Mode==1){
            while(1){
                switch(Lct){
                    case 0:
                        ht1++;ht1%=3;break;
                    case 1:
                        ht2++;ht2%=10;break;
                    case 2:
                        break;
                    case 3:
                        mt1++;mt1%=6;break;
                    case 4:
                        mt2++;mt2%=10;break;
                    case 5:
                        break;
                    case 6:
                        st1++;st1%=6;break;
                    case 7:
                        st2++;st2%=10;break;
                }
                sec1=st1;sec2=st2;
                min1=mt1;min2=mt2;
                hour1=ht1;hour2=ht2;
                break;
            }
        }
    while(!K3);
    }

//设置闹钟时间，按下按键K3，将设置的数码管显示的数值加一，设置的数即为闹钟时间
    if(K3==0&&Mode==2){
        Delay(20);
        if(K3==0&&Mode==2){
            while(1){
                switch(Lct){
                    case 0:
                        ha1++;ha1%=3;break;
                    case 1:
                        ha2++;ha2%=10;break;
                    case 2:
                        break;
                    case 3:
                        ma1++;ma1%=6;break;
                    case 4:
                        ma2++;ma2%=10;break;
                    case 5:
                        break;
                    case 6:
                        sa1++;sa1%=6;break;
                    case 7:
                        sa2++;sa2%=10;break;
                }
                break;
            }
        }
        while(!K3);
    }

//计时器开始与暂停，在Mode3下，按K2控制计时器的开始与停止
    if(K2==0&&Mode==3){
        Delay(20);
        if(K2==0&&Mode==3){tst++;tst%=2;}
        while(!K2);
    }
    
//计时器清零复位
    if(K3==0&&Mode==3&&tst==1){
        Delay(20);
        if(K3==0&&Mode==3){
            st1=0;st2=0;mt1=0;mt2=0;ht1=0;ht2=0;            
        }
        while(!K3);
    }
}

void Alarm(){                       //判断是否闹铃
    if(hour1==ha1&&hour2==ha2&&min1==ma1&&min2==ma2&&sec1==sa1&&sec2==sa2){Alm=1;}
    }

void ShowTime(){                    //数码管显示
    if(Mode==0){                    //Mode0下显示当前实时走时
        int L=1;
        for(L;L<9;L++){
            switch(L){
                case 1:P2_4=1;P2_3=1;P2_2=1;P0=Number[hour1];Delay(1);P0=0x00;break;
                case 2:P2_4=1;P2_3=1;P2_2=0;P0=Number[hour2];Delay(1);P0=0x00;break;
                case 3:P2_4=1;P2_3=0;P2_2=1;P0=0x40;Delay(1);P0=0x00;break;
                case 4:P2_4=1;P2_3=0;P2_2=0;P0=Number[min1];Delay(1);P0=0x00;break;
                case 5:P2_4=0;P2_3=1;P2_2=1;P0=Number[min2];Delay(1);P0=0x00;break;
                case 6:P2_4=0;P2_3=1;P2_2=0;P0=0x40;Delay(1);P0=0x00;break;
                case 7:P2_4=0;P2_3=0;P2_2=1;P0=Number[sec1];Delay(1);P0=0x00;break;
                case 8:P2_4=0;P2_3=0;P2_2=0;P0=Number[sec2];Delay(1);P0=0x00;break;
            }
        }
    }
    else if(Mode==1||Mode==3){      //Mode1下显示当前设置值，Mode3下显示当前计时器走时
        int L=1;
        for(L;L<9;L++){
            switch(L){
                case 1:P2_4=1;P2_3=1;P2_2=1;P0=Number[ht1];Delay(1);P0=0x00;break;
                case 2:P2_4=1;P2_3=1;P2_2=0;P0=Number[ht2];Delay(1);P0=0x00;break;
                case 3:P2_4=1;P2_3=0;P2_2=1;P0=0x40;Delay(1);P0=0x00;break;
                case 4:P2_4=1;P2_3=0;P2_2=0;P0=Number[mt1];Delay(1);P0=0x00;break;
                case 5:P2_4=0;P2_3=1;P2_2=1;P0=Number[mt2];Delay(1);P0=0x00;break;
                case 6:P2_4=0;P2_3=1;P2_2=0;P0=0x40;Delay(1);P0=0x00;break;
                case 7:P2_4=0;P2_3=0;P2_2=1;P0=Number[st1];Delay(1);P0=0x00;break;
                case 8:P2_4=0;P2_3=0;P2_2=0;P0=Number[st2];Delay(1);P0=0x00;break;
            }
        }
    }
    else if(Mode==2){               //Mode2下显示设置的闹钟时间
        int L=1;
        for(L;L<9;L++){
            switch(L){
                case 1:P2_4=1;P2_3=1;P2_2=1;P0=Number[ha1];Delay(1);P0=0x00;break;
                case 2:P2_4=1;P2_3=1;P2_2=0;P0=Number[ha2];Delay(1);P0=0x00;break;
                case 3:P2_4=1;P2_3=0;P2_2=1;P0=0x40;Delay(1);P0=0x00;break;
                case 4:P2_4=1;P2_3=0;P2_2=0;P0=Number[ma1];Delay(1);P0=0x00;break;
                case 5:P2_4=0;P2_3=1;P2_2=1;P0=Number[ma2];Delay(1);P0=0x00;break;
                case 6:P2_4=0;P2_3=1;P2_2=0;P0=0x40;Delay(1);P0=0x00;break;
                case 7:P2_4=0;P2_3=0;P2_2=1;P0=Number[sa1];Delay(1);P0=0x00;break;
                case 8:P2_4=0;P2_3=0;P2_2=0;P0=Number[sa2];Delay(1);P0=0x00;break;
            }
        }
    }        
}     

void Clock(){                       //正常整秒走时
    if(s==1)    {   s=0;    sec2++;  }
    if(sec2==10){   sec2=0; sec1++;  }
    if(sec1==6) {   sec1=0; min2++;  }
    if(min2==10){   min2=0; min1++;  }
    if(hour1==2){   if(hour2==4){hour1=0;hour2=0;}}
    if(min1==6) {   min1=0; hour2++; }
    if(hour2==10){  hour2=0;hour1++; }
}

void msTimer(){                     //正常毫秒计时
    if(ms==1)  {ms=0;st2++;}
    if(st2==10){st2=0;st1++;}
    if(st1==10){st1=0;mt2++;}
    if(mt2==10){mt2=0;mt1++;}
    if(mt1==6) {mt1=0;ht2++;}
    if(ht2==10){ht2=0;ht1++;}
    if(ht1==6) {ht1=0;}
}

void Delay(uchar xms){               //软件延时（毫秒级）
	uchar i, j;
	while(xms--){
		i = 2;
        j = 239;
		do{while (--j);} 
        while (--i);
	}
} 


void Timer0(){                      //定时器初始化
	TMOD = 0x11;                    //定时器0和1都设置01工作方式
    
	TL0=64536%256;  TH0=64536/256;  //定时器0设置初始值
    
    TL1=64536%256;  TH1=64536/256;  //定时器1设置初始值
    
	TF0 = 0;        TR0 = 1;        //开定时器0
    
    TF1 = 0;        TR1 = 1;        //开定时器1
    
    ET1=1;          ET0=1;          //开定时器中断0和1
    
	EA=1;	                        //开总中断
}

 void TimerFor1Second() interrupt 1{            //硬件定时（1秒）
	static uint c = 0;	
	TL0=64536%256;
	TH0=64536/256;
	c++;	
	if(c>=1000){
		c=0;
        s=1;
    }    
}
 
void TimerFor10MicroSecond() interrupt 3{       //硬件定时（10毫秒）
    static uint d =0;              
    TL1=64536%256;
	TH1=64536/256;
    d++;
    if(d>=10){
        d=0;
        ms=1;        
    }
}    