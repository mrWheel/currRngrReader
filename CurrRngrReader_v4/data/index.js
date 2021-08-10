/*
***************************************************************************  
**  Program  : index.js, part of CurrRngrReader
**
**  Copyright (c) 2021 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/
  //const APIGW='http://'+window.location.host+'/api/';
  const APIGW=window.location.protocol+'//'+window.location.host+'/api/';

"use strict";

  let webSocketConn;

  let needReload  = true;
  webSocketConn = new WebSocket('ws://'+location.host+':81/', ['arduino']);
  
  webSocketConn.onopen    = function () { 
    console.log("Connected!");
    webSocketConn.send('Connect ' + new Date()); 
    console.log("getDevInfo");
    webSocketConn.send("getDevInfo");
    document.getElementById('gif').style.display = "none";
    needReload  = false;
  }; 
  webSocketConn.onclose     = function () { 
    document.getElementById('gif').style.display = "block";
    console.log(" ");
    console.log("Disconnected!");
    console.log(" ");
    let redirectButton = "<font='font-size: 25px;'>Disconneted from DONOFF webserver"; 
    redirectButton    += "<input type='submit' value='re-Connect' "; 
    redirectButton    += " onclick='window.location=\"/\";' />  ";     
    document.getElementById("resources").innerHTML = redirectButton;
    DOMloaded = false;
    needReload  = true;

  }; 
  webSocketConn.onerror   = function (error)  { 
    document.getElementById('gif').style.display = "block";
    console.log("Error: " + error);
    console.log('WebSocket Error ', error);
  };
  webSocketConn.onmessage = function (e) {
    parsePayload(e.data); 
  };
  
  window.onload=bootsTrap;
  window.onfocus = function() {
    if (needReload) {
      window.location.reload(true);
    }
  };
    
  //============================================================================  
  function bootsTrap() {
    console.log("bootsTrap()");
    
    document.getElementById('DUTname').addEventListener('focusout',function()
                                                {saveInput("DUTname");});
    
    document.getElementById('cutOffCurr').addEventListener('focusout',function()
                                                {saveInput("cutOffCurr");});
    
    document.getElementById('sleepCurr').addEventListener('focusout',function()
                                                {saveInput("sleepCurr");});
    
    document.getElementById('skipCurr').addEventListener('focusout',function()
                                                {saveInput("skipCurr");});

    document.getElementById('lowPass').addEventListener('click',function()
                                                {saveInput("lowPass");});

    document.getElementById('startRec').addEventListener('click',function()
                                                {startRecording();});
    
    document.getElementById('stopRec').addEventListener('click',function()
                                                {stopRecording();});
    
    document.getElementById('listRec').addEventListener('click',function()
                                                {listRecording();});
    
    document.getElementById('analyzeRec').addEventListener('click',function()
                                                {analyzeRecording();});
    
    document.getElementById('downloadDat').addEventListener('click',function()
                                                {downloadDat();});
    
    needReload = false;
    
    document.getElementById("startRec").disabled    = false;
    document.getElementById("stopRec").disabled     = true;
    document.getElementById("listRec").disabled     = false;
    document.getElementById("analyzeRec").disabled  = false;
    document.getElementById("downloadDat").disabled = false;
    document.getElementById("gif").style.display    = "none";

    //refreshDMdata();
    //TimerTime = setInterval(refreshDMdata, 30 * 1000); // repeat every 10s
  
  } // bootsTrap()
  
  //============================================================================  
      
  function parsePayload(payload) 
  {
    document.getElementById('gif').style.display = "block";

    validJson = IsJsonString(payload);
    if (validJson)
          console.log("JSON: "+JSON.stringify(payload));
    else  console.log("PLAIN: "+payload);
    if ( payload.indexOf('state:Record') !== -1 )// not json!
    {
      console.log("is Recording");
      document.getElementById("message").innerHTML = "RECORDING";
      disableButtons();
      document.getElementById("stopRec").disabled = false;
    }          
    else if ( payload.indexOf('state:Idle') !== -1 )// not json!
    {
      console.log("state:Idle");
      document.getElementById("message").innerHTML    = "Idle";
      document.getElementById("startRec").disabled    = false;
      document.getElementById("stopRec").disabled     = true;
      document.getElementById("listRec").disabled     = false;
      document.getElementById("analyzeRec").disabled  = false;
      document.getElementById("downloadDat").disabled = false;
      document.getElementById('gif').style.display    = "none";
    }     
    else if ( payload.indexOf('state:ListRec') !== -1 )// not json!
    {
      console.log("state:ListRec");
      document.getElementById("message").innerHTML    = "LIST RECORDS";
      disableButtons();
      document.getElementById('gif').style.display    = "block";
    }
    else if ( payload.indexOf('state:AnalyzeRec') !== -1 )// not json!
    {
      console.log("state:AnalyzeRec");
      document.getElementById("message").innerHTML    = "ANALYZING";
      disableButtons();
      document.getElementById('gif').style.display    = "block";
          
    } 
    else if ( payload.indexOf('logWindowClear') !== -1 )// not json!
    {
      console.log(payload);
      document.getElementById("logWindow").value = "";
      document.getElementById("logWindow").wrap  ='off';   
             
    } 
    else if (validJson) 
      {
        let jsonMessage = JSON.parse(payload);
        console.log("parsePayload(): [" + jsonMessage.msgType + "]");
        if (jsonMessage.msgType == "devInfo")
        {
          document.getElementById("devName").innerHTML      = jsonMessage.devName;
          document.getElementById("devIPaddress").innerHTML = jsonMessage.devIPaddress
          document.getElementById("devVersion").innerHTML   = jsonMessage.devVersion;
          console.log("parsePayload(IP): "+jsonMessage.devIPaddress);
        }
        else if (jsonMessage.msgType == "fieldsVal")
        {
          console.log("update fields ..");
          document.getElementById("DUTname").value    = jsonMessage.DUTname;
          document.getElementById("cutOffCurr").value = jsonMessage.cutOffCurr;
          document.getElementById("sleepCurr").value  = jsonMessage.sleepCurr;
          document.getElementById("skipCurr").value   = jsonMessage.skipCurr;
          if (jsonMessage.lowPass == 1)
                document.getElementById("lowPass").checked = true;
          else  document.getElementById("lowPass").checked = false;
          document.getElementById('gif').style.display    = "none";
        }
        else if (jsonMessage.msgType == "analyzeData")
        {
          console.log(">>"+jsonMessage.msgType);
          document.getElementById("dataPoints").innerHTML = jsonMessage.dataPoints;
          document.getElementById("minCurr").innerHTML    = jsonMessage.minCurr;
          document.getElementById("maxCurr").innerHTML    = jsonMessage.maxCurr;
          document.getElementById("timeSpan").innerHTML   = jsonMessage.timeSpan;
          document.getElementById("cummPower").innerHTML  = jsonMessage.cummPower;
        }
        else if (jsonMessage.msgType == "logWindowAdd")
        {
          document.getElementById("logWindow").value += jsonMessage.line+"\n";
        }
        else if ((jsonMessage.msgType == "Status")) 
        {
          updateStatus(payload);
        }
      } else 
      {
        console.log("parsePayload(): Don't know: [" + payload + "]\r\n");
        document.getElementById('gif').style.display    = "none";
      }

  };

  //============================================================================  
  function refreshDMdata()
  {
      
  } //  refreshDMdata()
  
  //============================================================================  
  function saveInput(fieldName)
  {
    var fieldVal = document.getElementById(fieldName).value
    console.log("saveInput("+fieldName+") ..");
    if (fieldName == "lowPass")
    {
      if (document.getElementById(fieldName).checked)
            fieldVal = 1;
      else  fieldVal = 0;
    }
    console.log(fieldName+" ["+fieldVal+"]");
    webSocketConn.send("updateField=" + fieldName + ", Val=" + fieldVal);
  
  } //  saveInput()
  
  //============================================================================  
  function startRecording()
  {
    var surrId, valId, stateId;
    
    console.log("startRecording() ..");
    document.getElementById('message').innerHTML = "";
    webSocketConn.send("state:Record");
  
  } // startRecording()
  
  //============================================================================  
  function stopRecording()
  {
    console.log("stopRecording() ..");
        
    document.getElementById('message').innerHTML = "";
    webSocketConn.send("state:StopRec");
    document.getElementById('gif').style.display = "none";

  } // stopRecording()
  
  //============================================================================  
  function listRecording()
  {
    console.log("listRecording() ..");
        
    document.getElementById('message').innerHTML = "";
    webSocketConn.send("state:ListRec");
      
  } // listRecording()
  
  //============================================================================  
  function analyzeRecording()
  {
    console.log("analyzeRecording() ..");
        
    document.getElementById('message').innerHTML = "";
    //saveInput("DUTname");
    saveInput("DUTname");
    saveInput("cutOffCurr");
    saveInput("sleepCurr");
    saveInput("skipCurr");
    saveInput("lowPass");
    webSocketConn.send("state:AnalyzeRec");
      
  } // analyzeRecording()


  //============================================================================  
  function downloadDat() 
  {
    console.log("downloadDat()..");
    moment.locale(); 
    var date = moment().format("YY-MM-DD-HH:mm");
    var fileName = "gnuplot_"+date+".p";
    console.log("fileName["+fileName+"]");
    doDownload(APIGW+"downloadDat", fileName);
    
  } // downloadDat()

  //============================================================================  
  function doDownload(url, filename) 
  {
    fetch(url).then(function(t) {
      return t.blob().then((b)=>{
          var a = document.createElement("a");
          a.href = URL.createObjectURL(b);
          a.setAttribute("download", filename);
          a.click();
        }
      );
    });
  } // doDownload()
  
  
  //============================================================================  
  function disableButtons() 
  {
    document.getElementById('gif').style.display = "block";

    document.getElementById("startRec").disabled    = true;
    document.getElementById("stopRec").disabled     = true;
    document.getElementById("listRec").disabled     = true;
    document.getElementById("analyzeRec").disabled  = true;
    document.getElementById("downloadDat").disabled = true;
    
  } //  disableButtons() 

  
    //============================================================================  
  function round(value, precision) 
  {
    var multiplier = Math.pow(10, precision || 0);
    return Math.round(value * multiplier) / multiplier;
  }

    
  //============================================================================  
  function IsJsonString(str) 
  {
    try {
        JSON.parse(str);
    } catch (e) {
        return false;
    }
    return true;
    
  } // IsJsonString()

  //============================================================================  
  function existingId(elementId) 
  {
    if(document.getElementById(elementId))
    {
      return true;
    } 
    console.log("cannot find elementId [" + elementId + "] reload ..");
    return false;
    
  } // existingId()

  //============================================================================  
  function printAllVals(obj) 
  {
    for (let k in obj) {
      if (typeof obj[k] === "object") {
        printAllVals(obj[k])
      } else {
        // base case, stop recurring
        console.log(obj[k]);
      }
    }
  } // printAllVals()
  
/*
***************************************************************************
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
***************************************************************************
*/
