#include "lcd.h"
#include "actuators/rk043fn48h.h"
#include "actuators/RGB565_480x272.h"
#include "pin.h"

void lcd_init(){

  /*Our Startup config*/
  /*
  PLLSRC = HSI
  PLLQ = 7
  PLLP = 1
  PLLN = 336
  PLLM = 25
  */
  printf("lcd_init()\r\n");

    /* LCD clock configuration */
    /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
    /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
    /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/5 = 38.4 Mhz */
    /* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_4 = 38.4/4 = 9.6Mhz */

  // In our case PLLSAI_VCO_INPUT = HSI_VALUE/PLL_M = 1MHz
  // 
  RCC->PLLSAICFGR &= ~RCC_PLLSAICFGR_PLLSAIN &
                     ~RCC_PLLSAICFGR_PLLSAIR ;
  RCC->PLLSAICFGR |=  ((192<<6)&RCC_PLLSAICFGR_PLLSAIN) | ((5 << 28) & RCC_PLLSAICFGR_PLLSAIR); // multiply by 192 and divide by 5
  RCC->DCKCFGR1   &= ~RCC_DCKCFGR1_PLLSAIDIVR; // 
  RCC->DCKCFGR1   |=  RCC_DCKCFGR1_PLLSAIDIVR_0; // divide by 4
  // 1 * 192 / 5 / 4 = 192 / 20 = 9.6 MHz
  RCC->CR         |=  RCC_CR_PLLSAION;
  printf("waiting for PLLSAI to lock\r\n");
  while( (RCC->CR & RCC_CR_PLLSAIRDY) != RCC_CR_PLLSAIRDY );
  printf("PLLSAI locked\r\n");
  RCC->APB2ENR    |= RCC_APB2ENR_LTDCEN;        // turn on LCD clock
  RCC->APB2RSTR   |=  RCC_APB2RSTR_LTDCRST;     // Reset controller
  RCC->APB2RSTR   &= ~RCC_APB2RSTR_LTDCRST;

  lcd_init_pins();

  LTDC->GCR &= ~LTDC_GCR_LTDCEN;
  /* LCD CONFIG */
  //GCR Register 
  LTDC->GCR   &=  ~LTDC_GCR_HSPOL      & // HSYNC active Low
                  ~LTDC_GCR_VSPOL      & // VSYNC active Low
                  ~LTDC_GCR_DEPOL      & // Data Enable active Low
                  ~LTDC_GCR_PCPOL      ; // Use in Phase pixel clock


/*ST CONFIGS*/
uint32_t HorizontalSync = (RK043FN48H_HSYNC - 1);
uint32_t VerticalSync = (RK043FN48H_VSYNC - 1);
uint32_t AccumulatedHBP = (RK043FN48H_HSYNC + RK043FN48H_HBP - 1);
uint32_t AccumulatedVBP = (RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
uint32_t AccumulatedActiveH = (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
uint32_t AccumulatedActiveW = (RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP - 1);
uint32_t TotalHeigh = (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP + RK043FN48H_VFP - 1);
uint32_t TotalWidth = (RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP + RK043FN48H_HFP - 1);
//Layer variables
uint32_t WindowX0 = 0;
uint32_t WindowX1 = 480;
uint32_t WindowY0 = 0;
uint32_t WindowY1 = 272;
uint32_t PixelFormat = ((uint32_t)0x00000002); //LTDC_PIXEL_FORMAT_RGB565;
uint32_t FBStartAdress = (uint32_t)&RGB565_480x272;
uint32_t Alpha = 0xFF;
uint32_t BlendingFactor1 = ((uint32_t)0x00000400);//LTDC_BLENDING_FACTOR1_CA;
uint32_t BlendingFactor2 = ((uint32_t)0x00000005);//LTDC_BLENDING_FACTOR2_CA;
uint32_t ImageWidth  = 480;
uint32_t ImageHeight = 272;
uint32_t tmp;
// desperate attempt
LTDC->SSCR &= ~(LTDC_SSCR_VSH | LTDC_SSCR_HSW);
LTDC->SSCR |= ((HorizontalSync << 16) | VerticalSync);

LTDC->BPCR &= ~(LTDC_BPCR_AVBP | LTDC_BPCR_AHBP);
LTDC->BPCR |= ((AccumulatedHBP << 16) | AccumulatedVBP);

LTDC->AWCR &= ~(LTDC_AWCR_AAH | LTDC_AWCR_AAW);
LTDC->AWCR |= ((AccumulatedActiveW << 16) | AccumulatedActiveH);

LTDC->TWCR &= ~(LTDC_TWCR_TOTALH | LTDC_TWCR_TOTALW);
LTDC->TWCR |= ((TotalWidth << 16) | TotalHeigh); 

LTDC->BCCR &= ~(LTDC_BCCR_BCBLUE | LTDC_BCCR_BCGREEN | LTDC_BCCR_BCRED);

LTDC->IER |= LTDC_IER_FUIE                     |
             LTDC_IER_TERRIE                   ;

LTDC->GCR |= LTDC_GCR_LTDCEN;

  /* Configures the horizontal start and stop position */
  tmp = ((WindowX1 + ((LTDC->BPCR & LTDC_BPCR_AHBP) >> 16)) << 16);
  LTDC_Layer1->WHPCR &= ~(LTDC_LxWHPCR_WHSTPOS | LTDC_LxWHPCR_WHSPPOS);
  LTDC_Layer1->WHPCR = ((WindowX0 + ((LTDC->BPCR & LTDC_BPCR_AHBP) >> 16) + 1) | tmp);

  /* Configures the vertical start and stop position */
  tmp = ((WindowY1 + (LTDC->BPCR & LTDC_BPCR_AVBP)) << 16);
  LTDC_Layer1->WVPCR &= ~(LTDC_LxWVPCR_WVSTPOS | LTDC_LxWVPCR_WVSPPOS);
  LTDC_Layer1->WVPCR  = ((WindowY0 + (LTDC->BPCR & LTDC_BPCR_AVBP) + 1) | tmp);  

  /* Specifies the pixel format */
  LTDC_Layer1->PFCR &= ~(LTDC_LxPFCR_PF);
  LTDC_Layer1->PFCR = (PixelFormat);

  /* Configures the default color values */
  LTDC_Layer1->DCCR &= ~(LTDC_LxDCCR_DCBLUE | LTDC_LxDCCR_DCGREEN | LTDC_LxDCCR_DCRED | LTDC_LxDCCR_DCALPHA);

  /* Specifies the constant alpha value */
  LTDC_Layer1->CACR &= ~(LTDC_LxCACR_CONSTA);
  LTDC_Layer1->CACR = (Alpha);

  /* Specifies the blending factors */
  LTDC_Layer1->BFCR &= ~(LTDC_LxBFCR_BF2 | LTDC_LxBFCR_BF1);
  LTDC_Layer1->BFCR = (BlendingFactor1 | BlendingFactor2);

  /* Configures the color frame buffer start address */
  LTDC_Layer1->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);
  LTDC_Layer1->CFBAR = (FBStartAdress);
  /* Configures the color frame buffer pitch in byte */
  LTDC_Layer1->CFBLR  &= ~(LTDC_LxCFBLR_CFBLL | LTDC_LxCFBLR_CFBP);
  LTDC_Layer1->CFBLR  = (((ImageWidth * tmp) << 16) | (((WindowX1 - WindowX0) * tmp)  + 3));

  /* Configures the frame buffer line number */
  LTDC_Layer1->CFBLNR  &= ~(LTDC_LxCFBLNR_CFBLNBR);
  LTDC_Layer1->CFBLNR  = (ImageHeight);

  /* Enable LTDC_Layer by setting LEN bit */  
  LTDC_Layer1->CR |= (uint32_t)LTDC_LxCR_LEN;

  LTDC->SRCR = LTDC_SRCR_IMR;
//  // SSCR (Size register)
//  LTDC->SSCR  |=   RK043FN48H_VSYNC    | // Use ST values without thinking
//                   RK043FN48H_HSYNC<<16; // Think about it if problem
//  // Front/Back Porch (Defning active area)
//  LTDC->BPCR  |=   (( (RK043FN48H_HSYNC + RK043FN48H_HBP - 1) << 16) & LTDC_BPCR_AHBP) |   // setting horizontal accumulated Back Porch (HSYNC + HBP)
//                   ( (RK043FN48H_VSYNC + RK043FN48H_VBP - 1) &  LTDC_BPCR_AVBP )       ; // (VSYNC + VBP) 
//  LTDC->AWCR  |=   (((RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP - 1 ) <<16) & LTDC_AWCR_AAW) | // active area = BP + SYNC + Pixels used 
//                   ((RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP - 1 )  & LTDC_AWCR_AAH)   ; 
//  LTDC->TWCR  |=   (((RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP - 1 + RK043FN48H_HFP )<<16)&LTDC_TWCR_TOTALW)  | // Resulting total size
//                   ((RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP - 1 + RK043FN48H_VFP )& LTDC_TWCR_TOTALH) ;
//
//  //FIXME Just to confirm that it works lets put a red BG --> should put 0,0,0 for real application
//  LTDC->BCCR |=   ((0x0 << 16)   & LTDC_BCCR_BCRED)   |
//                  ((0x0 << 8)    & LTDC_BCCR_BCGREEN) |
//                  (0x0           & LTDC_BCCR_BCBLUE ) ;
//
//
//  /* Layer configuration */
//  //FIXME certainly need ot adapt according to camra resolution
//  
////  LTDC_Layer1->CR |= 
//    LTDC_Layer1->WHPCR |= ((522 + ((LTDC->BPCR & LTDC_BPCR_AHBP) >>16) + 1) <<16) |
//                          (43 + ((LTDC->BPCR & LTDC_BPCR_AHBP) >>16)+1)           ;
//    LTDC_Layer1->WVPCR |= ((283 + (LTDC->BPCR & LTDC_BPCR_AVBP)  + 1) <<16) |
//                          (12 + (LTDC->BPCR & LTDC_BPCR_AVBP) +1)           ;
//
//    LTDC_Layer1->PFCR  = 2; // RGB565 //TODO define all relevant values in a header lcd.h in includes/actuators
//                          //((RK043FN48H_HSYNC + RK043FN48H_HBP) +
////    LTDC_Layer1->WVPCR |= (RK043FN48H_HSYNC + RK043FN48H_VBP) +
//    LTDC_Layer1->CACR = 0xFF; // opaque layer
//    
//    LTDC_Layer1->DCCR = 0x0; //FIXME same as line 40 just for fun
//    LTDC_Layer1->BFCR |= 0x405;
//    LTDC_Layer1->CFBLR  = (((480 * 2/*2bitsperpixel*/) << 16) | (((522 - 43) * 2)  + 3));
//
//  /* Configures the frame buffer line number */
//    LTDC_Layer1->CFBLNR  = 240;
//    // Dont care about blending for now
//    //TODO configure buffer address and length 0x02000203 + line number 
//    LTDC_Layer1->CFBAR =(uint32_t)&RGB565_480x272;
//    LTDC->SRCR |=LTDC_SRCR_IMR;
//
//  // Enable interrupts
//  LTDC->IER |= //   LTDC_IER_LIE                      |
//                  LTDC_IER_FUIE                     |
//                  LTDC_IER_TERRIE                   ;
////                  LTDC_IER_RRIE                     ;
//  printf("LTDC ISR: 0x%8X\r\n", LTDC->ISR);
//  //Finally enable controller
//  LTDC->GCR |= LTDC_GCR_LTDCEN;
//  //Finally Enable Layer
//  LTDC_Layer1->CR |= LTDC_LxCR_LEN;
//
//  printf("LTDC ISR: 0x%8X\r\n", LTDC->ISR);
//  NVIC_SetPriority(LTDC_IRQn,5);
//  NVIC_EnableIRQ(LTDC_IRQn);
//  printf("LTDC ISR: 0x%8X\r\n", LTDC->ISR);
//  NVIC_SetPriority(LTDC_ER_IRQn,5);
//  NVIC_EnableIRQ(LTDC_ER_IRQn);

// Enable display
#define PORTI_LCD_DISP 12
#define PORTK_LCD_BL_CONTROL 3  // ??
  pin_set_output_high(GPIOI,PORTI_LCD_DISP);
  pin_set_output_high(GPIOK, PORTK_LCD_BL_CONTROL);
  printf("end of LCD init\r\n");

}

void lcd_tft_vector(){
  printf("ltdc interrupt %X",LTDC->ISR);
  if((LTDC->ISR & LTDC_ISR_LIF) == LTDC_ISR_LIF){
//    printf("%8X,",LTDC_Layer1->CLUTWR);
    LTDC->ICR |= LTDC_ICR_CLIF;
  }
  if((LTDC->ISR & LTDC_ISR_RRIF) == LTDC_ISR_RRIF){
    printf("register reload ");
  }
  //  printf("coucou");
  
}
void lcd_tft_error_vector(){
  printf("error LTDC !\r\n");
}
  /* ENABLE OVERDRIVE MODE*/
  //1) Select HSE as sys clock (in RCC_CR) => HSEON, HSERDY. RCC_CFGR-> SW = 01 (select HSE) then wait for sys clock to be ready (rcc_cr->sws)
  //2) config RCC_PLLCFCG and set PLLON in RCC_CR
  //3) Set ODEN in PWR_CR1 and wait for ODRDY in PWR_CSR1
  //4) set ODSW in PWR_CR1 and wait for ODSWRDY in PWR_CSR1
  //5) select flash latecny ?? and prescalers
  //6) wait for pll lock in RCC_CR
  //7) switch system to pll                               RCC->CR SW = 10 for pll then wait for SWS to return 10
  //8) enable all peripherals

//  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
//  //1
//  led_off();
//  RCC->CR |= RCC_CR_HSEON;
//  printf("turn on HSE\r\n");
//  while((RCC->CR & RCC_CR_HSERDY) != RCC_CR_HSERDY);
//  RCC->CFGR &= ~RCC_CFGR_SW;
//  RCC->CFGR |= RCC_CFGR_SW_0;
//  printf("waiting for HSE selection\r\n");
//  while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_0);
//  printf("HSE clk selected\r\n");
//  //2
//  uint32_t temp = 0;
//  temp = (9 << 24) | (1 << 22) | (2 << 16) | (432 << 6 ) | 25;
//  RCC->PLLCFGR = temp;
//  RCC->CR |= RCC_CR_PLLON;
//  //3
//  PWR->CR1 |= PWR_CR1_ODEN;
//  while((PWR->CSR1 & PWR_CSR1_ODRDY)!= PWR_CSR1_ODRDY);
//  //4
//  PWR->CR1 |= PWR_CR1_ODSWEN;
//  while((PWR->CSR1 & PWR_CSR1_ODSWRDY)!= PWR_CSR1_ODSWRDY);
//  //5
//  //6
//  while((RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY);
//  //7
//  RCC->CFGR = ((RCC->CFGR & ~RCC_CFGR_SW) | 0x2);
//  while((RCC->CFGR & RCC_CFGR_SWS) | RCC_CFGR_SWS_PLL);
//  led_on();