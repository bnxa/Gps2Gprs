#include "nema_decode.h"
#include "bsp_gps_usart.h"

//DMA传输结束标志
extern __IO uint8_t GPS_TransferEnd  , GPS_HalfTransferEnd;

extern uint8_t gps_rbuff[GPS_RBUFF_SIZE];
extern uint8_t gps_rbuff_BDGSV[GPS_BDGSV_SIZE];	//可见北斗卫星信息 
extern uint8_t gps_rbuff_GNGGA[GPS_GNGGA_SIZE]; //GPS/北斗定位信息
extern uint8_t gps_rbuff_GNGSA[GPS_GNGSA_SIZE]; //当前卫星信息
extern uint8_t gps_rbuff_GPGSV[GPS_GPGSV_SIZE];	//可见 GPS 卫星信息 
extern uint8_t gps_rbuff_GNRMC[GPS_GNRMC_SIZE];	//推荐定位信息
extern uint8_t gps_rbuff_GNVTG[GPS_GNVTG_SIZE];	//地面速度信息
extern uint8_t gps_rbuff_GNGLL[GPS_GNGLL_SIZE];	//大地坐标信息
extern uint8_t gps_rbuff_GNZDA[GPS_GNZDA_SIZE];	//当前时间(UTC 1 )信息 


//比较命令帧头
//失败 0
//成功 1
uint8_t cmp(uint16_t ccBegin,const char* ccStr)
{
	int i=0;
	for(i=0;i<strlen(ccStr);i++)
	{
		if(ccStr[i]!=gps_rbuff[ccBegin+i])
			return 0;
	}
	return 1;
}

 
void nema_cat(uint16_t ccBegin,uint16_t ccEnd)
{ 
	uint16_t i=0;
	uint16_t fLen = ccEnd - ccBegin;
	   
	// 1 $GNGGA GPS/北斗定位信息 72
	if(cmp(ccBegin,"$GNGGA"))
	{
		if(fLen<=GPS_GNGGA_SIZE)
		{
			Clear_GNGGA();
			for(i=0;i<=fLen;i++)
			{
				gps_rbuff_GNGGA[i]=gps_rbuff[ccBegin+i];
			}
			GPS_DEBUG("\r\n          1 %s", gps_rbuff_GNGGA);
			
		} 
	}
	//2 $GNGSA 当前卫星信息 65
	else if(cmp(ccBegin,"$GNGSA"))
	{
		if(fLen<= GPS_GNGSA_SIZE)
		{
			Clear_GNGSA();
			for(i=0;i<=fLen;i++)
			{
				gps_rbuff_GNGSA[i] = gps_rbuff[ccBegin+i];
			}
			GPS_DEBUG("\r\n          2 %s", gps_rbuff_GNGSA);			
		} 
	}
		
	//3 $GPGSV 可见 GPS 卫星信息 210
	else if(cmp(ccBegin,"$GPGSV"))
	{
		if(fLen<=GPS_GPGSV_SIZE)
		{
			Clear_GPGSV();
			for(i=0;i<=fLen;i++)
			{
				gps_rbuff_GPGSV[i]=gps_rbuff[ccBegin+i];
			}
			GPS_DEBUG("\r\n          3 %s", gps_rbuff_GPGSV);	
		} 
	}
	
	//4 $BDGSV 可见北斗卫星信息 210
	else if(cmp(ccBegin,"$BDGSV"))
	{
		if(fLen <= GPS_BDGSV_SIZE) //长度小于命令1最大帧长
		{ 
			Clear_BDGSV(); //清空命令缓冲区
			for(i=0;i<=fLen;i++)
			{
				gps_rbuff_BDGSV[i] = gps_rbuff[ccBegin+i]; 	//更新命令帧
			} 
			GPS_DEBUG("\r\n          4 %s",gps_rbuff_BDGSV);
		} 
	}
	//5 $GNRMC 推荐定位信息 70
	else if(cmp(ccBegin,"$GNRMC"))
	{
		if(fLen <= GPS_GNRMC_SIZE)
		{
			Clear_GNRMC();
			for(i=0;i<=fLen;i++)
			{
				gps_rbuff_GNRMC[i] = gps_rbuff[ccBegin+i];			
			}
			GPS_DEBUG("\r\n          5 %s",gps_rbuff_GNRMC);
		}
	}
	
	//6 $GNVTG 地面速度信息 34
	else if(cmp(ccBegin,"$GNVTG"))
	{
		if(fLen <= GPS_GNVTG_SIZE)
		{
			Clear_GNVTG();
			for(i=0;i<=fLen;i++)
			{
				gps_rbuff_GNVTG[i] = gps_rbuff[ccBegin+i];
			}
			GPS_DEBUG("\r\n          6 %s",gps_rbuff_GNVTG);
		}
	}
	//7 $GNGLL 大地坐标信息 --
	else if(cmp(ccBegin,"$GNGLL"))
	{
		if(fLen <= GPS_GNGLL_SIZE)
		{
			Clear_GNGLL();
			for(i=0;i<=fLen;i++)
			{
				gps_rbuff_GNGLL[i] = gps_rbuff[ccBegin+i];
			} 
			GPS_DEBUG("\r\n          7 %s",gps_rbuff_GNGLL);
		}
	}
	//8 $GNZDA 当前时间(UTC 1 )信息 --
	else if(cmp(ccBegin,"$GNZDA"))
	{
		if(fLen <= GPS_GNZDA_SIZE)
		{
			Clear_GNZDA();
			for(i=0;i<=fLen;i++)
			{
				gps_rbuff_GNZDA[i] = gps_rbuff[ccBegin+i];
			}
			GPS_DEBUG("\r\n          8 %s",gps_rbuff_GNZDA);
		}
	}
}

void nema_decode(void)
{ 
	uint16_t i =0;
	uint16_t fBegin =0;
	uint16_t fEnd =0;
	uint8_t fHasHead=0;
	
	GPS_DEBUG(" ------nema_decode  begin -------------\r\n");
	//	GSM_DELAY(1000);
  //收到 GPS 的数据
 
	for(i=0;i<GPS_RBUFF_SIZE ;i++)
	{
		if(gps_rbuff[i]=='$')
		{
			fHasHead = 1;
			fBegin = i;
		}
		if(gps_rbuff[i]=='\r')
		{					
			if(gps_rbuff[++i]=='\n')
			{
				fEnd = i;
				if(fEnd-fBegin > 10)
				{
					if(fHasHead)
					{
						GPS_DEBUG(" begin=%d,end=%d, fEnd-fBegin =%d ",fBegin,fEnd,fEnd-fBegin);
						nema_cat(fBegin,fEnd);
						fHasHead = 0;
					}
				}
			}
		}
	}
	
	GPS_DEBUG(" ------nema_decode  end -------------\r\n");
  
}

