//                     MSP430G2553
//                     ____________
//                    |            |
//                    |        P1.1|->RX
//                    |        P1.2|->TX
//                    |            |
//    Trigger signal<-|P2.0    P1.0|->General Interrupt
//    Trigger signal<-|P2.1    P1.3|->General Interrupt
//    Trigger signal<-|P2.2    P1.4|->General Interrupt
//    Trigger signal<-|P2.3    P1.5|->General Interrupt
//    Trigger signal<-|P2.4    P1.6|->General Interrupt
//    Trigger signal<-|P2.5    P1.7|->General Interrupt
//                    |____________|

#define msp430_ID 0
#define Ultrasound_Number 4

#include  <msp430g2553.h>

unsigned int Ultrasound_ID = 0;
unsigned int echo_buffer = 0;
unsigned int echo_start = 0;
unsigned int Distance = 0;
unsigned int Tigger = 0;
unsigned int ID = 0;
unsigned int i = 0;
unsigned int RX_value = 0;
unsigned int data_ID[6];
unsigned int Third_data = 0;
unsigned int Second_data = 0;
unsigned int First_data = 0;
unsigned int msp430_node = 0;
unsigned int Packet_reset = 0;
unsigned int temp = 0;

void Select_Ultrasound_ID(void);
void Get_Ultrasound_Data(void);
void StartUp_UART(void);
void ShutDown_UART(void);
void Packet(void);
void Temperature(void);

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;      // close WtachDogTimer
  
  P1SEL |= BIT1 + BIT2;                // P1.1 = RXD
  P1SEL2|= BIT1 + BIT2;                // P1.1 = RXD
  
  P2DIR |=  BIT0+BIT1+BIT2+BIT3+BIT4+BIT5;                 // P2.5 output        
  P2OUT &=~(BIT0+BIT1+BIT2+BIT3+BIT4+BIT5);                 // P2.5 Low
  
  BCSCTL1 = CALBC1_1MHZ;         // SMCLK 1MHz
  DCOCTL  = CALDCO_1MHZ;         // SMCLK 1MHz
  
  StartUp_UART();                // StartUp UART
  
  TACCTL0 = CCIE;                // TACCR0 interrupt enabled
  TACCR0  = 60000-1;               // PWM Period
  TACTL   = TASSEL_2 + MC_1;     // SMCLK, Up mode:the timer counts up to TACCR0.
  
  P1IE  &=~(BIT0+BIT3+BIT4+BIT5+BIT6+BIT7);// P1.3 P1.4 interrupt disabled
  P1IES &=~(BIT0+BIT3+BIT4+BIT5+BIT6+BIT7);// P1.3 P1.4 L->H edge
  P1IFG &=~(BIT0+BIT3+BIT4+BIT5+BIT6+BIT7);// P1.3 P1.4 IFG cleared
  
  ADC10CTL1 = INCH_10 + ADC10DIV_3; //選擇溫度感測 + 時間分頻/4?
  ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON;
  
  IE2 |= UCA0RXIE;
  
  _BIS_SR(GIE);                  // General Interrupt Enable
  
  TAR = 0;                     
  
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
  
  IE2 &=~UCA0RXIE;
  
  switch(Tigger)
  {
      case 0:
          Ultrasound_ID = BIT0;
          P1IE  |= BIT0;
          P1IE  &=~(BIT3+BIT4+BIT5+BIT6+BIT7);
          P2OUT |= BIT0;
      break;
      case 1:
          Ultrasound_ID = BIT3;
          P1IE  |= BIT3;
          P1IE  &=~(BIT0+BIT4+BIT5+BIT6+BIT7);
          P2OUT |= BIT1;
      break;
      case 2:
          Ultrasound_ID = BIT4;
          P1IE  |= BIT4;
          P1IE  &=~(BIT0+BIT3+BIT5+BIT6+BIT7);
          P2OUT |= BIT2;
      break;
      case 3:
          Ultrasound_ID = BIT5;
          P1IE  |= BIT5;
          P1IE  &=~(BIT0+BIT3+BIT4+BIT6+BIT7);
          P2OUT |= BIT3;
      break;
      case 4:
          Ultrasound_ID = BIT6;
          P1IE  |= BIT6;
          P1IE  &=~(BIT0+BIT3+BIT4+BIT5+BIT7);
          P2OUT |= BIT4;
      break;
      case 5:
          Ultrasound_ID = BIT7;
          P1IE  |= BIT7;
          P1IE  &=~(BIT0+BIT3+BIT4+BIT5+BIT6);
          P2OUT |= BIT5;
      break;
  }
  __delay_cycles(6);
  P2OUT = 0x00;
  TAR = 0;
  i++;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  echo_start = TAR;
  
  Get_Ultrasound_Data();
  
  P1IFG &=~(BIT0+BIT3+BIT4+BIT5+BIT6+BIT7);                          
}

void Get_Ultrasound_Data(void)
{
  do
  {
      echo_buffer = P1IN & Ultrasound_ID;
  }while( echo_buffer != 0x00);
  
  Distance = TAR - echo_start;
  
  IE2 |= UCA0RXIE;
  
  switch(Ultrasound_ID)
  {
    case 0x01:ID=0;break;
    case 0x08:ID=1;break;
    case 0x10:ID=2;break;
    case 0x20:ID=3;break;
    case 0x40:ID=4;break;
    case 0x80:ID=5;break;
  }
  data_ID[ID]=Distance;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  while (!(IFG2&UCA0RXIFG));                // USCI_A0 RX buffer ready?
  RX_value = UCA0RXBUF;                     // receive character
  
    Packet_reset = RX_value & 0x0f;
    
    Packet();
    
    while (!(IFG2&UCA0TXIFG));                 // USCI_A0 TX buffer ready?
    UCA0TXBUF = Third_data;                    // Transmit character
    
    while (!(IFG2&UCA0TXIFG));                 // USCI_A0 TX buffer ready?
    UCA0TXBUF = Second_data;                   // Transmit character
    
    while (!(IFG2&UCA0TXIFG));                 // USCI_A0 TX buffer ready?
    UCA0TXBUF = First_data;                    // Transmit character
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
  ADC10CTL0 |= ENC + ADC10SC; 
  while((ADC10BUSY & ADC10CTL1));
  temp = ((((ADC10MEM - 673) * 423) / 1024)-10) << 2;
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
