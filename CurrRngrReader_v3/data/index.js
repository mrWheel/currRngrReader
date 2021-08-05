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

  let needReload  = true;
  
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
    //refreshDMdata();
    //TimerTime = setInterval(refreshDMdata, 30 * 1000); // repeat every 10s
  
  } // bootsTrap()
  
  //============================================================================  
  function refreshDMdata()
  {
    console.log("RefreshDMdata");
    fetch(APIGW+"dataPoints", {mode: 'cors'})
      .then(response => response.json())
      .then(json => {
        console.log("parsed --> json is ["+ JSON.stringify(json)+"]");
        data = json.dataPoints;
        console.log("json.dataPoints is ["+JSON.stringify(data)+"]");
        console.log("json.dataPoints is ["+data+"]");
        document.getElementById('dataPoints').innerHTML = data;
      })
      .catch(function(error) {
        document.getElementById('message').innerHTML = error;
      });     
      
    fetch(APIGW+"timeSpan", {mode: 'cors'})
      .then(response => response.json())
      .then(json => {
        console.log("parsed --> json is ["+ JSON.stringify(json)+"]");
        data = json.timeSpan;
        console.log("json.timeSpan is ["+JSON.stringify(data)+"]");
        console.log("json.timeSpan is ["+data+"]");
        document.getElementById('timeSpan').innerHTML = data;
      })
      .catch(function(error) {
        document.getElementById('message').innerHTML = error;
      });     
      
    fetch(APIGW+"minCurrent", {mode: 'cors'})
      .then(response => response.json())
      .then(json => {
        console.log("parsed --> json is ["+ JSON.stringify(json)+"]");
        data = json.minCurrent;
        console.log("json.minCurrent is ["+JSON.stringify(data)+"]");
        console.log("json.minCurrent is ["+data+"]");
        document.getElementById('minCurrent').innerHTML = data;
      })
      .catch(function(error) {
        document.getElementById('message').innerHTML = error;
      });     
      
    fetch(APIGW+"maxCurrent", {mode: 'cors'})
      .then(response => response.json())
      .then(json => {
        console.log("parsed --> json is ["+ JSON.stringify(json)+"]");
        data = json.maxCurrent;
        console.log("json.maxCurrent is ["+JSON.stringify(data)+"]");
        console.log("json.maxCurrent is ["+data+"]");
        document.getElementById('maxCurrent').innerHTML = data;
      })
      .catch(function(error) {
        document.getElementById('message').innerHTML = error;
      });     
      
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
      console.log(fieldName+" ["+fieldVal+"]");
    }
    document.getElementById('message').innerHTML = fieldVal;
    
    var payload = {
      field: fieldName,
      value: fieldVal
    };

    var data = new FormData();
    data.append( "json", JSON.stringify( payload ) );

    fetch(APIGW+"updateField",
    {
      method: "POST",
      body: data
    })
    .then(function(res){ return res.json(); })
    .then(function(data){ JSON.stringify( data ) })  

  
  } //  saveInput()
  
  //============================================================================  
  function startRecording()
  {
    var surrId, valId, stateId;
    
    console.log("startRecording() ..");
    document.getElementById('message').innerHTML = "";
    
    /*
    for(var s=1; s<8; s++)
    {
      valId   = "Val"+s;
      stateId = "State"+s;
      document.getElementById(valId).innerHTML = "  ";
      document.getElementById(stateId).innerHTML = " &nbsp; ";
      document.getElementById(stateId).style.backgroundColor = "lightgray"; 
    }
    */
    /*
    fetch(APIGW+"startRec", {mode: 'cors'})
    .then(function(response) {
      console.log(response);
      return response.text();
    })
    .then(function(text) {
      console.log('Request successful', text);
    })
    .catch(function(error) {
      console.log('Request failed', error)
    });
    */
            
    fetch(APIGW+"startRec", {mode: 'cors'})
      .then(response => response.json())
      /*.then(response => console.log(response))*/
      .then(json => {
        console.log("parsed --> json is ["+ JSON.stringify(json)+"]");
        data = json.startRec;
        console.log("json.startRec is ["+JSON.stringify(data)+"]");
        if (data == "OK")  document.getElementById('message').innerHTML = "RECORDING DATA";
        else               document.getElementById('message').innerHTML = "ERROR RECORDING";
        /**
        for(var Sens in data) {
          //console.log(Sens);
          //console.log(data[Sens]);
          sensor = data[Sens];
          var theNum = Sens.match(/\d+/)[0];
          //console.log("Sensor["+Sens+"] => theNum["+theNum+"]");
          var Surr  = "Surr"+theNum;
          var Val   = "Val"+theNum;
          var State = "State"+theNum;

          document.getElementById(State).innerHTML = " &nbsp; ";
          
          if (sensor.c > 0)
          {
            document.getElementById(Surr).innerHTML = sensor.c +" cm ";
            document.getElementById(Val).innerHTML = sensor.v +" cm ";
          //document.getElementById(State).innerHTML = sensor.s;
            if (sensor.s > 0)
                 document.getElementById(State).style.backgroundColor = "red"; 
            else document.getElementById(State).style.backgroundColor = "lightgreen"; 
          }
          else
          {
            document.getElementById(Surr).innerHTML = "- ";
            document.getElementById(Val).innerHTML = "- ";
            document.getElementById(State).style.backgroundColor = "lightgray"; 
          }
        };  // for(..)
        **/
      })
      .catch(function(error) {
        document.getElementById('message').innerHTML = error;
      });   
  
  } // startRecording()
  
  //============================================================================  
  function stopRecording()
  {
    console.log("stopRecording() ..");
        
    document.getElementById('message').innerHTML = "";
    
    fetch(APIGW+"stopRec")
      .then(response => response.json())
      .then(json => {
        console.log("parsed --> json is ["+ JSON.stringify(json)+"]");
        data = json.stopRec;
        if (data == "OK")  document.getElementById('message').innerHTML = "RECORDING STOPPED";
        else               document.getElementById('message').innerHTML = "ERROR STOPPING RECORDING";

      })
      .catch(function(error) {
        document.getElementById('message').innerHTML = error;
      });     
  } // stopRecording()
  
  //============================================================================  
  function listRecording()
  {
    console.log("listRecording() ..");
        
    document.getElementById('message').innerHTML = "";
    
    fetch(APIGW+"listRec")
      .then(response => response.json())
      .then(json => {
        console.log("parsed --> json is ["+ JSON.stringify(json)+"]");
        data = json.listRec;
        if (data == "OK")  document.getElementById('message').innerHTML = "LISTING DATA (TELNET)";
        else               document.getElementById('message').innerHTML = "ERROR LISTING DATA!";
      })
      .catch(function(error) {
        document.getElementById('message').innerHTML = error;
      });     
      
  } // listRecording()
  
  //============================================================================  
  function analyzeRecording()
  {
    console.log("analyzeRecording() ..");
        
    document.getElementById('message').innerHTML = "";
    //saveInput("DUTname");
    saveInput("cutOffCurr");
    saveInput("sleepCurr");
    saveInput("skipCurr");
    
    fetch(APIGW+"analyzeRec")
      .then(response => response.json())
      .then(json => {
        console.log("json is ["+json+"]");
        console.log("parsed --> json is ["+ JSON.stringify(json)+"]");
        data = json.analyzeRec;
        if (data == "OK")  
              document.getElementById('message').innerHTML = "ANALIZING DATA (TELNET)";
        else if (data == "DONE")  
              document.getElementById('message').innerHTML = "ANALIZING DATA DONE";
        else  document.getElementById('message').innerHTML = "ERROR ANALIZING DATA!";
      })
      .catch(function(error) {
        document.getElementById('message').innerHTML = error;
      });     
      
  } // analyzeRecording()
  
  
  //============================================================================  
  function updateInput() {
    console.log("in updateInput() ..");
    fetch("/echo/json/",
    {
      method: "POST",
      body: data
    })
    .then(function(res){ return res.json(); })
    .then(function(data){ alert( JSON.stringify( data ) ) })  
  
  } // setState();


  //============================================================================  
  function downloadDat() {
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
  function round(value, precision) {
    var multiplier = Math.pow(10, precision || 0);
    return Math.round(value * multiplier) / multiplier;
  }

  
  //============================================================================  
  function printAllVals(obj) {
    for (let k in obj) {
      if (typeof obj[k] === "object") {
        printAllVals(obj[k])
      } else {
        // base case, stop recurring
        console.log(obj[k]);
      }
    }
  } // peintAllVals()
  
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
