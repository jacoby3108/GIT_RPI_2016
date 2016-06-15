#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// bitbangspi v4.00 by Dany (User space version) 29-mayo-2016

//****************Select Debug  on/off *****************************************************
#define ON   1
#define OFF  0

#define DEBUG  OFF     

//*****************Select Hardware Type: Serial or parallel*********************************
#define PARA		0
#define SERIAL		1

#define HARDWARE 	PARA
//*****************************************************************************************


#if DEBUG == ON
# define DEBUG_PRINT(x) fprintf x
#elif DEBUG == OFF
# define DEBUG_PRINT(x) do {} while (0)
#endif

#define TEXTIFY(A) #A
#define GPIO(pin)  TEXTIFY(/sys/class/gpio/gpio ## pin )

typedef struct 
		{
			const unsigned char * name; 
		    const unsigned int  numpin;
		    const unsigned char * txtpin;
		    const unsigned char * direction;
			int handle;
			
		} PIN;
			
			
#if HARDWARE == SERIAL


PIN pins[] ={{"LOAD",17,"17","out",0},
			 {"SCK",22,"22","out",0},
			 {"DATA_IN",23,"23","out",0}};

enum pin_index {PIN_LOAD,
				PIN_SCK,
				PIN_DATA_IN};

#elif HARDWARE == PARA
			
PIN pins[] ={
				{"D0",4,"4","out",0},
				{"D1",17,"17","out",0},
				{"D2",27,"27","out",0},
				{"D3",22,"22","out",0},
				{"D4",18,"18","out",0},
				{"D5",23,"23","out",0},
				{"D6",24,"24","out",0},
				{"D7",25,"25","out",0}};
				

				
enum pin_index {PIN_D0,
				PIN_D1,
				PIN_D2,
				PIN_D3,
				PIN_D4,
				PIN_D5,
				PIN_D6,
				PIN_D7};

#endif




// Pin file system

#define EXPORT "/sys/class/gpio/export"      // Export file 
 


// CLI Usermode examples (Keep this for reference)

//echo "17" > /sys/class/gpio/export
//echo "17" > /sys/class/gpio/unexport
//echo "out" > /sys/class/gpio/gpio17/direction  (NOTE "out" must be in lower case)
//cat /sys/class/gpio/gpio9/value
//echo "1" > /sys/class/gpio/gpio22/value


void Export_Pins(void);
void Set_PIN_Direction(void);
void Set_Port_handles(void);
void Set_Pin(int pin);
void Clr_Pin(int pin);
unsigned char spiWrite(const unsigned char regData);
void initspi(void);


int handle_export;
int nWritten,nread;  // bytes read or written during file operations
unsigned char contador;
int i,j,k;



void initspi(void)
{
	Export_Pins();					// Export Used pins to User space for debug
	Set_PIN_Direction();			// Set Pin I/O 
	Set_Port_handles();				// Anotate pin handles for I/O operations
}



#if HARDWARE == SERIAL

unsigned char spiWrite(const unsigned char regData)
{

  unsigned char SPICount;                               // Counter used to clock out the data
  unsigned char SPIData;                                // Define a data structure for the SPI data.

  Clr_Pin(PIN_SCK);                                     // Load and CK low (steady state)
  Clr_Pin(PIN_LOAD); 
  
  SPIData = regData;                                    // Preload the data to be sent 


  

  for (SPICount = 0; SPICount < 8; SPICount++)          // Prepare to clock out Data
  {
    if (SPIData & 0x80)									// Unmask each bit
      Set_Pin(PIN_DATA_IN);	
    else
      Clr_Pin(PIN_DATA_IN);
      
  
    Clr_Pin(PIN_SCK); 									// Shift out each bit
    Set_Pin(PIN_SCK);
     
    SPIData <<= 1;										// MSB first
       
  }                                                       
 
   Set_Pin(PIN_LOAD);

return(0);
}

#elif HARDWARE == PARA


unsigned char spiWrite(const unsigned char regData)
{

  unsigned char SPICount;                               // Counter used to clock out the data
  unsigned char SPIData;                                // Define a data structure for the SPI data.

 
  SPIData = regData;                                    // Preload the data to be sent 
  

  for (SPICount = 0; SPICount < 8; SPICount++)          // Prepare to clock out Data
  {
    if (SPIData & 0x01)									// Unmask each bit
      Set_Pin(SPICount);	
    else
      Clr_Pin(SPICount);
        
    SPIData >>= 1;										// LSB first
    
      
  }                                                       
 
  
return(0);
}


#endif




void Set_Pin(int pin)
{
	
	if(write(pins[pin].handle,"1",1)==-1) // int  write(  int  handle,  void  *buffer,  int  nbyte  );
	{
		DEBUG_PRINT((stdout, "Set_Pin: Cannot write to file %s (handle=%X). Try again later.\n",pins[pin].txtpin,pins[pin].handle));
		exit(1);
	}
	else
		DEBUG_PRINT((stdout, "Set_Pin write to file %s (handle=%X) successfully done.\n",pins[pin].txtpin,pins[pin].handle));
	
	
}

void Clr_Pin(int pin)
{


	if(write(pins[pin].handle,"0",1)==-1) // int  write(  int  handle,  void  *buffer,  int  nbyte  );
	{
		DEBUG_PRINT((stdout, "Clr_Pin: Cannot write to file %s (handle=%X). Try again later.\n",pins[pin].txtpin,pins[pin].handle));
		exit(1);
	}
	else
		DEBUG_PRINT((stdout, "Clr_Pin write to file %s (handle=%X) successfully done.\n",pins[pin].txtpin,pins[pin].handle));
} 

void Export_Pins(void)
{

// Open export file 


	if ((handle_export = open(EXPORT, O_WRONLY,
    	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
	{
    		DEBUG_PRINT((stdout, "Cannot open EXPORT File. Try again later.\n"));
    		exit(1);
	}

// And export pins
	
	for (i=0 ; i<sizeof(pins)/sizeof(PIN); i++)
	{ 
	// int  write(  int  handle,  void  *buffer,  int  nbyte  );
		nWritten=write(handle_export,pins[i].txtpin,strlen(pins[i].txtpin)); 
	
		if (nWritten==-1){
			DEBUG_PRINT((stdout, "Cannot open EXPORT File for PIN %s(%s). Try again later.\n",pins[i].name,pins[i].txtpin));
			//	exit(1);
		}	
		else
			DEBUG_PRINT((stdout, "EXPORT File for PIN %s(%s) opened succesfully \n",pins[i].name,pins[i].txtpin));
		
		
	}
	
}
	
	
void Set_PIN_Direction(void)
{
	char file[50]; 
	const char *gpio_str="/sys/class/gpio/gpio";
	const char *gpio_dir_str="/direction";
	int handle_direction;
	
	
	
	
	for (i=0 ; i<sizeof(pins)/sizeof(PIN); i++)
	{ 
		// Open direction file 
	 
		strcpy(file,gpio_str);
		strcat(file,pins[i].txtpin);
		strcat(file,gpio_dir_str);
		
		
		if ((handle_direction = open(file, O_WRONLY,
    		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
		{
    		DEBUG_PRINT((stdout, "Cannot open DIRECTION File. Try again later.\n"));
    		
    		exit(1);
		}

		// Set pins Directions
		if ((nWritten=write(handle_direction,pins[i].direction,strlen(pins[i].direction)))==-1)
		{	
			
			DEBUG_PRINT((stdout, "Cannot open DIRECTION File for PIN %s(%s) as %s . Try again later.\n",pins[i].name,pins[i].txtpin,pins[i].direction));
			//exit(1);
		}
		else
		{	
		
			DEBUG_PRINT((stdout, "DIRECTION File for PIN %s(%s) opened succesfully \n",pins[i].name,pins[i].txtpin));
		    		
		}
	}
	


	
}


// Open device port 
void Set_Port_handles(void)
{
	
	
	char device[50]; 
	const char *gpio_str="/sys/class/gpio/gpio";
	const char *gpio_val_str="/value";
	int handle;
	
	// int  open(  char  *filename,  int  access,  int  permission  );
	
	for (i=0 ; i<sizeof(pins)/sizeof(PIN); i++)
	{ 
		int mode;
		strcpy(device,gpio_str);
		strcat(device,pins[i].txtpin);
		strcat(device,gpio_val_str);
		
		
		
		if(strcmp(pins[i].direction,"out")==0)
			mode=O_WRONLY;
		else
			mode=O_RDONLY;
				
				
		if ((handle = open(device,mode,  
    		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
		{
    			DEBUG_PRINT((stdout, "Cannot open %s. Try again later.\n", device ));
    			exit(1);
		}
		else
		{
				pins[i].handle=handle;
				DEBUG_PRINT((stdout, "Pin port %s successfully opened (handle=%X) \n", device,handle ));
		}
		
	}
	
}
