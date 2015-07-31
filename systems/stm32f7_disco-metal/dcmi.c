#include "dcmi.h"
// This file implements init function for dcmi and dma mo,dules to talk to a camera

void dma_init();

void dcmi_init(){
  /************************
   ** DCMI CONFIGURATION***
   ************************/
  RCC->AHB2ENR |= RCC_AHB2ENR_DCMIEN;// Enable DCMI clock
  RCC->AHB2RSTR |= RCC_AHB2RSTR_DCMIRST; // Reset DCMI
  // wait ?
  RCC->AHB2RSTR &= ~RCC_AHB2RSTR_DCMIRST; // Release Reset DCMI
  // wait ?

  DCMI->CR &= ~DCMI_CR_EDM_0;
  DCMI->CR &= ~DCMI_CR_EDM_1;
//  #ifdef DCMI_MODE_SNAP
  DCMI->CR |= DCMI_CR_CM;               // Set snapshot mode
  DCMI->CR &= ~DCMI_CR_LSM  &           // buffer all receive lines
              ~DCMI_CR_BSM  &           // buffer all receives bytes
              ~DCMI_CR_JPEG &           // receive uncompressed video
              ~DCMI_CR_CROP ;           // receive full image

// Camera config from st OV9655driver
// HSYNC active low, VSYNC active high, PIXCLK sampling en rising edge
  DCMI->CR &= ~DCMI_CR_HSPOL;           // HSYNC active low
  DCMI->CR |= DCMI_CR_VSPOL;            // VSYNC active high
  DCMI->CR |= DCMI_CR_PCKPOL;           // PIXCLK sampling en rising edge 
  

  DCMI->CR &= ~DCMI_CR_ESS;             // Hardware synchronisation 
//  //Enable interrupts
  DCMI->IER |=  DCMI_IER_FRAME_IE   ;//|     //Frame interrupt
//                DCMI_IER_OVF_IE     |     //Overflow interrupt
//                DCMI_IER_ERR_IE     |     //Error interrupt
//                DCMI_IER_VSYNC_IE   |     //VSYNC interrupt
//                DCMI_IER_LINE_IE    ;     //Line interrupt

  NVIC_SetPriority(DCMI_IRQn,6);
  NVIC_EnableIRQ(DCMI_IRQn);

  // finaly enable DCMI
  DCMI->CR |= DCMI_CR_ENABLE;
  while(!(DCMI->CR & DCMI_CR_ENABLE));

//  dma_init();
}

void dma_init(){

  /***********************
   ** DMA CONFIGURATION***
   ***********************/
//  DCMI_DR address (uint32_t)(DCMI_BASE + 0x28);
RCC->AHB1ENR  |= RCC_AHB1ENR_DMA2EN   ;  // enable DMA2 clock
RCC->AHB1RSTR |= RCC_AHB1RSTR_DMA2RST ;  // reset DMA2 controller
// wait ?
RCC->AHB1RSTR &= ~RCC_AHB1RSTR_DMA2RST ;  // Release DMA2 controller

  //configure DMA2 (Stream1, Channel 1)
  DMA2_Stream1->CR &= ~DMA_SxCR_EN      ;
  while(DMA2_Stream1->CR & DMA_SxCR_EN) ;  // be sure that DMA disabled
  DMA2->LIFCR |=       DMA_LIFCR_CTCIF1 |  // Clear all interrupts flags
                       DMA_LIFCR_CHTIF1 | 
                       DMA_LIFCR_CTEIF1 | 
                       DMA_LIFCR_CDMEIF1|
                       DMA_LIFCR_CFEIF1 ; 

  // Set addresses 
  DMA2_Stream1->PAR = (uint32_t)(DCMI_BASE + 0x28); // DCMI DR register address

  DMA2_Stream1->CR &= ~DMA_SxCR_CHSEL_1 & // Select Channel 1
                      ~DMA_SxCR_CHSEL_2 ; // Idem
  DMA2_Stream1->CR |=  DMA_SxCR_CHSEL_0 | // Idem
                       DMA_SxCR_DBM     | // Double buffer mode
                       DMA_SxCR_CIRC    | // Use a circular buffer
                       DMA_SxCR_PL_1    ; // DMA priority High
  DMA2_Stream1->CR &= ~DMA_SxCR_PL_0    & // Idem
                      ~DMA_SxCR_DIR     & // Periph to memory mode
                      ~DMA_SxCR_PINC    ; // Always read at same periph adress
  DMA2_Stream1->CR |=  DMA_SxCR_MINC    ; // Inc memory adress eachtime
  DMA2_Stream1->CR &= ~DMA_SxCR_PSIZE_0 ; // Use Word size (32-bit )
  DMA2_Stream1->CR |=  DMA_SxCR_PSIZE_1 ; // idem
  DMA2_Stream1->CR &= ~DMA_SxCR_MSIZE_0 ; // Use Word size (32-bit )
  DMA2_Stream1->CR |=  DMA_SxCR_MSIZE_1 ; // idem

  DMA2_Stream1->CR |=   DMA_SxCR_TCIE     | // Enable DMA Interrupts
                        DMA_SxCR_HTIE     | // Idem
                        DMA_SxCR_TEIE     | // Idem
                        DMA_SxCR_DMEIE    ; // Idem
  
  //Enable IRQs
  NVIC_SetPriority(DMA2_Stream1_IRQn,6);
  NVIC_EnableIRQ(DMA2_Stream1_IRQn);

  DMA2_Stream1->CR|= DMA_SxCR_EN          ; //Finally enable DMA
  while(!(DMA2_Stream1->CR & DMA_SxCR_EN));
}






/* OLD GENERAL CONFIG

  #ifndef DCMI_DATA_WIDTH
    #define DCMI_DATA_WIDTH 8
  #endif
  // Set DCMI data width
//  if(DCMI_DATA_WIDTH == 8){
    // For this specific board only 8bits are routed to the camera connector
    DCMI->CR &= ~DCMI_CR_EDM_0;
    DCMI->CR &= ~DCMI_CR_EDM_1;
//  }

  else if(DCMI_DATA_WIDTH == 10){
      DCMI->CR |=  DCMI_CR_EDM_0;
      DCMI->CR &= ~DCMI_CR_EDM_1;
  }
  else if(DCMI_DATA_WIDTH == 12){
        DCMI->CR &= ~DCMI_CR_EDM_0;
        DCMI->CR |=  DCMI_CR_EDM_1;

  }
  else if(DCMI_DATA_WIDTH == 14){
          DCMI->CR |=  DCMI_CR_EDM_0;
          DCMI->CR |=  DCMI_CR_EDM_1;
  }


////    DCMI->CR &= ~DCMI_CR_CM;              // Set Continuous mode otherwise
//      // Select framerate in continuous mode
////    DCMI->CR |= DCMI_CR_FCRC_0/1 //(divide by 1 to 4)
//


  
//  // Embedded Synchronisation otherwise
//  DCMI->CR |= DCMI_CR_ESS;
//  DCMI->ESCR // set embedded synchronization code
  
*/
