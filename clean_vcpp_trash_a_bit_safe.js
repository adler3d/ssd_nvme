var execSync=require('child_process').execSync;var fs=require('fs');
var ext="ipch";// obj,pdb,ilk,tlog,ipch,pch,sdf,idb
var arr=(execSync("find -type f|grep "+ext)+"").split("\r").join("").split("\n");
//console.log(JSON.stringify(arr,0,2));
if(arr.length)if(arr[0].split(":").length>1)arr=arr.map(e=>e.split(":").slice(1).join(":"));
var a=arr.filter(e=>e.length).filter(e=>e.split(".").reverse()[0]===ext).filter(e=>!e.includes("Python")&&!e.includes("python"));
var buf=Buffer.alloc(4);
var is3D=fn=>{
  var f=fs.openSync(fn,'r');
  var say=s=>{fs.closeSync(f);return s;}
  var n=fs.readSync(f,buf,0,4,null);
  if(n!=4)return say('err');
  var a=[0,0,0xff,0xff];
  for(var i=0;i<4;i++)if(a[i]!=buf[i])return say('3D');
  return say((fs.readFileSync(fn)+"").split("drectve").length>1?'no':'idk');
}
if(ext==="obj")a=a.filter(e=>is3D(e)==='no');
t=0;a.map(e=>t+=fs.statSync(e).size/1024/1024);
console.log(JSON.stringify(a,0,2));
console.log(JSON.stringify({total_mb:t,files:a.length}));
//a.map(e=>fs.unlinkSync(e));

//startwith 00 00 FF FF;keyword="drectve";
