#include "usb.h"
#include "flash.h"

const uint8_t deviceDescriptor[] =
{
    0x12,        // bLength
	0x01,        // bDescriptorType (Device)
	0x10, 0x01,  // bcdUSB 1.10
	0x00,        // bDeviceClass (Use class information in the Interface Descriptors)
	0x00,        // bDeviceSubClass
	0x00,        // bDeviceProtocol
	EP0_SZ,      // bMaxPacketSize0 8
	0x86, 0x19,  // idVendor 0x1986
	0x86, 0x19,  // idProduct 0x1986
	0x01, 0x00,  // bcdDevice 0.01
	0x01,        // iManufacturer (String Index)
	0x02,        // iProduct (String Index)
	0x03,        // iSerialNumber (String Index)
	0x01         // bNumConfigurations 1
};

const uint8_t HID_DeviceReportDescriptor[] =
{
	0x06, 0x00, 0xff,              // USAGE_PAGE (Vendor Defined Page 1)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x85, 0x01,                    //   REPORT_ID (1)
    0x95, 0x47,                    //   REPORT_COUNT (72)
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
    0xc0                           // END_COLLECTION
};

const uint8_t configDescriptor[] = {
	0x09,        // bLength
	0x02,        // bDescriptorType (Configuration)
	0x22, 0x00,  // wTotalLength 34
	0x01,        // bNumInterfaces 1
	0x01,        // bConfigurationValue
	0x00,        // iConfiguration (String Index)
	0xC0,        // bmAttributes Self Powered
	0x32,        // bMaxPower 100mA

	0x09,        // bLength
	0x04,        // bDescriptorType (Interface)
	0x00,        // bInterfaceNumber 0
	0x00,        // bAlternateSetting
	0x01,        // bNumEndpoints 1
	0x03,        // bInterfaceClass
	0x00,        // bInterfaceSubClass
	0x00,        // bInterfaceProtocol
	0x00,        // iInterface (String Index)

	0x09,        // bLength
	0x21,        // bDescriptorType (HID)
	0x11, 0x01,  // bcdHID 1.11
	0x00,        // bCountryCode
	0x01,        // bNumDescriptors
	0x22,        // bDescriptorType[0] (HID)
	/*0x20*/0x18, 0x00,  // wDescriptorLength[0] 32

	0x07,        // bLength
	0x05,        // bDescriptorType (Endpoint)
	0x81,        // bEndpointAddress (IN/D2H)
	0x03,        // bmAttributes (Interrupt)
	0x08, 0x00,  // wMaxPacketSize 8
	0x05         // bInterval 5 (unit depends on device speed)
};

const uint8_t stringDescriptor[] = {
	0x04,	// bLength
	0x03,	// bDescriptorType
	0x09,	// wLANGID[0] (low byte)
	0x04	// wLANGID[0] (high byte)
};

const uint8_t manufacturerDescriptor[] = {
	46,
    0x03,
    'H', 0, 'u', 0, 'n', 0, 'g', 0, ' ', 0, 'C', 0, 'u', 0, 'o', 0,
    'n', 0, 'g', 0, ' ', 0, 'E', 0, 'l', 0, 'e', 0, 'c', 0, 't', 0,
    'r', 0, 'o', 0, 'n', 0, 'i', 0, 'c', 0, 's', 0
};

const uint8_t productDescriptor[] = {
	0x26, // Size,
	0x03, // Descriptor type
	'U', 0, 'S', 0, 'B', 0, ' ', 0, 'H', 0, 'I', 0,
	'D', 0, ' ', 0, 'B', 0, 'O', 0, 'O', 0, 'T', 0,
	'L', 0, 'O', 0, 'A', 0, 'D', 0, 'E', 0, 'R', 0
};

const uint8_t serialDescriptor[] = {
	0x28, // Size,
	0x03, // Descriptor type
	'w', 0, 'w', 0, 'w', 0, '.', 0, 'v', 0,
	'i', 0, 'd', 0, 'i', 0, 'e', 0, 'u', 0, 'k', 0, 'h', 0, 'i', 0,
	'e', 0, 'n', 0, '.', 0, 'o', 0, 'r', 0, 'g', 0
};

uint8_t u8SetupPacket[8];
uint8_t u8Ep0Buff[72];
uint8_t u8TransBuff[72];

uint8_t *pu8Buff;
uint8_t u8Total;
uint8_t u8HidReq = 0;
uint8_t u8HidNum = 0;

uint8_t u8Addr = 0;
uint8_t u8Config = 0;

#define SETUP_STATE 0
#define DATA_STATE 1
#define STATUS_STATE 2

uint8_t u8ControlState = SETUP_STATE;

static void EP0_Setup(void);
static void EP0_Out(void);
static void EP0_In(void);
static void StandardRequest(void);
static void ClassRequest(void);
static void VendorRequest(void);

static void GetStatus(void);
static void ClearFeature(void);
static void SetFeature(void);
static void SetAddress(void);
static void GetDescriptor(void);
static void SetDescriptor(void);
static void GetConfiguration(void);
static void SetConfiguration(void);
static void GetInterface(void);
static void SetInterface(void);
static void SynchFrame(void);

static void HID_SetIdle(void);
static void HID_SetReport(void);
static void HID_GetReport(void);
void EP1_Out(void);
void EP1_In(void);

void USB_Init(void)
{
	GPIO_InitTypeDef gpioInit;
	
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
	//cho phep usb hoat dong
	_SetCNTR(0x00);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Pin = GPIO_Pin_7;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB, &gpioInit);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_7);
}

void USB_Task(void)
{
	uint32_t u32ISTR;
	uint32_t u32Tmp;
	
	// Handle Reset
	u32ISTR = _GetISTR();
	if (u32ISTR & ISTR_RESET) {
		_SetDADDR(0x80);
		u8ControlState = SETUP_STATE;
		u8Addr = 0;
		u8Config = 0;
		SetEPAddress(ENDP0, 0);
		SetEPType(ENDP0, EP_CONTROL);
		SetEPTxStatus(ENDP0, EP_TX_STALL);
		SetEPRxAddr(ENDP0, 0x40);
		SetEPTxAddr(ENDP0, 0x80);
		Clear_Status_Out(ENDP0);
		SetEPRxCount(ENDP0, EP0_SZ);
		SetEPRxValid(ENDP0);
		SetISTR(u32ISTR & CLR_RESET);
		//return;
	}
	
	// Handle EP data
	u32ISTR = _GetISTR();
	if (u32ISTR & ISTR_CTR) {
		if ((u32ISTR & ISTR_EP_ID) == 0x00) {
			//EP0
			if (u32ISTR & ISTR_DIR) {
				//OUT SETUP
				u32Tmp = _GetENDPOINT(0);
				if (u32Tmp & EP_SETUP) {
					//SETUP
					EP0_Setup();
				} else {
					//OUT
					EP0_Out();
				}
				ClearEP_CTR_RX(0);
			} else {
				//IN
				EP0_In();
				ClearEP_CTR_TX(0);
			}
		} else if ((u32ISTR & ISTR_EP_ID) == 0x01) {
			//EP1
			if (u32ISTR & ISTR_DIR) {
				//OUT
				//EP1_Out();
				ClearEP_CTR_RX(1);
			} else {
				//IN
				//EP1_In();
				ClearEP_CTR_TX(1);
			}
		}
		//SetISTR(u32ISTR & CLR_CTR);
		//return;
	}
	
	// Handle DOVR
	u32ISTR = _GetISTR();
	if (u32ISTR & ISTR_DOVR) {
		SetISTR(u32ISTR & CLR_DOVR);
		//return;
	}
	
	// Handle Suspend
	u32ISTR = _GetISTR();
	if (u32ISTR & ISTR_SUSP) {
		SetISTR(u32ISTR & CLR_SUSP);

		// If device address is assigned, then reset it
		/*
		if (_GetDADDR() & 0x007f) {
			_SetDADDR(0);
			_SetCNTR(_GetCNTR() & ~CNTR_SUSPM);
		}
		*/
		//return;
	}
	
	// Handle Error
	u32ISTR = _GetISTR();
	if (u32ISTR & ISTR_ERR) {
		SetISTR(u32ISTR & CLR_ERR);
		//return;
	}
	
	// Handle Wakeup
	u32ISTR = _GetISTR();
	if (u32ISTR & ISTR_WKUP) {
		SetISTR(u32ISTR & CLR_WKUP);
		//return;
	}
	
	// Handle SOF
	u32ISTR = _GetISTR();
	if (u32ISTR & ISTR_SOF) {
		SetISTR(u32ISTR & CLR_SOF);
		//return;
	}
	
	// Handle ESOF
	u32ISTR = _GetISTR();
	if (u32ISTR & ISTR_ESOF) {
		SetISTR(u32ISTR & CLR_ESOF);
		//return;
	}
	
	//_SetISTR(0);
}

static void EP0_Setup(void) {
	//kiem tra xem host yeu cau gi
	uint8_t u8RequestType;
	
	//nhan ve du lieu setup packet
	PMAToUserBufferCopy((uint8_t *)u8SetupPacket, GetEPRxAddr(0), 8);
	u8RequestType = (u8SetupPacket[0] & 0x60) >> 5;
	switch(u8RequestType) {
	case 0x00:
		//standard request
		StandardRequest();
		break;
	case 0x01:
		//class request
		ClassRequest();
		break;
	case 0x02:
		//vendor request
		VendorRequest();
		break;
	default:
		//stall EP0 IN
		//===================================
		SetEPTxStatus(ENDP0, EP_TX_STALL);
		SetEPRxCount(ENDP0, EP0_SZ);
		SetEPRxValid(ENDP0);
		break;
	}
}

uint8_t u8Success = 1;

static void EP0_Out(void) {
	uint32_t i;
	uint32_t n;
	GPIO_InitTypeDef gpioInit;
	
	if (u8ControlState == DATA_STATE) {
		if (u8Total <= EP0_SZ) {
			//sao du lieu nhan duoc vao vung nho
			PMAToUserBufferCopy((uint8_t *)pu8Buff, GetEPRxAddr(0), u8Total);
			
			//kiem tra hid request
			//thu dieu khien led tu HOST
			if (u8HidReq) {				
				u8HidReq = 0;
				//GPIO_SetBits(GPIOA, GPIO_Pin_1);
				//GPIO_ResetBits(GPIOA, GPIO_Pin_1);
				//xu ly yeu cau HID o day
				switch (u8Ep0Buff[1]) {
					case 0x00:
						/* Get info */
						n = sizeof(DEV_INFO);
						for (i = 0; i < n; ++i) {
							u8TransBuff[i] = DEV_INFO[i];
						}
						u8TransBuff[i] = 0;
						n = APP_BASE_ADDRESS;
						u8TransBuff[32] = (uint8_t)(n >> 0);
						u8TransBuff[33] = (uint8_t)(n >> 8);
						u8TransBuff[34] = (uint8_t)(n >> 16);
						u8TransBuff[35] = (uint8_t)(n >> 24);
						
						n = APP_SIZE;
						u8TransBuff[36] = (uint8_t)(n >> 0);
						u8TransBuff[37] = (uint8_t)(n >> 8);
						u8TransBuff[38] = (uint8_t)(n >> 16);
						u8TransBuff[39] = (uint8_t)(n >> 24);
						
						n = FLASH_ERASE_PAGE_SIZE;
						u8TransBuff[40] = (uint8_t)(n >> 0);
						u8TransBuff[41] = (uint8_t)(n >> 8);
						u8TransBuff[42] = (uint8_t)(n >> 16);
						u8TransBuff[43] = (uint8_t)(n >> 24);
						
						n = FLASH_WRITE_PAGE_SIZE;
						u8TransBuff[44] = (uint8_t)(n >> 0);
						u8TransBuff[45] = (uint8_t)(n >> 8);
						u8TransBuff[46] = (uint8_t)(n >> 16);
						u8TransBuff[47] = (uint8_t)(n >> 24);
						
						n = sizeof(BOOT_VER);
						for (i = 0; i < n; ++i) {
							u8TransBuff[48 + i] = BOOT_VER[i];
						}
						u8TransBuff[48 + i] = 0;
						u8HidNum = 72;
						u8Success = 1;
						break;
					case 0x01:
						/* Erase */
						//tinh dia chi
						n = u8Ep0Buff[7];
						n <<= 8;
						n += u8Ep0Buff[6];
						n <<= 8;
						n += u8Ep0Buff[5];
						n <<= 8;
						n += u8Ep0Buff[4];
						
						if ((n >= APP_BASE_ADDRESS) && (n < (APP_BASE_ADDRESS + APP_SIZE))) {
							FlashUnlock();
							if (FormatFlashPage(n)) {
								u8Success = 1;
							} else {
								u8Success = 0;
							}
							FlashLock();
						} else {
							u8Success = 0;
						}
						break;
					case 0x02:
						/* write */
						//tinh dia chi
						n = u8Ep0Buff[7];
						n <<= 8;
						n += u8Ep0Buff[6];
						n <<= 8;
						n += u8Ep0Buff[5];
						n <<= 8;
						n += u8Ep0Buff[4];
						
						if ((n >= APP_BASE_ADDRESS) && (n < (APP_BASE_ADDRESS + APP_SIZE))) {
							FlashUnlock();							
							if (WriteFlash(n, &u8Ep0Buff[8], 64)) {
								u8Success = 1;
							} else {
								u8Success = 0;
							}
							FlashLock();
						} else {
							u8Success = 0;
						}
						break;
					case 0x03:
						//_SetBCDR(~BCDR_DPPU);
						gpioInit.GPIO_Mode = GPIO_Mode_IN_FLOATING;
						gpioInit.GPIO_Pin = GPIO_Pin_7;
						gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	
						GPIO_Init(GPIOB, &gpioInit);
						//around 13ms
						for (i = 0; i < 0x2ffff; ++i) {
						}
						NVIC_SystemReset();
						break;
				}
			}
			if (u8Success) {
				u8ControlState = STATUS_STATE;
				SetEPTxCount(ENDP0, 0);
				SetEPTxValid(ENDP0);
			} else {
				u8ControlState = SETUP_STATE;
				SetEPTxCount(ENDP0, 0);
				SetEPTxStatus(ENDP0, EP_TX_STALL);
				SetEPRxCount(ENDP0, EP0_SZ);
				SetEPRxValid(ENDP0);
			}
		} else {
			PMAToUserBufferCopy((uint8_t *)pu8Buff, GetEPRxAddr(0), EP0_SZ);
			pu8Buff += EP0_SZ;
			u8Total -= EP0_SZ;
			if (u8Total <= EP0_SZ) {
				SetEPRxCount(0, u8Total);
			} else {
				SetEPRxCount(0, EP0_SZ);
			}
			SetEPRxValid(0);
		}
	} else if (u8ControlState == STATUS_STATE) {
		u8ControlState = SETUP_STATE;
	}
}

static void EP0_In(void) {
	
	if (u8ControlState == DATA_STATE) {
		if (u8Total <= EP0_SZ) {
			u8Total = 0;
			u8ControlState = STATUS_STATE;
			SetEPRxCount(0, EP0_SZ);
			SetEPRxValid(0);
			SetEPTxCount(0, 0);
			SetEPTxValid(0);
			//Led_Off();
		} else {
			pu8Buff += EP0_SZ;
			u8Total -= EP0_SZ;
			if (u8Total <= EP0_SZ) {
				SetEPTxCount(0, u8Total);
				UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), u8Total);
			} else {
				SetEPTxCount(0, EP0_SZ);
				UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), EP0_SZ);
			}
			SetEPTxValid(0);
		}
	} else if (u8ControlState == STATUS_STATE) {
		if (u8Addr) {
			SetDADDR(u8Addr | DADDR_EF);
			u8Addr = 0;
		}
		u8ControlState = SETUP_STATE;
		SetEPRxCount(0, EP0_SZ);
		SetEPRxValid(0);
	}
}

static void StandardRequest(void) {
	
	//xu ly 11 loai request
	switch(u8SetupPacket[1]) {
	case 0x00:
		//Get Status
		GetStatus();
		break;
	case 0x01:
		//Clear Feature
		ClearFeature();
		break;
	case 0x03:
		//Set Feature
		SetFeature();
		break;
	case 0x05:
		//Set Address
		SetAddress();
		break;
	case 0x06:
		//Get Descriptor
		GetDescriptor();
		break;
	case 0x07:
		//Set Descriptor
		SetDescriptor();
		break;
	case 0x08:
		//Get Configuration
		GetConfiguration();
		break;
	case 0x09:
		//Set Configuration
		SetConfiguration();
		break;
	case 0x0A:
		//Get Interface
		GetInterface();
		break;
	case 0x0B:
		//Set Interface
		SetInterface();
		break;
	case 0x0C:
		//Synch Frame
		SynchFrame();
		break;
	}
}

static void ClassRequest(void) {
	
	switch(u8SetupPacket[1]) {
	case 0x0A:
		HID_SetIdle();
		break;
	case 0x09:
		HID_SetReport();
		break;
	case 0x01:
		HID_GetReport();
		break;
	}
}

static void VendorRequest(void) {
	
}

static void GetStatus(void) {
	uint8_t u8R;
	
	u8R=u8SetupPacket[0] & 0x1F;
	
	switch(u8R) {
	case 0x00:
	case 0x01:
	case 0x02:
		//Device
		u8Ep0Buff[0] = 0x00;
		u8Ep0Buff[1] = 0x00;
		UserToPMABufferCopy(u8Ep0Buff, GetEPTxAddr(0), 2);
		SetEPTxCount(0, 2);
		SetEPTxValid(0);
		u8ControlState = DATA_STATE;
		break;
#if 0	
	case 0x01:
		//Interface
		u8Ep0Buff[0] = 0x00;
		u8Ep0Buff[1] = 0x00;
		UserToPMABufferCopy(u8Ep0Buff, GetEPTxAddr(0), 2);
		SetEPTxCount(0, 2);
		SetEPTxValid(0);
		u8ControlState = DATA_STATE;
		break;
	case 0x02:
		//EndPoint
		u8Ep0Buff[0] = 0x00;
		u8Ep0Buff[1] = 0x00;
		UserToPMABufferCopy(u8Ep0Buff, GetEPTxAddr(0), 2);
		SetEPTxCount(0, 2);
		SetEPTxValid(0);
		u8ControlState = DATA_STATE;
		break;
#endif
	default:
		//stall EP0 IN
		//===================================
		SetEPRxCount(0, EP0_SZ);
		SetEPRxValid(0);
		u8ControlState = SETUP_STATE;
		SetEPTxStatus(0, EP_TX_STALL);
		break;
	}
}
static void ClearFeature(void) {
	uint8_t u8R;
	
	u8R=u8SetupPacket[0] & 0x1F;
	
	switch(u8R) {
	case 0x00:
	case 0x02:
		//device
		SetEPTxStatus(0, EP_TX_STALL);
		u8ControlState = STATUS_STATE;
		break;
	//case 0x01:
		//interface
		//break;
#if 0
	case 0x02:
		SetEPTxCount(0, 0);
		SetEPTxValid(0);
		u8ControlState = STATUS_STATE;
		break;
#endif
	default:
		//stall EP0 IN
		//===================================
		SetEPRxCount(0, EP0_SZ);
		SetEPRxValid(0);
	
		SetEPTxStatus(0, EP_TX_STALL);
		u8ControlState = SETUP_STATE;
		break;
	}
}
static void SetFeature(void) {
	uint8_t u8R;
	
	u8R=u8Ep0Buff[0] & 0x1F;
	
	switch(u8R) {
	case 0x00:
	case 0x02:
		SetEPTxCount(0, 0);
		SetEPTxValid(0);
		u8ControlState = STATUS_STATE;
		break;
	//case 0x01:
		//interface
		//break;
#if 0
	case 0x02:
		//endpoint
		SetEPTxCount(0, 0);
		SetEPTxValid(0);
		u8ControlState = STATUS_STATE;
		break;
#endif
	default:
		//stall EP0 IN
		//===================================
		SetEPRxCount(0, EP0_SZ);
		SetEPRxValid(0);
	
		SetEPTxStatus(0, EP_TX_STALL);
		u8ControlState = SETUP_STATE;
		break;
	}
}

static void SetAddress(void) {	
	//chi luu lai dia chi, chua thay doi dia chi ngay
	u8Addr = u8SetupPacket[2];
	
	SetEPTxCount(0, 0);
	SetEPTxValid(0);
	u8ControlState = STATUS_STATE;
}

static void GetDescriptor(void) {
	uint16_t u16Len;
	
	u16Len = u8SetupPacket[7];
	u16Len <<= 8;
	u16Len += u8SetupPacket[6];
	
	switch(u8SetupPacket[3]) {
	case 0x01:
		//DEVICE DESCRIPTOR
		if(u16Len >= 0x12) {
			u16Len = 0x12;
		}
		if (u16Len <= EP0_SZ) {
			pu8Buff = (uint8_t *)deviceDescriptor;
			UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), u16Len);
			SetEPTxCount(0, u16Len);
		} else {
			pu8Buff = (uint8_t *)deviceDescriptor;
			UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), EP0_SZ);
			SetEPTxCount(0, EP0_SZ);
		}
		SetEPRxCount(0, EP0_SZ);
		SetEPRxValid(0);
		u8Total = u16Len;
		u8ControlState = DATA_STATE;
		SetEPTxValid(0);
		break;
	case 0x02:
		//CONFIGURATION DESCRIPTOR
		if(u16Len >= 34) {
			u16Len = 34;
		}
		if (u16Len <= EP0_SZ) {
			pu8Buff = (uint8_t *)configDescriptor;
			UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), u16Len);
			SetEPTxCount(0, u16Len);
		} else {
			pu8Buff = (uint8_t *)configDescriptor;
			UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), EP0_SZ);
			SetEPTxCount(0, EP0_SZ);
		}
		SetEPRxCount(0, EP0_SZ);
		SetEPRxValid(0);
		u8Total = u16Len;
		u8ControlState = DATA_STATE;
		SetEPTxValid(0);
		break;
	case 0x03:
		//STRING DESCRIPTOR
		switch(u8SetupPacket[2]) {
		case 0x00:
			//STRING LANG
			if(u16Len >= 0x04) {
				u16Len = 0x04;
			}
			if (u16Len <= EP0_SZ) {
				pu8Buff = (uint8_t *)stringDescriptor;
				UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), u16Len);
				SetEPTxCount(0, u16Len);
			} else {
				pu8Buff = (uint8_t *)stringDescriptor;
				UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), EP0_SZ);
				SetEPTxCount(0, EP0_SZ);
			}
			SetEPRxCount(0, EP0_SZ);
			SetEPRxValid(0);
			u8Total = u16Len;
			u8ControlState = DATA_STATE;
			SetEPTxValid(0);
			break;
		case 0x01:
			//STRING MAN
			if(u16Len >= 46) {
				u16Len = 46;
			}
			if (u16Len <= EP0_SZ) {
				pu8Buff = (uint8_t *)manufacturerDescriptor;
				UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), u16Len);
				SetEPTxCount(0, u16Len);
			} else {
				pu8Buff = (uint8_t *)manufacturerDescriptor;
				UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), EP0_SZ);
				SetEPTxCount(0, EP0_SZ);
			}
			SetEPRxCount(0, EP0_SZ);
			SetEPRxValid(0);
			u8Total = u16Len;
			u8ControlState = DATA_STATE;
			SetEPTxValid(0);
			break;
		case 0x02:
			//STRING PRODUCT
			if(u16Len >= 38) {
				u16Len = 38;
			}
			if (u16Len <= EP0_SZ) {
				pu8Buff = (uint8_t *)productDescriptor;
				UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), u16Len);
				SetEPTxCount(0, u16Len);
			} else {
				pu8Buff = (uint8_t *)productDescriptor;
				UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), EP0_SZ);
				SetEPTxCount(0, EP0_SZ);
			}
			SetEPRxCount(0, EP0_SZ);
			SetEPRxValid(0);
			u8Total = u16Len;
			u8ControlState = DATA_STATE;
			SetEPTxValid(0);
			break;
		case 0x03:
			//STRING SERIAL
			if(u16Len >= 40) {
				u16Len = 40;
			}
			if (u16Len <= EP0_SZ) {
				pu8Buff = (uint8_t *)serialDescriptor;
				UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), u16Len);
				SetEPTxCount(0, u16Len);
			} else {
				pu8Buff = (uint8_t *)serialDescriptor;
				UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), EP0_SZ);
				SetEPTxCount(0, EP0_SZ);
			}
			SetEPRxCount(0, EP0_SZ);
			SetEPRxValid(0);
			u8Total = u16Len;
			u8ControlState = DATA_STATE;
			SetEPTxValid(0);
			break;
		default:
			SetEPRxCount(0, EP0_SZ);
			SetEPRxValid(0);
			SetEPTxStatus(0, EP_TX_STALL);
			
			u8ControlState = SETUP_STATE;
			
			break;
		}
		break;
	case 0x22:
		switch (u8SetupPacket[2]) {
		case 0x00:
			if(u16Len >= 24) {
				u16Len = 24;
			}
			if (u16Len <= EP0_SZ) {
				pu8Buff = (uint8_t *)HID_DeviceReportDescriptor;
				UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), u16Len);
				SetEPTxCount(0, u16Len);
			} else {
				pu8Buff = (uint8_t *)HID_DeviceReportDescriptor;
				UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), EP0_SZ);
				SetEPTxCount(0, EP0_SZ);
			}
			SetEPRxCount(0, EP0_SZ);
			SetEPRxValid(0);
			u8Total = u16Len;
			u8ControlState = DATA_STATE;
			SetEPTxValid(0);
			break;
		}
		break;
	default:
		SetEPRxCount(0, EP0_SZ);
		SetEPRxValid(0);
		SetEPTxStatus(0, EP_TX_STALL);
		
		u8ControlState = SETUP_STATE;
		
		break;
	}
}

static void SetDescriptor(void) {
	
	SetEPRxCount(0, EP0_SZ);
	SetEPRxValid(0);
	SetEPTxStatus(0, EP_TX_STALL);
	u8ControlState = SETUP_STATE;
}

static void GetConfiguration(void) {
	
	u8Ep0Buff[0] = u8Config;
	UserToPMABufferCopy(u8Ep0Buff, GetEPTxAddr(0), 1);
	SetEPTxCount(0, 1);
	SetEPTxValid(0);
	u8ControlState = DATA_STATE;
}

static void SetConfiguration(void) {
	
	u8Config=u8SetupPacket[2];
	SetEPAddress(ENDP1, 1);
	SetEPType(ENDP1, EP_INTERRUPT);
	SetEPTxStatus(ENDP1, EP_TX_NAK);
	SetEPRxAddr(ENDP1, 0xC0);
	SetEPTxAddr(ENDP1, 0x100);
	SetEPRxCount(1, 0x40);
	SetEPRxValid(1);
	SetEPTxCount(0, 0);
	SetEPTxValid(0);
	u8ControlState = STATUS_STATE;
}

static void GetInterface(void) {
	
	u8Ep0Buff[0] = 0x00;
	UserToPMABufferCopy(u8Ep0Buff, GetEPTxAddr(0), 1);
	SetEPTxCount(0, 1);
	SetEPTxValid(0);
	u8ControlState = DATA_STATE;
}

static void SetInterface(void) {
	
	SetEPTxCount(0, 0);
	SetEPTxValid(0);
	u8ControlState = STATUS_STATE;
}

static void SynchFrame(void) {
	
}

uint8_t rxBuff[64];
uint8_t txBuff[64];

void EP1_Out(void) {

	PMAToUserBufferCopy(rxBuff, GetEPRxAddr(1), 0x40);
	if (rxBuff[0] == 0x00) {
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	} else if (rxBuff[0] == 0x01) {
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
	} else if (rxBuff[0] == 0x02) {
		txBuff[0] = 0x14;
		txBuff[1] = 0x12;
		txBuff[2] = 0x19;
		txBuff[3] = 0x86;
		UserToPMABufferCopy(txBuff, GetEPTxAddr(1), 0x40);
		SetEPTxCount(1, 0x40);
		SetEPTxValid(1);
	}
	SetEPRxCount(1, 0x40);
	SetEPRxValid(1);
}

void EP1_In(void) {
	
#if 0
	uint8_t t;
	UEP1_T_LEN = 0x40;
	t = UEP1_CTRL;
	t &= ~(0x03);
	t |= 0x02;
	UEP1_CTRL = t;
#endif
}

static void HID_SetIdle(void) {
	
	u8ControlState = SETUP_STATE;
	SetEPTxStatus(0, EP_TX_STALL);
	SetEPRxCount(0, EP0_SZ);
	SetEPRxValid(0);
}

static void HID_SetReport(void) {
	
	u8HidReq = 1;
	u8ControlState = DATA_STATE;
	u8Total = u8SetupPacket[6];
	pu8Buff = u8Ep0Buff;
	SetEPRxCount(0, EP0_SZ);
	SetEPRxValid(0);
}

static void HID_GetReport(void) {
	
	//GPIO_SetBits(GPIOA, GPIO_Pin_1);
	//GPIO_ResetBits(GPIOA, GPIO_Pin_1);
	u8ControlState = DATA_STATE;
	pu8Buff = (uint8_t *)u8TransBuff;
	u8Total = u8HidNum;
	if (u8Total <= EP0_SZ) {
		UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), u8Total);
		SetEPTxCount(0, u8Total);
	} else {
		UserToPMABufferCopy(pu8Buff, GetEPTxAddr(0), EP0_SZ);
		SetEPTxCount(0, EP0_SZ);
	}
	SetEPTxValid(0);
}
