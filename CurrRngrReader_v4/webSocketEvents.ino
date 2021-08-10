

//===========================================================================================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght)
//===========================================================================================
{
    String  text = String((char *) &payload[0]);
    char *  textC = (char *) &payload[0];
    String  pOut[5], pFld[5], pVal[5], jsonString;
    int8_t  deviceNr;

    switch(type) {
        case WStype_DISCONNECTED:
            DebugTf("[%u] Disconnected!\n", num);
            isConnected = false;
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                if (!isConnected) {
                  DebugTf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
                  isConnected = true;
                  webSocket.broadcastTXT("{\"msgType\":\"ConnectState\",\"Value\":\"Connected\"}");
                  sendDevInfo();
                  sendFieldsVal();
                }
        
            }
            break;
        case WStype_TEXT:

            DebugTf("[%u] Got message: [%s]\n", num, payload);
            //webSocketHeartbeat = millis() + 20000;
            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            if (text.indexOf("getDevInfo") > -1) 
            {
              sendDevInfo();
              DebugTln(jsonString);
              webSocket.broadcastTXT(jsonString);
            } 
            else if (text.indexOf("getFieldsVal") > -1) 
            {
              DebugTln("sendFieldsVal");
              sendFieldsVal();
              DebugTln(jsonString);
              webSocket.broadcastTXT(jsonString);
            } 
            if (text.indexOf("updateField") > -1) 
            {
              //Debugf("(1) text: [%s]\n", text.c_str());
              splitString(text, ',', pOut, 3);
              //Debugf("(2) updateField: pOut[0] [%s]\n", pOut[0].c_str());
              splitString(pOut[0], '=', pFld, 3);
              //DebugTf("(3) pFld0:%s pFld1:[%s] => ", pFld[0].c_str(), pFld[1].c_str());
              splitString(pOut[1], '=', pVal, 3);
              DebugTf("%s [%s]\n", pVal[0].c_str(), pVal[1].c_str());
              //DebugTf("(4) indexOf(lowPass) is [%d]\r\n", pFld[1].indexOf("lowPass"));
              if (pFld[1].indexOf("cutOffCurr") > -1) 
                    cutOffCurr = pVal[1].toFloat();
              else if (pFld[1].indexOf("sleepCurr")> -1) 
                    sleepCurr = pVal[1].toFloat();
              else if (pFld[1].indexOf("skipCurr") > -1) 
                    skipCurr = pVal[1].toFloat();
              else if (pFld[1].indexOf("lowPass") > -1) 
              {
                //DebugT("lowPass : ");
                if (pVal[1].toInt() == 1) 
                      lowPassFilterOn = true;
                else  lowPassFilterOn = false;
                //DebugTf("%s\r\n", lowPassFilterOn ? "ON" : "OFF");
              }
              else if (pFld[0].indexOf("DUTname") == 0) 
                    snprintf(DUTname, sizeof(DUTname), "%s", pVal[1].c_str());
            }
            else if (text.indexOf("state:Record") > -1) 
            {
              webSocket.broadcastTXT("state:Record"); 
              actState = START_REC_STATE;
            }
            else if (text.indexOf("state:StopRec") > -1) 
            {
              webSocket.broadcastTXT("state:Idle"); 
              actState = STOP_REC_STATE;
            }
            else if (text.indexOf("state:ListRec") > -1) 
            {
              webSocket.broadcastTXT("state:ListRec"); 
              actState = LIST_REC_STATE;
            }
            else if (text.indexOf("state:AnalyzeRec") > -1) 
            {
              webSocket.broadcastTXT("state:AnalyzeRec"); 
              actState = ANALYZE_REC_STATE;
            }
            break;
                        
    } // switch(type)
    
} // webSocketEvent()


//===========================================================================================
void handleWebSocketState(int16_t deviceNr, uint8_t * webSocketData,  size_t lenght)
//===========================================================================================
{
  String    message = "", response;

  DebugTf("[%s] \n", webSocketData);

  //statusChanged = true;

  webSocket.broadcastTXT("OK");  // tell all other browser-clients

  //sendHTTPrequest(masterIPaddress.c_str(), deviceNr);

} // handleWebSocketState()


//===========================================================================================
void sendDevInfo() 
//===========================================================================================
{
  String jsonString = "";

    jsonString  = "{";
    jsonString += "\"msgType\": \"devInfo\"";
    jsonString += ", \"devName\": \"" + String(_HOSTNAME) + " \"";
    jsonString += ", \"devIPaddress\": \"" + WiFi.localIP().toString() + " \"";
    jsonString += ", \"devVersion\": \"" + String(_FW_VERSION) + "\"";
    jsonString += "}";
    DebugTf("broadcastTXT(%s)\n", jsonString.c_str());
    webSocket.broadcastTXT(jsonString);

} // sendDevInfo()


//===========================================================================================
void sendFieldsVal() 
//===========================================================================================
{
  String jsonString = "";

    jsonString  = "{";
    jsonString += "\"msgType\": \"fieldsVal\"";
    jsonString += ", \"DUTname\": \"" + String(DUTname) + "\"";
    jsonString += ", \"cutOffCurr\": " + String(cutOffCurr);
    jsonString += ", \"sleepCurr\": " + String(sleepCurr);
    jsonString += ", \"skipCurr\": " + String(skipCurr);
    if (lowPassFilterOn)
            jsonString += ", \"lowPass\": 1";
    else    jsonString += ", \"lowPass\": 0";
    jsonString += "}";
    DebugTf("broadcastTXT(%s)\n", jsonString.c_str());
    webSocket.broadcastTXT(jsonString);
    delay(100);
    webSocket.broadcastTXT("state:Idle"); 

} // sendFieldsVal()
