// page.h - the web page the board serves, stored in flash (PROGMEM).
//
// This is one self-contained HTML file: CSS and JavaScript inline, no
// internet needed. The board sends it as-is when a phone asks for "/".
// The JavaScript at the bottom calls /api/status once a second and
// /api/peers every two seconds (the "Boards nearby" table).
// Edit the HTML freely; to the ESP32 it is just text.

#pragma once
#include <pgmspace.h>

const char PAGE_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32</title>
<style>
body{margin:0;padding:16px;background:#fff;color:#000;font:16px/1.5 -apple-system,"Segoe UI",Roboto,Helvetica,Arial,sans-serif}
main{max-width:560px;margin:0 auto}
h1{font-size:1.6em;margin:0 0 12px}
h2{font-size:1.1em;margin:20px 0 6px}
table{width:100%;border-collapse:collapse}
td,th{padding:6px 4px;border-bottom:1px solid #ddd;text-align:left;vertical-align:top}
th{font-weight:600}
#status td:first-child{width:45%}
.buttons{margin:14px 0}
button{font:inherit;padding:8px 14px;margin:0 6px 6px 0}
#conn{margin:4px 0 0}
pre{margin:0;padding:8px;border:1px solid #ddd;font-size:14px;white-space:pre-wrap;word-break:break-word;min-height:3em}
</style></head><body><main>
<h1 id="name">ESP32</h1>

<table id="status">
<tr><td>Uptime</td><td id="uptime">-</td></tr>
<tr><td>Chip temperature</td><td id="temp">-</td></tr>
<tr><td>Touch</td><td id="touch">-</td></tr>
<tr><td>Button</td><td id="button">-</td></tr>
<tr><td>LED</td><td id="led">-</td></tr>
<tr><td>Pattern</td><td id="pattern">-</td></tr>
<tr><td>Connected devices</td><td id="clients">-</td></tr>
<tr><td>Free heap</td><td id="heap">-</td></tr>
<tr><td>Chip</td><td id="chip">-</td></tr>
</table>

<div class="buttons">
<button onclick="cmd('/api/led?state=on','LED on')">LED on</button>
<button onclick="cmd('/api/led?state=off','LED off')">LED off</button>
<br>
<button onclick="cmd('/api/pattern?name=off','pattern off')">off</button>
<button onclick="cmd('/api/pattern?name=on','pattern on')">on</button>
<button onclick="cmd('/api/pattern?name=blink','pattern blink')">blink</button>
<button onclick="cmd('/api/pattern?name=sos','pattern sos')">sos</button>
</div>

<p id="conn">waiting for the first response</p>

<h2>Boards nearby</h2>
<table>
<thead><tr><th>Name</th><th>Last seen</th><th>Temp</th><th>LED</th><th>RSSI</th></tr></thead>
<tbody id="peers"><tr><td colspan="5">no other boards heard yet</td></tr></tbody>
</table>

<h2>Event log</h2>
<pre id="log"></pre>
</main>
<script>
var $=function(id){return document.getElementById(id)};
var lines=[],last={},lastOk=0,known={};
function log(m){
  var t=new Date().toTimeString().slice(0,8);
  lines.push(t+' '+m);if(lines.length>10)lines.shift();
  $('log').textContent=lines.join('\n');
}
function fmtUptime(s){
  s=Math.floor(s);var h=Math.floor(s/3600),m=Math.floor(s%3600/60),x=s%60;
  return (h?h+' h ':'')+(h||m?m+' min ':'')+x+' s';
}
function esc(s){return String(s).replace(/[<>&]/g,function(c){return {'<':'&lt;','>':'&gt;','&':'&amp;'}[c]})}
function render(d){
  $('name').textContent=d.name;document.title=d.name;
  $('uptime').textContent=fmtUptime(d.uptime_s);
  $('temp').textContent=d.temp_c.toFixed(1)+' \u00b0C';
  $('touch').textContent=d.touch;
  $('button').textContent=d.button?'pressed':'released';
  $('led').textContent=d.led?'on':'off';
  $('pattern').textContent=d.pattern;
  $('clients').textContent=d.clients+(d.rssi!=null?' (RSSI '+d.rssi+' dBm)':'');
  $('heap').textContent=Math.round(d.heap_free/1024)+' KB';
  $('chip').textContent=d.chip;
  if(last.pattern!==undefined&&last.pattern!==d.pattern)log('pattern changed to '+d.pattern);
  if(last.button!==undefined&&d.button&&!last.button)log('button pressed');
  last=d;
}
function poll(){
  fetch('/api/status',{cache:'no-store'}).then(function(r){
    if(!r.ok)throw 0;return r.json();
  }).then(function(d){
    if(lastOk&&Date.now()-lastOk>3000)log('connection is back');
    lastOk=Date.now();render(d);
  }).catch(function(){});
}
function pollPeers(){
  fetch('/api/peers',{cache:'no-store'}).then(function(r){return r.json()}).then(function(list){
    var seen={};
    list.forEach(function(p){seen[p.name]=1;if(!known[p.name])log('heard '+p.name);});
    Object.keys(known).forEach(function(n){if(!seen[n])log('lost '+n);});
    known=seen;
    if(!list.length){$('peers').innerHTML='<tr><td colspan="5">no other boards heard yet</td></tr>';return;}
    $('peers').innerHTML=list.map(function(p){
      return '<tr><td>'+esc(p.name)+'</td><td>'+p.age_s+' s ago</td><td>'+p.temp_c.toFixed(1)+' \u00b0C</td><td>'+(p.led?'on':'off')+'</td><td>'+(p.rssi!=null?p.rssi+' dBm':'-')+'</td></tr>';
    }).join('');
  }).catch(function(){});
}
function tick(){
  if(!lastOk)return;
  var age=(Date.now()-lastOk)/1000;
  $('conn').textContent=age<3?'updated '+age.toFixed(1)+' s ago':'no response for '+Math.floor(age)+' s';
}
function cmd(url,label){
  fetch(url,{cache:'no-store'}).then(function(r){return r.json()}).then(function(d){
    log(label+(d.ok?': ok':': error, '+d.error));
    if(d.ok){lastOk=Date.now();render(d);}
  }).catch(function(){log(label+': no reply');});
}
log('page loaded');
poll();setInterval(poll,1000);pollPeers();setInterval(pollPeers,2000);setInterval(tick,200);
</script></body></html>)rawliteral";
