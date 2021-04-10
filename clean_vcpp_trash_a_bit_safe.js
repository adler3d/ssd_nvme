var execSync=require('child_process').execSync;var fs=require('fs');
var exts="ipdb,iobj,ipch,obj,pdb,ilk,tlog,pch,sdf,idb".split(",").map(e=>"."+e);
exts.push("Browse.VC.db");
//var files=(execSync("find -type f")+"").split("\r").join("").split("\n");
var is_dir=path=>{
  try{
    return fs.statSync(path).isDirectory()?'dir':'no_dir';
  }catch(err){console.log("err: "+path);return 'err';}
}
var find_safe=path=>{
  try{
    return find_type_f(path);
  }catch(err){console.log("err: "+path);return [];}
}
var find_type_f=path=>{
  var arr=fs.readdirSync(path);
  var darr=[];
  var farr=[];
  var t2a={dir:darr,no_dir:farr,err:[]};
  arr.map(e=>t2a[is_dir(path+e)].push(e));
  //console.log(t2a);//return t2a;
  var out=farr.map(e=>path+e);
  darr.map(e=>find_safe(path+e+"/").map(e=>out.push(e)));
  return out;
}
var files=find_type_f("./");
//console.log(files);
fs.writeFileSync("allfiles.txt",files.join("\n"));
var main=ext=>{
  var arr=files.filter(e=>e.endsWith(ext));
  //(execSync("find -type f|grep "+ext)+"").split("\r").join("").split("\n");
  //console.log(JSON.stringify(arr,0,2));
  if(arr.length)if(arr[0].split(":").length>1)arr=arr.map(e=>e.split(":").slice(1).join(":"));
  var a=arr.filter(e=>!e.includes("Python")&&!e.includes("python"));
  var buf=Buffer.alloc(4);
  var fails={};
  var getdef=(m,k,def)=>{if(!(k in m))m[k]=def;return m[k];};
  var qapsort=(arr,cb)=>{if(typeof cb=='undefined')cb=e=>e;return arr.sort((a,b)=>cb(b)-cb(a));}
  var is3D=fn=>{
    var f=fs.openSync(fn,'r');
    var say=s=>{fs.closeSync(f);return s;}
    var n=fs.readSync(f,buf,0,4,null);
    if(n!=4)return say('err');
    if(buf[0]===35)return say("3D"); // 35=#
    var B2=(a,b)=>a===buf[0]&&b==buf[1];
    var drectve=()=>(fs.readFileSync(fn)+"").includes("drectve");
    var obj_x64=B2(100,134);
    var obj_x86=B2(76,1);
    if(obj_x64&&drectve())return say("no");
    if(obj_x86&&drectve())return say("no");
    var a=[0,0,0xff,0xff];
    for(var i=0;i<4;i++)if(a[i]!=buf[i]){
      var b=[];for(var k=0;k<4;k++)b[k]=buf[k];
      getdef(fails,JSON.stringify(b),[]).push(fn);
      return say('3D');
    }
    return say('idk');
  }
  //qapsort(Object.keys(fails).map(k=>({head:k,arr:fails[k]})),e=>e.arr.length);
  if(ext===".obj"){
    a=a.filter(e=>is3D(e)==='no');
    var info=qapsort(Object.keys(fails).map(k=>({head:k,arr:fails[k]})),e=>e.arr.length);
    console.log(JSON.stringify(info,0,2));
  }
  t=0;a.map(e=>t+=fs.statSync(e).size/1024/1024);
  console.log(JSON.stringify(a,0,2));
  console.log(JSON.stringify({ext,total_mb:t,files:a.length}));
  //a.map(e=>fs.unlinkSync(e));
};
exts.map(e=>main(e));
//startwith 00 00 FF FF;keyword="drectve";

// 18.8 GB -> 20.5 -> 21.2 ... 22.6 -> 25.3