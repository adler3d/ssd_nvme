const util = require('util');
const vm = require('vm');
const assert=require('assert');

var child_process=require('child_process');
var execSync=child_process.execSync;var exec=child_process.exec;
var spawnSync=child_process.spawnSync;var spawn=child_process.spawn;

var qs = require('querystring');
var http = require("http"),
    https = require("https"),
    url = require("url"),
    path = require("path"),
    fs = require("fs"),
    os = require("os"),
    process = require('process'),
    crypto = require('crypto');

var getDateTime_v2=t=>{
  var now     = typeof t==='number'?new Date(t):new Date();
  var year    = now.getFullYear();
  var f=v=>(v.toString().length==1?'0':'')+v;
  var month   = f(now.getMonth()+1); 
  var day     = f(now.getDate());
  var hour    = f(now.getHours());
  var minute  = f(now.getMinutes());
  var second  = f(now.getSeconds());
  var ms=now.getMilliseconds()+"";var ttt=[0,"00","0",""];ms=ttt[ms.length]+ms;
  var dateTime = year+'.'+month+'.'+day+' '+hour+':'+minute+':'+second+'.'+ms;
  return dateTime;
}

var qap_add_time=s=>"["+getDateTime_v2()+"] "+s;
var qap_log=s=>console.log(qap_add_time(s));

var json=JSON.stringify;
var mapkeys=Object.keys;var mapvals=(m)=>mapkeys(m).map(k=>m[k]);
var inc=(m,k)=>{if(!(k in m))m[k]=0;m[k]++;return m[k];};

var FToS=n=>(n+0).toFixed(2);
var mapswap=(k2v)=>{var v2k={};for(var k in k2v){v2k[k2v[k]]=k;}return v2k;}
var qapavg=(arr,cb)=>{if(typeof cb=='undefined')cb=e=>e;return arr.length?arr.reduce((pv,ex)=>pv+cb(ex),0)/arr.length:0;}
var qapsum=(arr,cb)=>{if(typeof cb=='undefined')cb=e=>e;return arr.reduce((pv,ex)=>pv+cb(ex),0);}
var qapmin=(arr,cb)=>{if(typeof cb=='undefined')cb=e=>e;var out;var i=0;for(var k in arr){var v=cb(arr[k]);if(!i){out=v;}i++;out=Math.min(out,v);}return out;}
var qapmax=(arr,cb)=>{if(typeof cb=='undefined')cb=e=>e;var out;var i=0;for(var k in arr){var v=cb(arr[k]);if(!i){out=v;}i++;out=Math.max(out,v);}return out;}
var qapsort=(arr,cb)=>{if(typeof cb=='undefined')cb=e=>e;return arr.sort((a,b)=>cb(b)-cb(a));}
var mapdrop=(e,arr,n)=>{var out=n||{};Object.keys(e).map(k=>arr.indexOf(k)<0?out[k]=e[k]:0);return out;}
var mapsort=(arr,cb)=>{if(typeof cb=='undefined')cb=(k,v)=>v;var out={};var tmp=qapsort(mapkeys(arr),k=>cb(k,arr[k]));for(var k in tmp)out[tmp[k]]=arr[tmp[k]];return out;}
var table_fix_fields=arr=>{var n2keys=[];arr.map(e=>n2keys[mapkeys(e).length]=mapkeys(e));var order=n2keys.pop();return arr.map(e=>{var m={};order.map(k=>m[k]=k in e?e[k]:0);return m;});};

var qap_unique=arr=>{var tmp={};arr.map(e=>tmp[e]=1);return mapkeys(tmp);};var unique_arr=qap_unique;


var json_once_v2=(e,v,lim)=>json_once(e,v,2,lim);
var inspect=json_once_v2;

var escapeHtml=(s)=>{
  if("string"!==(typeof s)){return s;}
  return s.replace(/&/g,"&amp;").replace(/</g,"&lt;").replace(/>/g,"&gt;").replace(/"/g,"&quot;").replace(/'/g,"&#039;");
}
var maps2table_impl=(table)=>{
  function skip_field(field){
    var ignore=[];//["user_agent","request_uri","referrer"];
    for(var key in ignore)if(ignore[key]==field){return true;}
    return false;
  };
  //var def_table=[{'id':1,'nick':'Owen'},{'id':2,'nick':'Kyle'}];
  if(!table.length){return 'table is empty';}
  var km={};for(var i=0;i<table.length;i++){var ex=table[i];for(var k in ex){inc(km,k);}}
  var arr=Object.keys(km);
  if(!arr.length){return 'table look like empty: '+json({"table.keys.length":arr.length,"table.length":table.length});}
  var out="";var head="";
  for(var i in arr)
  {
    if(skip_field(arr[i]))continue;
    out+='<td>'+escapeHtml(arr[i])+'</td>';
  }
  var head='<thead><tr>'+out+'</tr></thead>';
  out="";
  for(var i=0;i<table.length;i++)
  {
    var tmp="";
    //var tmp_arr=table[table.length-i-1];
    var tmp_arr=table[i];
    for(var j=0;j<arr.length;j++){
      //if(skip_field(key))continue;
      var k=arr[j];var v="<b>0</b>";var bg="";
      if(k in tmp_arr){v=escapeHtml(tmp_arr[k]);}else{/*bg='class="bgw"';*/}
      tmp+='<td>'+v+'</td>';
    }
    out+='<tr>'+tmp+'</tr>';
  }
  out='<table>'+head+'<tbody>'+out+'</tbody></table>';
  return out;
}
var gen_maps2table_style=dtm=>`<style>
  @dtm tr:nth-child(2n){background:#FEFEFE;}
  @dtm table{border-collapse:collapse;font-size:10pt;text-align:right}
  @dtm td,th,thead{border:1px solid #bbb;padding:4px;}
  @dtm th,thead{text-align:center;font-weight:bold;background:#ccc;}
</style>`.split("@dtm").join('undefined'===typeof dtm?"div.table_main":dtm);

var with_style_for_center_pre_div_table=(str,dc)=>{
  dc='undefined'===typeof dc?'table_main':dc;
  var s=gen_maps2table_style("div."+dc);
  return s+'<center><pre><div class="'+dc+'">'+str+'</div></pre></center>';
}

var maps2table=(table,dc)=>{return with_style_for_center_pre_div_table(maps2table_impl(table),dc);};
var maps2csv=arr=>{var h=mapkeys(arr[0]);return h.join(",")+"\n"+arr.map(e=>h.map(k=>e[k]).join(",")).join("\n");}
var parse_csv=(s,sep)=>{var t=s.split("\r").join("").split("\n").map(e=>e.split('undefined'===typeof sep?",":sep));return t;}
var parse_csv_with_head=(s,sep)=>{
  var t=parse_csv(s,sep);
  var pcsv={head:t[0],arr:t.slice(1)};
  pcsv.get=(y,key)=>pcsv.arr[y][pcsv.head.indexOf(key)];
  return pcsv;
}
var parsed_csv2maps=csv=>csv.arr.map(e=>{var out={};csv.head.map((k,id)=>out[k]=e[id]);return out;});
var pcsv2table_impl=(pcsv,cb)=>{
  cb="undefined"!==typeof cb?cb:(str,pos,pcsv,arr)=>escapeHtml(str);
  var h=pcsv.head;
  var out=h.map((e,id)=>"<td>"+cb(e,{t:'h',y:0,x:id},pcsv,h)+"</td>").join("");
  var head='<thead><tr>'+out+'</tr></thead>';
  out=pcsv.arr.map((arr,y)=>{
    return h.map((key,id)=>id<arr.length?cb(arr[id],{t:'b',y:y,x:id,key:key},pcsv,arr):"<b>0</b>").map(e=>"<td>"+e+"</td>").join("");
  });
  out=out.map(e=>"<tr>"+e+"</tr>").join("");
  return '<table>'+head+'<tbody>'+out+'</tbody></table>';
}
var pcsv2table=(pcsv,cb)=>{
  //var cb=(str,pos,pcsv,arr)=>{if(0)escapeHtml(str);return "<b>"+escapeHtml(json(pos))+"</b>";};
  return with_style_for_center_pre_div_table(pcsv2table_impl(pcsv,cb));
}
var csv2table=(str,sep,cb)=>{
  //var cb=(str,pos,pcsv,arr)=>{if(0)escapeHtml(str);return "<b>"+escapeHtml(json(pos))+"</b>";};
  var pcsv=parse_csv_with_head(str,sep);
  return pcsv2table(pcsv,cb);
}
var pcsv2csv=(pcsv,sep)=>{
  sep='undefined'===typeof sep?",":sep;
  return pcsv.head.join(sep)+"\n"+pcsv.arr.map(e=>e.join(sep)).join("\n");
}
var pcsv2table_impl_v2=(pcsv,cb)=>{
  cb=("undefined"!==typeof cb)&&cb?cb:(str,pos,pcsv,arr,td,tag,bg,rg)=>td(escapeHtml(str));
  var tag=(t,s)=>'<'+t+'>'+s+'</'+t.split(" ")[0]+'>';var td=s=>tag('td',s);
  var bg=(r,g,b,str)=>'<td style="background-color:rgb('+r+','+g+','+b+');">'+str+'</td>';
  var wr=(c,str)=>bg(0xff,c,c,str);var wg=(c,str)=>bg(c,0xff,c,str);
  var rg=(v,str,max,base)=>{max=max?max:12.8;base=base?base:128;return (v<0?wr:wg)((Math.max(0.0,1.0-Math.abs(v)/max)*(0xff-base)+base)|0,str);}
  var h=pcsv.head;
  var head=h.map((e,id)=>cb(e,{t:'h',y:0,x:id,key:e},pcsv,h,td,tag,bg,rg)).join("");
  var out=pcsv.arr.map((arr,y)=>{
    return h.map((key,id)=>id<arr.length?cb(arr[id],{t:'b',y:y,x:id,key:key},pcsv,arr,td,tag,bg,rg):"<b>0</b>").join("");
  });
  out=out.map(e=>"<tr>"+e+"</tr>").join("");
  return tag('table',tag('thead',tag('tr',head))+tag('tbody',out));
}
var pcsv2table_v2=(pcsv,cb)=>{
  pcsv.get=(y,key)=>pcsv.arr[y][pcsv.head.indexOf(key)];
  return with_style_for_center_pre_div_table(pcsv2table_impl_v2(pcsv,cb));
}

var links2table=arr=>{
  var head=("<html><style>table{border-spacing:64px 0;font-size:1.17em;font-weight:bold;}div{"+
    "position:absolute;top:5%;left:50%;transform:translate(-50%,0%);"+
    "}</style><body><div>"
  );
  var as_table=arr=>'<table>'+(arr.map(e=>'<tr><td><a href="'+e+'">'+e+'</a></td></tr>').join("\n"))+"</table>";
  return head+as_table(arr);
}

var pslist2json=s=>{
  var t=s.split("\r").join("").split("\n").slice(2).slice(0,-1).map(e=>e.split(/\s+/));
  var h=t[0].join(" ").split("Priv Pk").join("Priv_Peek").split(" ").slice(1);var a=t.slice(1);
  var out=a.map(e=>({
    name:e.slice(0,-8).join(" "),
    fields:e.reverse().slice(0,8).reverse(),
    //fn:e.reverse().slice(0,h.length).reverse().length
  }));//return inspect({hl:h.length,h,out});
  out=out.map(q=>{var t={Name:q.name};h.map((k,i)=>t[k]=q.fields[i]);return t;});
  return out;
}

const requestListener = function (request, res) {
  var purl=url.parse(request.url);var uri=purl.pathname;var qp=qs.parse(purl.query);
  var html=((res)=>{var r=res;return s=>{r.writeHead(200,{"Content-Type":"text/html"});r.end(s);}})(res);
  if(purl.path!=="/"+"favicon.ico")qap_log("url = "+purl.path);
  if("/sitemap"==uri){
    var hide="close,exit,inc,dec,del,put,get,internal,eval,tick,ping".split(",").concat("g,");
    var preproc=s=>s.split('+"/').join("*cut*");
    return html(links2table(
      qap_unique(
        preproc(fs.readFileSync("ff_sus.js")+"").split('"'+'/').map(e=>e.split('"')[0]).slice(1).filter(e=>e.length)
      ).filter(e=>hide.indexOf(e)<0).map(e=>'/'+e))
    );
  }
  var run=fn=>""+execSync(path.normalize("../ps/"+fn));
  if("/pslist"==uri){
    res.writeHead(200);res.end(run("pslist -m -nobanner"));
  }
  if("/pslist/html"==uri){
    res.writeHead(200);res.end(maps2table(pslist2json(run("pslist -m -nobanner"))));
  }
  if("/pslist/json"==uri){
    res.writeHead(200);res.end(json(pslist2json(run("pslist -m -nobanner")),0,2));
  }
  if("/set_cpu_maxpower"==uri){
    var v=qp.v|0;if(v<20)v=20;if(v>100)v=100;
    execSync("Powercfg -setacvalueindex scheme_current sub_processor PROCTHROTTLEMAX "+v);
    execSync("Powercfg -setactive scheme_current");
    console.log("["+getDateTime_v2()+"]: PROCTHROTTLEMAX = "+v);
    res.writeHead(200);res.end("PROCTHROTTLEMAX = "+v);
  }
  if("/pssuspend/s/firefox"==uri){
    res.writeHead(200);res.end(run("pssuspend.exe firefox.exe -nobanner"));
  }
  if("/pssuspend/r/firefox"==uri){
    res.writeHead(200);res.end(run("pssuspend.exe -r firefox.exe -nobanner"));
  }
  if("/GUI"==uri){
    res.writeHead(200);
    var s="";
    s+="<script>var f=(api)=>()=>fetch(api);var start=f('/pssuspend/r/firefox');var stop=f('/pssuspend/s/firefox');</script>";
    s+=`<style>button{
      display: block;
      width: 50%;
      border: none;
      background-color: #4CAF50;
      padding: 14px 28px;
      font-size: 72px;
      cursor: pointer;
      text-align: center;
    }</style>
    <button onclick="start()">START</button>
    <br><br><br>
    <button onclick="stop()">STOP</button></center>`;
    res.end(html('<html><body><center>'+s+'</body></hmtl>'));
  }
  if("/ssd_nvme"==uri){
    var s=""+execSync('node read.js tail_k=0.995 tail_min=32000 tail_max=99000 mode=by_recs');
    res.writeHead(200);res.end(maps2table(s.split("\n").reverse().filter(e=>e.trim().length).map(e=>JSON.parse(e))));
  }
  if("/ssd_nvme_full"==uri){
    var s=""+execSync('node read.js');
    res.writeHead(200);res.end(maps2table(s.split("\n").reverse().filter(e=>e.trim().length).map(e=>JSON.parse(e))));
  }
  res.writeHead(404);res.end('not found');
}

const server = http.createServer(requestListener);
server.listen(80);
qap_log("ff_sus.js runned at localhost:80\n");
