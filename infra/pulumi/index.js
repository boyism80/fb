const pulumi = require("@pulumi/pulumi");
const k8s = require("@pulumi/kubernetes");
const mysql = require('./mysql')
const rabbitmq = require('./rabbitmq')
const redis = require('./redis')
const internal = require('./internal')
const wb = require('./write-back')
const gateway = require('./gateway')
const login = require('./login')
const game = require('./game')
const bot = require('./bot')
const fs = require('fs');
const path = require('path')

const namespace = new k8s.core.v1.Namespace("fb-namespace", {
    metadata: {
        name: "fb",
    }
});

const filePath = path.join(__dirname, 'develop.json')
const content = fs.readFileSync(filePath, 'utf-8')
const conf = JSON.parse(content)

// const mysqlService = mysql.setup(namespace, conf)
const redisService = redis.setup(namespace, conf)
// const rabbitmqService = rabbitmq.setup(namespace, conf)
// const internalService = internal.setup(namespace, conf, [redisService, rabbitmqService])
// const writeBackService = wb.setup(namespace, conf, [redisService, rabbitmqService])
// const gatewayService = gateway.setup(namespace, conf, [internalService])
// const loginService = login.setup(namespace, conf, [internalService])
// const gameService = game.setup(namespace, conf, [internalService])
// bot.setup(namespace, conf, [gatewayService, loginService, gameService])