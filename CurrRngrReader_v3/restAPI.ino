/*
***************************************************************************  
**  Program  : restAPI, part of CurrRngrReader
**  Version  : v3.0.4
**
**  Copyright (c) 20210 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

char objSprtr[10]         = "";


//=============================================================================
void processAPI() 
{
  char jsonBuff[300] = "";

  char *URI = (char*)httpServer.uri().c_str();

  DebugTln("processAPI() ..");

  if (httpServer.method() == HTTP_GET)
        {DebugTf("incomming URI is [%s] method[GET] \r\n", URI); }
  else  {Debugf("incomming URI is [%s] method[PUT] \r\n", URI); }
  
  if (httpServer.uri().indexOf("api/updateField") > 0)
  {
    String  jsonIn  = httpServer.arg(0).c_str();
    String  pair[3];
    int8_t  nPair = 0;
    DebugTln(jsonIn);
    jsonIn.replace("{", "");
    jsonIn.replace("}", "");
    jsonIn.replace("\"", "");
    //--> words[0]      words[1]
    //--> field:DUTname,value:ZD Device
    int8_t  wc = splitString(jsonIn, ',', words, 10);
    //--> pair[0]      pair[1]
    //--> field        DUTname
    nPair = splitString(words[0], ':', pair, 3);
    if (pair[0] == "field" && pair[1] == "DUTname")
    {
      //--> pair[0]      pair[1]
      //--> value        <newValue>
      nPair = splitString(words[1], ':', pair, 3);
      DebugT("DUTname => [");
      Debug(pair[1]);
      Debugln("]");
      sprintf(DUTname, "%s", pair[1].c_str());
    }
    else if (pair[0] == "field" && pair[1] == "cutOffCurr")
    {
      nPair = splitString(words[1], ':', pair, 3);
      DebugT("cutOffCurr => [");
      Debug(pair[1]);
      Debugln("]");
      cutOffCurr = pair[1].toFloat();
    }
    else if (pair[0] == "field" && pair[1] == "sleepCurr")
    {
      nPair = splitString(words[1], ':', pair, 3);
      DebugT("sleepCurr => [");
      Debug(pair[1]);
      Debugln("]");
      sleepCurr = pair[1].toFloat();
    }
    else if (pair[0] == "field" && pair[1] == "skipCurr")
    {
      nPair = splitString(words[1], ':', pair, 3);
      DebugT("skipCurr => [");
      Debug(pair[1]);
      Debugln("]");
      skipCurr = pair[1].toFloat();
    }
    else if (pair[0] == "field" && pair[1] == "lowPass")
    {
      nPair = splitString(words[1], ':', pair, 3);
      DebugT("lowPass => [");
      Debug(pair[1]);
      Debugln("]");
      if (pair[1])  lowPassFilterOn = true;
      else          lowPassFilterOn = false;
    }

    httpServer.sendContent("{\"updateField\": \"OK\"}");
    actState = IDLE_STATE;

  }
  else if (httpServer.uri().indexOf("api/startRec") > 0)
  {
    httpServer.sendContent("{\"startRec\": \"OK\"}");
    actState = START_REC_STATE;

  }
  else if (httpServer.uri().indexOf("api/stopRec") > 0)
  {
    if (actState == RECORDING_STATE)
    {
      httpServer.sendContent("{\"stopRec\": \"OK\"}");
      actState = STOP_REC_STATE;
    }
    else
    {
      httpServer.sendContent("{\"stopRec\": \"ERROR\"}");
      actState = IDLE_STATE;
    }

  }
  else if (httpServer.uri().indexOf("api/listRec") > 0)
  {
    String lLine;
    if (actState == IDLE_STATE)
    {
      httpServer.sendContent("{\"listRec\": \"OK\"}");
      actState = LIST_REC_STATE;
    }
    else
    {
      httpServer.sendContent("{\"listRec\": \"ERROR\"}");
      actState = IDLE_STATE;
    }
  }
  else if (httpServer.uri().indexOf("api/dataPoints") > 0)
  {
    snprintf(jsonBuff, sizeof(jsonBuff), "{\"dataPoints\":%d}", dataPoints);
    httpServer.sendContent(jsonBuff);
  }
  else if (httpServer.uri().indexOf("api/timeSpan") > 0)
  {
    snprintf(jsonBuff, sizeof(jsonBuff), "{\"timeSpan\":%d}", timeSpan);
    httpServer.sendContent(jsonBuff);
  }
  else if (httpServer.uri().indexOf("api/minCurrent") > 0)
  {
    snprintf(jsonBuff, sizeof(jsonBuff), "{\"minCurrent\":%.1f}", minCurrent);
    httpServer.sendContent(jsonBuff);
  }
  else if (httpServer.uri().indexOf("api/maxCurrent") > 0)
  {
    snprintf(jsonBuff, sizeof(jsonBuff), "{\"maxCurrent\":%.1f}", maxCurrent);
    httpServer.sendContent(jsonBuff);
  }
  else if (httpServer.uri().indexOf("api/analyzeRec") > 0)
  {
    if (actState == IDLE_STATE)
    {
      httpServer.sendContent("{\"analyzeRec\": \"OK\"}");
      actState = ANALYZE_REC_STATE;
    }
    else
    {
      httpServer.sendContent("{\"stopRec\": \"ERROR\"}");
      actState = IDLE_STATE;
    }

  }
  else if (httpServer.uri().indexOf("api/downloadDat") > 0)
  {    
    char inLine[100];
    int  inLineLen = 0;
    if (LittleFS.exists("currRngr.p"))
    {
      File datFile;
      datFile = LittleFS.open("currRngr.p", "r");
      sprintf(inLine, "attachment; filename=\"currRngr.p\"");
      DebugTf("api/downloadDat: [%s]\r\n", inLine);
      //-- send headers ---
      httpServer.sendHeader("Access-Control-Allow-Origin", "*");
      httpServer.sendHeader("Content-Disposition", inLine);
      httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
      if (datFile.available())
      {
        memset(inLine, 0, sizeof(inLine));
        datFile.readBytesUntil('\n', inLine, sizeof(inLine));
        inLineLen = strlen(inLine);
        inLine[inLineLen++] = '\n';
        inLine[inLineLen++] = 0;
        httpServer.send(200, "application/csv", inLine);
      }
      //-- and now the rest ..
      while(datFile.available())
      {
        memset(inLine, 0, sizeof(inLine));
        datFile.readBytesUntil('\n', inLine, sizeof(inLine));
        inLineLen = strlen(inLine);
        inLine[inLineLen++] = '\n';
        inLine[inLineLen++] = 0;
        httpServer.sendContent(inLine);
      }
      datFile.close();
    }
    else
    {
      httpServer.sendContent("{\"downloadDat\": \"ERROR\"}");
      actState = IDLE_STATE;
    }
    //-- send endi ---
    httpServer.sendContent("");
    httpServer.client().stop(); // Stop is needed because no content length was sent

  }

} // processAPI()


//=============================================================================
void sendStartJsonObj(const char *objName)
{
  char sBuff[50] = "";
  objSprtr[0]    = '\0';

  sprintf(sBuff, "{\"%s\":{\r\n", objName);
  DebugT(sBuff);
  httpServer.sendHeader("Access-Control-Allow-Origin", "*");
  httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer.send(200, "application/json", sBuff);
  
} // sendStartJsonObj()


//=======================================================================
void sendEndJsonObj()
{
  DebugTln("}}");
  httpServer.sendContent("}}\r\n");

  //httpServer.sendHeader( "Content-Length", "0");
  //httpServer.send ( 200, "application/json", "");
  
} // sendEndJsonObj()

/***************************************************************************
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit
* persons to whom the Software is furnished to do so, subject to the
* following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
* OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
* THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* 
****************************************************************************
*/
