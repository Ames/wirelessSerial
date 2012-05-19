var net = require('net');

var ePts={a:0,b:0};

var serverA = net.createServer(function(c) { //'connection' listener

  if(ePts.a){
    c.end();
    return;
  }
  
  ePts.a=c;
  
  console.log('A connected');
  ePts.a.on('end', function() {
    console.log('A disconnected');
    ePts.a=0;
  });

  
//  ePts.a.write(telnetConfig);

  
  if(tryLink()){
    // fail
  }else{
    // success
  }
});

var serverB = net.createServer(function(c) { //'connection' listener
  
  if(ePts.b){
    c.end();
    return;
  }
  
  ePts.b=c;
  
  console.log('B connected');
  ePts.b.on('end', function() {
    console.log('B disconnected');
    ePts.b=0;
  });
  
//  ePts.b.write(telnetConfig);

  if(tryLink()){
    // fail
  }else{
    // success
  }

});

serverA.listen(8801, function() { //'listening' listener
  console.log('A bound');
});

serverB.listen(8802, function() { //'listening' listener
  console.log('B bound');
});

telnetConfig=new Buffer([255,254,34,255,254,1]);
//telnetConfig=new Buffer([255,252,34,255,252,1]);

function tryLink(){
  if(ePts.a && ePts.b){
    
//    ePts.a.write(255,254,1]);

//    ePts.a.write(telnetConfig);
    
    ePts.a.write("welcome.\n");
    ePts.b.write("welcome.\n");
    
    // put the phones together.
    ePts.a.pipe(ePts.b);
    ePts.b.pipe(ePts.a);
    
    console.log("ends are linked.");
    
    return 0;
  }else{
    return 1;
  }
}


