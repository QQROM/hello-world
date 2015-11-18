//                     MSP430F5529
//                     ____________
//                    |        P3.4|->RX
//                    |        P3.3|->TX
//                    |            |
// General Interrupt<-|P1.2    P3.0|->Trigger signal
// General Interrupt<-|P1.3    P3.1|->Trigger signal
// General Interrupt<-|P1.4    P3.2|->Trigger signal
// General Interrupt<-|P1.6    P3.6|->Trigger signal
// General Interrupt<-|P2.0    P3.7|->Trigger signal
// General Interrupt<-|P2.2    P4.0|->Trigger signal
// General Interrupt<-|P2.3    P4.1|->Trigger signal
// General Interrupt<-|P2.4    P4.2|->Trigger signal
// General Interrupt<-|P2.5    P4.3|->Trigger signal
// General Interrupt<-|P2.6    P6.0|->Trigger signal
// General Interrupt<-|P2.7    P6.1|->Trigger signal       
//                    |____________|

#define msp430_ID 0
#define Ultrasound_Number 1

#define yo_30C  *((unsigned int *)0x1A1A)
#define yo_85C  *((unsigned int *)0x1A1C)

#include  <msp430F5529.h>

unsigned int Ultrasound_ID = 0;
unsigned int echo_buffer = 0;
unsigned int echo_start = 0;
unsigned int Distance = 0;
unsigned int Tigger = 0;
unsigned int ID = 0;
unsigned int i = 0;
unsigned int RX_value = 0;
unsigned int data_ID[Ultrasound_Number];
unsigned int Third_data = 0;
unsigned int Second_data = 0;
unsigned int First_data = 0;
unsigned int msp430_node = 0;
unsigned int Packet_reset = 0;
unsigned int temp = 0;

void Select_Ultrasound_ID(void);
void Get_Ultrasound_Data(void);
void Get_Ultrasound_Data2(void);
void StartUp_UART(void);
void ShutDown_UART(void);
void Packet(void);
void Temperature(void);

void main(void)
{
  WDTCTL =  WDTPW + WDTHOLD;      // close WtachDogTimer
  
  P3SEL |=  BIT3 + BIT4;
  
  P3DIR |=  BIT0 + BIT1 + BIT2 + BIT5 +BIT6 + BIT7;
  P3OUT &=~(BIT0 + BIT1 + BIT2 + BIT5 +BIT6 + BIT7);
  
  P4DIR |=  BIT0 + BIT1 + BIT2 + BIT3;
  P4OUT &=~(BIT0 + BIT1 + BIT2 + BIT3);
  
  P6DIR |=  BIT0 + BIT1;
  P6OUT &=~(BIT0 + BIT1);
  
  UCSCTL3 = SELREF1 + FLLREFDIV_5;
  UCSCTL0 = 0x0000;
  UCSCTL1 = DCORSEL_0; 
  UCSCTL2 = 488;
  
  __delay_cycles(500000);
  
  StartUp_UART();                
  
  TA0CCTL0 = CCIE;
  TA0CCR0  = 60000-1;
  TA0CTL   = TASSEL_2 + MC_1;

  P1IE  &=~(BIT2 + BIT3 + BIT4 +BIT5 + BIT6);
  P1IES &=~(BIT2 + BIT3 + BIT4 +BIT5 + BIT6);
  P1IFG &=~(BIT2 + BIT3 + BIT4 +BIT5 + BIT6);
  
  P2IE  &=~(BIT0 + BIT2 + BIT3 + BIT4 +BIT5 + BIT6 + BIT7);
  P2IES &=~(BIT0 + BIT2 + BIT3 + BIT4 +BIT5 + BIT6 + BIT7);
  P2IFG &=~(BIT0 + BIT2 + BIT3 + BIT4 +BIT5 + BIT6 + BIT7);
  
  REFCTL0   &=~REFMSTR;
  ADC12CTL0  = ADC12SHT0_8 + ADC12REFON + ADC12ON;
  ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10;
  ADC12CTL1  = ADC12SHP;
  ADC12CTL0 |= ADC12ENC;
  
  UCA0IE |= UCRXIE;
  
  __bis_SR_register(GIE);                  // General Interrupt Enable
  
  TA0R = 0;                     
  
  while(1)
  {
   Temperature();
  }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  Select_Ultrasound_ID();
  
  //Enter I/O interrupt
  
  //Ultrasound_Data
}

void Select_Ultrasound_ID(void)
{
  Tigger = i % Ultrasound_Number;
  
  UCA0IE &=~UCRXIE;
  
  switch(Tigger)
  {
      case 0:
          Ultrasound_ID = BIT2;
          P1IE  |= BIT2;
          P1IE  &=~(BIT3 + BIT4 + BIT5 + BIT6);
          P2IE  &=~(BIT0 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
          P3OUT |= BIT0;
      break;
      case 1:
          Ultrasound_ID = BIT3;
          P1IE  |= BIT3;
          P1IE  &=~(BIT2 + BIT4 + BIT5 + BIT6);
          P2IE  &=~(BIT0 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
          P3OUT |= BIT1;
      break;
      case 2:
          Ultrasound_ID = BIT4;
          P1IE  |= BIT4;
          P1IE  &=~(BIT2 + BIT3 + BIT5 + BIT6);
          P2IE  &=~(BIT0 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
          P3OUT |= BIT2;
      break;
      case 3:
          Ultrasound_ID = BIT5;
          P1IE  |= BIT5;
          P1IE  &=~(BIT2 + BIT3 + BIT4 + BIT6);
          P2IE  &=~(BIT0 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
          P3OUT |= BIT5;
      break;
      case 4:
          Ultrasound_ID = BIT6;
          P1IE  |= BIT6;
          P1IE  &=~(BIT2 + BIT3 + BIT4 + BIT5);
          P2IE  &=~(BIT0 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
          P3OUT |= BIT6;
      break;
      case 5:
          Ultrasound_ID = BIT0;
          P2IE  |= BIT0;
          P1IE  &=~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6);
          P2IE  &=~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
          P3OUT |= BIT7;
      break;
      case 6:
          Ultrasound_ID = BIT2;
          P2IE  |= BIT2;
          P1IE  &=~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6);
          P2IE  &=~(BIT0 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
          P4OUT |= BIT0;
      break;
      case 7:
          Ultrasound_ID = BIT3;
          P2IE  |= BIT3;
          P1IE  &=~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6);
          P2IE  &=~(BIT0 + BIT2 + BIT4 + BIT5 + BIT6 + BIT7);
          P4OUT |= BIT1;
      break;
      case 8:
          Ultrasound_ID = BIT4;
          P2IE  |= BIT4;
          P1IE  &=~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6);
          P2IE  &=~(BIT0 + BIT2 + BIT3 + BIT5 + BIT6 + BIT7);
          P4OUT |= BIT2;
      break;
      case 9:
          Ultrasound_ID = BIT5;
          P2IE  |= BIT5;
          P1IE  &=~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6);
          P2IE  &=~(BIT0 + BIT2 + BIT3 + BIT4 + BIT6 + BIT7);
          P4OUT |= BIT3;
      break;
      case 10:
          Ultrasound_ID = BIT6;
          P2IE  |= BIT6;
          P1IE  &=~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6);
          P2IE  &=~(BIT0 + BIT2 + BIT3 + BIT4 + BIT5 + BIT7);
          P6OUT |= BIT0;
      break;
      case 11:
          Ultrasound_ID = BIT7;
          P2IE  |= BIT7;
          P1IE  &=~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6);
          P2IE  &=~(BIT0 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6);
          P6OUT |= BIT1;
      break;
  }
  __delay_cycles(6);
  P3OUT = 0x00;
  P4OUT = 0x00;
  P5OUT = 0x00;
  TA0R = 0;
  i++;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  echo_start = TA0R;
  
  Get_Ultrasound_Data();
  
  P1IFG &=~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6);                        
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
  echo_start = TA0R;
  
  Get_Ultrasound_Data2();
  
  P2IFG &=~(BIT0 +BIT2 + BIT3 + BIT4 +BIT5 + BIT6 + BIT7);                          
}

void Get_Ultrasound_Data(void)
{
  do
  {
      echo_buffer = P1IN & Ultrasound_ID;
  }while( echo_buffer != 0x00);
  
  Distance = TA0R - echo_start;
  
  UCA0IE |= UCRXIE;
  
  switch(Ultrasound_ID)
  {
    case 0x04:ID=0;break;
    case 0x08:ID=1;break;
    case 0x10:ID=2;break;
    case 0x20:ID=3;break;
    case 0x40:ID=4;break;
  }
  data_ID[ID]=Distance;
}

void Get_Ultrasound_Data2(void)
{
  do
  {
      echo_buffer = P2IN & Ultrasound_ID;
  }while( echo_buffer != 0x00);
  
  Distance = TA0R - echo_start;
  
  UCA0IE |= UCRXIE;
  
  switch(Ultrasound_ID)
  {
    case 0:ID=5;break;
    case 2:ID=6;break;
    case 3:ID=7;break;
    case 4:ID=8;break;
    case 5:ID=9;break;
    case 6:ID=10;break;
    case 7:ID=11;break;
  }
  data_ID[ID]=Distance;
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  while (!(UCA0IFG&UCRXIFG));
  
  RX_value = UCA0RXBUF;                     // receive character
  
  msp430_node = RX_value >> 4;
  
  if(msp430_node == msp430_ID)
  {   
    Packet_reset = RX_value & 0x0f;
    
    Packet();
    
    while (!(UCA0IFG&UCTXIFG));
    
    UCA0TXBUF = Third_data;                    // Transmit character
    
    while (!(UCA0IFG&UCTXIFG));
    
    UCA0TXBUF = Second_data;                   // Transmit character
        
    while (!(UCA0IFG&UCTXIFG));
 
    UCA0TXBUF = First_data;                    // Transmit character
  }
}

void Packet(void)
{
  //0000 0000 0000 0000 <= data
  //           --- ---- <= get 7bit
  //          0--- ---- <= Packet data          
  Third_data =  data_ID[Packet_reset] & 0x7f ;
  
  //0000 0000 0000 0000 <= data
  //  -- ---- -         <= get 7bit
  //          0--- ---- <= Packet data 
  Second_data  = ( data_ID[Packet_reset] >> 7 ) & 0x7f;
  
  //0000 0000 0000 0000 <= data
  //--                  <= get 2bit
  //          1??? ??-- <= Packet data 
  First_data = (( data_ID[Packet_reset] >> 14) | 0x80) + temp;
}

void Temperature(void)
{
   ADC12CTL0 |= ADC12SC;
   while((ADC12BUSY & ADC12CTL1));
   temp = (float)(((long) ADC12MEM0 - yo_30C) * (85 - 30)) / (yo_85C - yo_30C) + 26.5f;
}

void StartUp_UART(void)
{
  UCA0CTL1 |= UCSSEL_2;
  UCA0BR0 = 104;                            // 1MHz 9600
  UCA0BR1 = 0;                              // 1MHz 9600
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

}

void ShutDown_UART(void)
{
  UCA0CTL1 |= UCSWRST;                           // Disable UART
}
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
1
