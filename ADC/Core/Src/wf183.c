
#include "wf183.h"
#include "SoftIIC.h"
#include "cmsis_os.h"
#include "ads1220.h"

uint16_t sys_adc_183[] = { 0, TYPE1_GET_ADC };
uint16_t sys_vdd_off_183[] = { 0, TYPE1_SET_VDD,0x00 };
uint16_t sys_vdd_on_183[] = { 0, TYPE1_SET_VDD,0xFF };
int ADC_Value_183 = 0;

/************WF183DE****************/
 uint16_t uart_id[] = { 0, 0, 4, 100, 8, 20, 0x55, 0x04, 0x24, 0x37 };
 uint16_t uart_version[] = { 0, 0, 4, 20, 8, 100, 0x66, 0x04, 0x1D, 0x2F };
 uint16_t uart_tk[] = { 0, 0, 4, 100, 8, 20, 0x66, 0x04, 0x14, 0xB3 };
 uint16_t uart_tb[] = { 0, 0, 4, 100, 8, 20, 0x66, 0x04, 0x11, 0x8C };
 uint16_t uart_pk[] = { 0, 0, 4, 100, 8, 20, 0x66, 0x04, 0x38, 0x33 };
 uint16_t uart_pb[] = { 0, 0, 4, 100, 8, 20, 0x66, 0x04, 0x39, 0x6D };
 uint16_t uart_calT[] = { 0, 0, 4, 100, 6, 200, 0x55, 0x04, 0x0E, 0x6A };
 uint16_t uart_calP[] = { 0, 0, 4, 100, 8, 200, 0x55, 0x04, 0x0D, 0x88 };
 uint16_t uart_sleep01[] = { 0, 0, 4, 100, 0, 0, 0x55, 0x04, 0x01, 0x2B };
 uint16_t uart_sleep02[] = { 0, 0, 4, 100, 0, 0, 0x55, 0x04, 0x02, 0xC9 };
 uint16_t uart_single_rawT[] = { 0, 0, 4, 100, 6, 100,  0x55, 0x04, 0x0E, 0x6A };
 uint16_t sys_adc[] = { 0, TYPE1_GET_ADC };
 uint16_t sys_vdd_off[] = { 0, TYPE1_SET_VDD,0x00 };
 uint16_t sys_vdd_on[] = { 0, TYPE1_SET_VDD,0xFF };
 uint16_t iic_write_calT[] = { 0, 0, 0x11, 0xDA, 0x00, 0x02,0x0A, 0x04};
 uint16_t iic_write_calT2[] = { 0, 0, 0x11, 0xDA, 0x00, 0x01, 0x0F};
 uint16_t iic_read_calT[] = { 0, 0, 0x22, 0xDA, 0x00, 0x02};
 uint16_t iic_write_calP[] = { 0, 0, 0x11, 0xDA, 0x00, 0x02, 0x0A, 0x06};
 uint16_t iic_write_calP2[] = { 0, 0, 0x11, 0xDA, 0x00, 0x01, 0x0B};
 uint16_t iic_read_calP[] = { 0, 0, 0x22, 0xDA, 0x00, 0x04};
 
 uint16_t iic_write_statusCmd[] = { 0, 0, 0x11, 0xDA, 0x00, 0x01, 0x13};
 uint16_t iic_read_status[] = { 0, 0, 0x22, 0xDA, 0x00, 0x01};
/**********************************/
 
void AllTestFor183(uint16_t *out, uint16_t out_lens){ 
 
    uint16_t _out[10]; 
    uint32_t adc2;
    uint8_t i; 
    volatile float curr;
    //oppen power
    ParseSystemCmd(sys_vdd_on_183, 2, _out, 4);
    osDelay(150);
    do {
        //short dect
        ADC_Value_183 = GET_ADS1220_val(&ADS1220);
        curr = (float)ADC_Value_183/8388608.f*3.3/120;
        if(curr>=0.02) {
            _out[0] = (uint8_t)(ADC_Value_183); 
            _out[1] = (uint8_t)(ADC_Value_183>>8);
            _out[2] = (uint8_t)(ADC_Value_183>>16);
            _out[3] = (uint8_t)(ADC_Value_183>>24);
            CopyAndClear(_out, &out[110], 4);
            break;
        }
        {            
            //sleep         ParseSerialCmd
            ParseSerialCmd(uart_sleep02, 8, _out, 10);
            //sleep current     ParseSystemCmd     
            ADC_Value_183 = GET_ADS1220_val(&ADS1220);
            adc2 = GET_ADS1220_val(&ADS1220);
            if(adc2>ADC_Value_183) ADC_Value_183 = adc2;
            out[114] = (uint8_t)(ADC_Value_183); 
            out[115] = (uint8_t)(ADC_Value_183>>8);
            out[116] = (uint8_t)(ADC_Value_183>>16);
            out[117] = (uint8_t)(ADC_Value_183>>24);
        }
        {
            //id          ParseSerialCmd
            for(i=0; i<5; i++) {
                ParseSerialCmd(uart_id, 8, _out, 10);
                if(_out[2]==0xAA && _out[3]==0x08) break;            
            }
            if(i>4) break;        
            CopyAndClear(_out, out, 10);
        }           
        {
            //version     ParseSerialCmd
            ParseSerialCmd(uart_version, 8, _out, 10);
            CopyAndClear(_out, &out[20], 10);
            //tk         ParseSerialCmd
            ParseSerialCmd(uart_tk, 8, _out, 10);
            CopyAndClear(_out, &out[30], 10);
            //tb         ParseSerialCmd
            ParseSerialCmd(uart_tb, 8, _out, 10);
            CopyAndClear(_out, &out[40], 10);
            //pk         ParseSerialCmd
            ParseSerialCmd(uart_pk, 8, _out, 10);
            CopyAndClear(_out, &out[50], 10);
            //pb         ParseSerialCmd
            ParseSerialCmd(uart_pb, 8, _out, 10);
            CopyAndClear(_out, &out[60], 10);
            //uart temp     ParseSerialCmd
            ParseSerialCmd(uart_calT, 8, _out, 10);
            CopyAndClear(_out, &out[70], 10);
            //uart press     ParseSerialCmd
            ParseSerialCmd(uart_calP, 8, _out, 10);
            CopyAndClear(_out, &out[80], 10);
        }        
                
        {
            //iic  temp     ParseI2CCmd
            ParseI2CCmd(iic_write_calT, 0, _out, 0 );
            osDelay(100);
            ParseI2CCmd(iic_write_calT2, 0, _out, 0 );
            ParseI2CCmd(iic_read_calT, 0, _out, 10 );  
            CopyAndClear(_out, &out[90], 10);
            osDelay(20);
            //iic  press     ParseI2CCmd
            ParseI2CCmd(iic_write_calP, 0, _out, 0 );
            osDelay(100);
            ParseI2CCmd(iic_write_calP2, 0, _out, 0 );
            ParseI2CCmd(iic_read_calP, 0, _out, 10 ); 
            CopyAndClear(_out, &out[100], 10);        
        } 
        {
            //owi press    ParseProductCmd
            ParseProductCmd(NULL,0,_out, 10);
            CopyAndClear(_out, &out[10], 10);           
        }        
            
        {
            //work current  ParseSystemCmd
            ParseSystemCmd(sys_adc_183, 1, _out, 4);
            CopyAndClear(_out, &out[110], 4);        
        }


    } while(0);        
    
    //Close power
    ParseSystemCmd(sys_vdd_off_183, 2, _out, 4);
}
