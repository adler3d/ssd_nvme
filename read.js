var qp={mode:"by_days",tail_k:"0",fn:'nvme_main.log'};process.argv.slice(2).map(e=>e.split("=")).map(e=>qp[e[0]]=e.length<2?"true":e[1]);
var fs=require('fs');
var getdef=(m,k,def)=>{if(!(k in m))m[k]=def;return m[k];};
var qapsum=(arr,cb)=>{if(typeof cb=='undefined')cb=e=>e;return arr.reduce((pv,ex)=>pv+cb(ex),0);}
var json=JSON.stringify;
var mapkeys=Object.keys;var mapvals=(m)=>mapkeys(m).map(k=>m[k]);
var rev=e=>e.trim().split(".").reverse().join(".");
var tail=(fn,k)=>{
  return new Promise((resolve,reject)=>{
    var all=fs.statSync(fn).size;
    var pos=(k*all)|0;
    process.stderr.write(JSON.stringify({fn,k,all,pos,dpos:all-pos})+'\n');
    var arr=[];
    var rs=fs.createReadStream(fn,{start:pos});
    rs.on('data',chunk=>arr.push(chunk));
    rs.on('end',()=>resolve(Buffer.concat(arr)));
    rs.on('error',err=>reject(err));
  });
}
var k=parseFloat(qp.tail_k);
var main=async()=>{
  var str=""+(k?await tail(qp.fn,k):fs.readFileSync(qp.fn));
  var qqq=str;//.substr(str.indexOf(rev("2020.04.23")));
  var L=qqq.split("\r").join("").split("\n");
  var out=[];
  var is_time=e=>e.length==5&&e[2]==":";
  var is_date=e=>e[2]=="."&&e[5]==".";
  var t="";var d="";var d2i={};var rd_gb=0;var wd_gb=0;
  L.map(e=>{
    if(is_date(e))d=rev(e);if(is_time(e))t=e;
    if(e.includes("HostReads_delta_GB = "))rd_gb=e.split("HostReads_delta_GB = ")[1]*1.0;
    if(e.includes("HostWrites_delta_GB = "))wd_gb=e.split("HostWrites_delta_GB = ")[1]*1.0;
    if(e.includes("RAW:")){
      var rec={d,t,rd_gb,wd_gb};getdef(d2i,d,[]).push(rec);out.push(rec);
    }
  });
  if(qp.mode==="by_recs"){console.log(out.map(json).join("\n"));process.exit();}
  mapkeys(d2i).map(d=>{
    var q=d2i[d];var f=v=>v.toFixed(3);
    d2i[d]={d,n:q.length,rd_gb:f(qapsum(q,e=>e.rd_gb)),wd_gb:f(qapsum(q,e=>e.wd_gb))}
  });
  console.log(mapvals(d2i).map(json).join("\n"));
  //return jstable_right(mapvals(d2i));
  //console.log(out);
  };
main();
