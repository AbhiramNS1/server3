const { Server } = require('ws')
const socket= new Server({port:4000})
var Clients =[]

socket.on('connection',ws=>{
    Clients.push(ws)
    ws.on('message',(e)=>{
        console.log(e.toString())
    })
    ws.on('close',console.log)
    ws.on('error',console.log)
})