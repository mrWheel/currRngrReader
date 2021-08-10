/*
***************************************************************************  
**  Program  : CurrRngrReader
*/
#define _FW_VERSION "v4.0.1 (10-08-2021)"
/*
**  Copyright (c) 2021 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*      
  Arduino-IDE settings for ATtiny3216:

    - Board: "Generic ESP8266 Module"
    - Builtin Led: "2"
    - CPU Frequency: "80 MHz"
    - Crystal Frequency: "26 MHz"
    - Flash Size: "4MB (FS:2MB OTA:~1019KB)"
    - Port: <select correct port>
*/

#define USE_UPDATE_SERVER

#include "CurrRngrReader.h"

//ESP8266WebServer httpServer(80);
//#ifdef USE_UPDATE_SERVER
//  #include "updateServerHtml.h"
//  ESP8266HTTPUpdateServer httpUpdater(true);
//#endif

static    FSInfo LittleFSinfo;

SoftwareSerial currRngr(SOFT_RX, SOFT_TX); // RX, TX

enum    { IDLE_STATE, START_REC_STATE, RECORDING_STATE, STOP_REC_STATE, LIST_REC_STATE, ANALYZE_REC_STATE };

uint8_t   actState = 0;
uint16_t  startPos = 0;
uint32_t  totalDuration;
char      DUTname[50] = {"Device Under Test"};
float     cutOffCurr = 1.0;
float     sleepCurr = 5.0;
float     skipCurr = 1000;
float     thisCurr;
float     minCurrent = 99999.99;
float     maxCurrent = 0;
int32_t   dataPoints = 0, timeSpan  = 0;
float     cummPower = 0, cyclePower = 0;
bool      lowPassFilterOn = false;
String    jsonString;

File csvFile, gnuFile;



//-------------------------------------------------------------
float powerConsumption()
{
  uint32_t  startTime = 0, plotTime = 0, prevPlotTime = 0;
  int32_t   duration;
  float     power = 0.0, cummPower = 0;
  float     prevCurr, workCurr;
  char      buff[80] = {};
  
  minCurrent = 99999.99;
  maxCurrent = 0.00;

  if (LittleFS.exists("currRngr.csv"))
  {
    csvFile  = LittleFS.open("currRngr.csv", "r");
    while(csvFile.available())
    {
      prevPlotTime  = plotTime;      
      plotTime      = csvFile.parseInt();
      if (prevPlotTime == 0) prevPlotTime = plotTime;
      prevCurr = thisCurr;
      thisCurr = csvFile.parseFloat();
      if ((startTime == 0) && (thisCurr > (cutOffCurr*1.2))) startTime = plotTime;
      if (thisCurr < 0.0)       thisCurr = 0.01;          //-- never negative ...
      if (thisCurr < sleepCurr) thisCurr = sleepCurr;     //-- modurate spikes ...
      if (thisCurr > skipCurr)  thisCurr = (prevCurr*2);  //-- modurate spikes ...
      if (lowPassFilterOn)
            workCurr = (thisCurr + prevCurr) / 2;
      else  workCurr = thisCurr;
      duration = plotTime - prevPlotTime;
      if ((duration >= 0) && (workCurr > (cutOffCurr*1.2)))
      {
        snprintf(buff, sizeof(buff),"[%6d][%4d] -> [%.3f]", plotTime, duration, workCurr);
        DebugTln(buff);
        jsonString  = "{";
        jsonString += "\"msgType\": \"logWindowAdd\"";
        jsonString += ", \"line\": \"" + String(buff) + "\"";
        jsonString += "}";
        webSocket.broadcastTXT(jsonString); 

        power = (duration * workCurr) / 3600000;
        cummPower += power;
        totalDuration = plotTime - startTime;
        if (workCurr < minCurrent) minCurrent = workCurr;
        if (workCurr > maxCurrent) maxCurrent = workCurr;
      }
    }
    csvFile.close();
  }
  
  return cummPower;
    
} //  powerConsumption()


//-------------------------------------------------------------
void analyzeRecording() 
{
  uint32_t  startTime=0, endTime=0, plotTime=0, prevPlotTime=0;
  float     leadingVal[_MAX_LEADING];
  uint32_t  leadingTime[_MAX_LEADING];
  float     prevCurr, workCurr;
  uint16_t  leadingPos  = 0, o, s;
  bool      readLeading = true, leadingDone = false;
  char      buff[100] = {};
  
  webSocket.broadcastTXT("logWindowClear"); 

  //Debugln("=============================================================");
  //listRecording();
  Debugln("=============================================================");

  dataPoints = 0;
  cummPower  = 0;
  timeSpan   = 0;
  
  // calculates also totalDuration and sets minCurrent and maxCurrent
  cyclePower = powerConsumption();
  cummPower  = cyclePower + (sleepCurr * (3600000 - totalDuration)) / 3600000;

  if (LittleFS.exists("currRngr.csv"))
  {
    csvFile = LittleFS.open("currRngr.csv", "r");
    gnuFile = LittleFS.open("currRngr.p", "w+");

    gnuFile.println("set terminal pngcairo size 1024,512 enhanced font 'Verdane,10'");
    gnuFile.println("set output './currRngr.png'");
    gnuFile.printf( "set title 'power consumption %s %.3f mA/h'\r\n", DUTname, cummPower);
    gnuFile.println("set style fill solid 0.5 noborder");
    gnuFile.println("set xlabel 'milliseconds'");
    gnuFile.println("set ylabel 'mA'");
    gnuFile.printf("set label 4  \"  sleep Current %.2f mA\" at 100,%d  front norotate nopoint\r\n"
                                            , sleepCurr, (int)maxCurrent-2);

    gnuFile.println("set mxtics 10");
    gnuFile.println("set tics out");
    gnuFile.println("set grid");
    gnuFile.printf("plot '-' title '1 cycle %.3f mA/h' with boxes lc rgb 'red'\r\n", cyclePower);
    gnuFile.println(" ");

    gnuFile.printf("%6d    %.2f\r\n", 0, 0.0); 

    while(csvFile.available())
    {
      prevPlotTime = plotTime;
      prevCurr     = thisCurr;
      plotTime  = csvFile.parseInt();
      thisCurr  = csvFile.parseFloat();
      if (prevPlotTime == 0) prevPlotTime = plotTime;
      if (thisCurr < 0.0)  thisCurr = 0.001;              //-- never negative ...
      if (thisCurr < sleepCurr) thisCurr = sleepCurr;     //-- modurate spikes ...
      if (thisCurr > skipCurr)  thisCurr = (prevCurr*2);  //-- modurate spikes ...
      if (lowPassFilterOn)
            workCurr = (thisCurr + prevCurr) / 2;
      else  workCurr = thisCurr;
      if (plotTime > endTime) endTime   = plotTime;
      if ((startTime == 0) && (workCurr > (cutOffCurr*1.2)))  startTime = plotTime;
      if (workCurr > maxCurrent) maxCurrent = workCurr;
      if (workCurr < minCurrent) minCurrent = workCurr;
      dataPoints++;
      if (readLeading && (workCurr < (cutOffCurr*1.2)))
      {
        leadingTime[(leadingPos%_MAX_LEADING)] = plotTime;
        leadingVal[(leadingPos%_MAX_LEADING)]  = workCurr;
        leadingPos++;
      }
      else 
      {
        readLeading = false;
        if (!leadingDone && (startTime == 0))
        {
          leadingPos++;
          if (startTime == 0) startTime = plotTime;
          for (o=0; o<(_MAX_LEADING -1); o++)
          {
            DebugTf("[%3d] -> [%5d] [%.2f]\r\n", o
                  , leadingTime[((o+leadingPos)%_MAX_LEADING)] - startTime
                  , leadingVal[((o+leadingPos)%_MAX_LEADING)] );
            gnuFile.printf("%6d    %.2f\r\n", 
                    leadingTime[((o+leadingPos)%_MAX_LEADING)] - startTime
                  , leadingVal[((o+leadingPos)%_MAX_LEADING)] );
          }
          leadingDone = true;
          s = 0;
          DebugTf("[%3d] -> [%5d] [%.2f]\r\n", o++
                                             , plotTime - startTime
                                             , workCurr );
          gnuFile.printf("%6d    %.2f\r\n", plotTime - startTime
                                          , workCurr );

        }
        else
        {
          if ((workCurr < (cutOffCurr*1.2)) && (s >= _MAX_LEADING))
          {
            break;
          }
          if (workCurr >= (cutOffCurr*1.2))
                s = 0;
          else  s++;
          if (plotTime > prevPlotTime)
          {
            DebugTf("[%3d] -> [%5d] [%.2f]\r\n", o++
                                             , plotTime - startTime
                                             , workCurr );
            gnuFile.printf("%6d    %.2f\r\n", plotTime - startTime
                                          , workCurr );
          }
          
        }
      }
    }
    csvFile.close();
    gnuFile.close();
  }
  timeSpan = endTime - startTime;
  Debugln();
  Debugln("=============================================================");
  DebugTf("   DataPoints [%d]\r\n", dataPoints);
  DebugTf("leadingPoints [%d]\r\n", leadingPos);
  DebugTf("totalDuration [%d] ms\r\n", totalDuration);
//DebugTf("      endTime [%d] ms\r\n", endTime);
  DebugTf("    Time span [%d] ms\r\n", timeSpan);
  DebugTf("  Min.Current [%.2f]\r\n", minCurrent);
  DebugTf("  Max.Current [%.2f]\r\n", maxCurrent);
  DebugTf("        mA/h  [%.2f]\r\n", cummPower);

  jsonString  = "{";
  jsonString += "\"msgType\": \"analyzeData\"";
  jsonString += ", \"dataPoints\": " + String(dataPoints);
  jsonString += ", \"minCurr\": "    + String(minCurrent);
  jsonString += ", \"maxCurr\": "    + String(maxCurrent);
  jsonString += ", \"cummPower\": "  + String(cummPower);
  jsonString += ", \"timeSpan\": "   + String(timeSpan);
  jsonString += "}";
  DebugTln(jsonString);
  webSocket.broadcastTXT(jsonString); 

  webSocket.broadcastTXT("state:Idle"); 

} //  analyzeRecording()


//-------------------------------------------------------------
void listRecording() 
{
  uint16_t  o = 0;
  char buff[100] = {};

  webSocket.broadcastTXT("logWindowClear"); 

  if (!LittleFS.exists("currRngr.csv"))
  {
    jsonString  = "{";
    jsonString += "\"msgType\": \"logWindowAdd\"";
    jsonString += ", \"line\": \"currRngr.csv does not exist .. (error)\"";
    jsonString += "}";
    webSocket.broadcastTXT(jsonString); 

    DebugTln("\"currRngr.csv\" does not exist .. (error)");
    return;
  }
  
  csvFile = LittleFS.open("currRngr.csv", "r");
  while(csvFile.available())
  {
    uint32_t  i = csvFile.parseInt();
    float     x = csvFile.parseFloat();
    if (i<1)  continue;
    snprintf(buff, sizeof(buff),"[%3d] -> [%8d] [%.2f]", o++, i, x );
    DebugTln(buff);
    jsonString  = "{";
    jsonString += "\"msgType\": \"logWindowAdd\"";
    jsonString += ", \"line\": \"" + String(buff) + "\"";
    jsonString += "}";
    webSocket.broadcastTXT(jsonString); 

  }
  csvFile.close();
  DebugTln("Done ..");
  webSocket.broadcastTXT("state:Idle"); 
  
} //  listRecording()


//-------------------------------------------------------------
void stopRecording() 
{
  DebugTln("Close \"currRngr.csv\"..");
  csvFile.close();

} //  stopRecording()


//-------------------------------------------------------------
void startRecording() 
{
  csvFile = LittleFS.open("currRngr.csv", "w+");
  if (!csvFile)
  {
    DebugTln("Something went wrong opening \"currRngr.csv\" ..");
  }
  DebugTln("Start recording to \"currRngr.csv\" ..");

} //  startRecording()


//-------------------------------------------------------------
void record() 
{
  if (!csvFile) { actState = IDLE_STATE; return; }
  thisCurr = currRngr.parseFloat();
  csvFile.print(millis());
  csvFile.print("; ");
  csvFile.println(thisCurr);
  //Debugln(thisCurr);

} //  record()


//-------------------------------------------------------------
void setup() 
{
  Serial.begin(115200);
  while(!Serial) { delay(10); }

  startWiFi(_HOSTNAME, 240);
  startMDNS(_HOSTNAME);
  startTelnet();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  currRngr.begin(230400);

  Serial.println("\nLittleFS.begin() ..");
  setupFS();
  //-- remove csv file
  //if (LittleFS.exists("currRngr.csv"))
  //{
  //  Serial.println("Remove pending currRngr.csv file ..");
  //  LittleFS.remove("currRngr.csv");
  //}

  httpServer.serveStatic("/",           LittleFS, "/index.html");
  httpServer.on("/api",     processAPI);
  httpServer.serveStatic("/FSexplorer", LittleFS, "/littleFSexplorer.html");
  httpServer.serveStatic("/FSexplorer.png", LittleFS, "/FSexplorer.png");

  httpServer.begin();
  Serial.println("HTTP httpServer started");

#ifdef USE_UPDATE_SERVER
  httpUpdater.setup(&httpServer);
  httpUpdater.setIndexPage(UpdateServerIndex);
  httpUpdater.setSuccessPage(UpdateServerSuccess);
#endif

   
  Serial.println("\nAnd than it begins ...\n");
  //memset(buffer, 0, sizeof(buffer));
  //webSocket.broadcastTXT("state:Idle"); 
  actState = IDLE_STATE;
  
} //  setup()


//-------------------------------------------------------------
void loop() 
{
  httpServer.handleClient();
  MDNS.update();
  webSocket.loop();
  
  switch(actState)
  {
    case IDLE_STATE:  
        break;

    case START_REC_STATE:
        DebugTln("START_REC_STATE ..");
        startRecording();
        actState = RECORDING_STATE;
        break;

    case RECORDING_STATE:
        record();
        break;

    case STOP_REC_STATE:
        DebugTln("STOP_REC_STATE ..");
        stopRecording();
        actState = IDLE_STATE;
        break;

    case LIST_REC_STATE:
        DebugTln("LIST_REC_STATE ..");
        listRecording();
        actState = IDLE_STATE;
        break;

    case ANALYZE_REC_STATE:
        DebugTln("ANALYZE_REC_STATE ..");
        analyzeRecording();
        actState = IDLE_STATE;
        break;

    default:
        actState = IDLE_STATE;
        break;
        
  } // switch(actState)

} //  loop()
