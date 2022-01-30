
  switch (missing)
	{
	case 1:
	  prvHistory[start + hMax - missing] = tse_buf;
	  missing -= 1;
	case 0:
	  prvHistory[start++] = tse_buf;
	  break;
	default:
	  prvHistory[start + hMax - missing] = tse_buf;
	  missing -= 1;
	  snprintf(prvSpeedToShowLine1.DataStr, sizeof(lcdLine_t), "Missing values ");
	  snprintf(prvSpeedToShowLine2.DataStr, sizeof(lcdLine_t), "%d               ", missing);            
	}



// length power of 2 -- circular 
#define historyPower  2 
static const unit32_t prvhMax = (1 << historyPower);
static timeStoreElement_t prvHistory[prvhMax];

static void prvInitHistory(void)
{
  int i;
  for(i=0; i < (1 << historyPower)prvhMax; i++)
  {
    prvHistory[i].minuteCount = timer1_speedINVALID_TSE_MINUTE_COUNT;
  }
}

static void prvSpeedCalc(timeStoreElement_t tse_buf)
{
  static uint32_t start = 0;
  static uint32_t missing = prvhMax;
  
  uint32_t j;
  uint32_t h0;
  uint32_t h1;
  const uint64_t tick2rpm = configCPU_CLOCK_HZ / 60; 
  uint64_t sum = 0;
  
  BaseType_t retval;

  /* Filling up history */
  if (1 < missing)
	{
	  prvHistory[start + prvhMax - missing] = tse_buf;
	  missing -= 1;
	  snprintf(prvSpeedToShowLine1.DataStr, sizeof(lcdLine_t), "Missing values ");
	  snprintf(prvSpeedToShowLine2.DataStr, sizeof(lcdLine_t), "%d               ", missing);
	  return;
	}
  /* Got enough history  */
  if (1 == missing)
	{
	  prvHistory[start + prvhMax - missing] = tse_buf;
	  missing -= 1;
	}
  else
	{
	  prvHistory[start++] = tse_buf;
	}
  /* 
   * Averaging diffs better than "(last - first) / N" 
   * Easy way out - require prvHmax even
   */
  for(j = 0; j < prvhMax ; j += 2) 
	{
	  h0 = (start + j ) & (prvhMax - 1);
	  h1 = (start + hmax/2 + j ) & (prvhMax - 1);
	  /* 
	   * Accepting minutCount diff of 2 is not really reasonable.
	   * It does allow drop-through though :-)
	   */
	  switch (prvHistory[h1].minuteCount - prvHistory[h0].minuteCount)
		{
		case 2:
		  sum += timer1_TICKS_PER_MINUTE;
		case 1:
		  sum += timer1_TICKS_PER_MINUTE;
		case 0:
		  sum += (prvHistory[h1].captureCount - prvHistory[h0].captureCount);
		  break;
		default:
		  snprintf(prvSpeedToShowLine1.DataStr, sizeof(lcdLine_t), "Invalid mC diff");
		  snprintf(prvSpeedToShowLine2.DataStr, sizeof(lcdLine_t), "%08X       ",
				   prvHistory[h1].minuteCount - prvHistory[h0].minuteCount);            
		  return;
		}
	}
  snprintf(prvSpeedToShowLine1.DataStr, sizeof(lcdLine_t), "Avg rpm:       ");
  snprintf(prvSpeedToShowLine2.DataStr, sizeof(lcdLine_t), "%04d           ",
		   (uint32_t) (sum / prvhMax / tick2rpm));
  return;
}
#undef historyPower
