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
const adminTool = require('./admin-tool')
const log = require('./log')
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
conf.host = new pulumi.Config().require('host')


// Setup mysql, redis, rabbitmq in parallel (no dependsOn between them)
const mysqlResources = mysql.setup(namespace, conf)
const redisResources = redis.setup(namespace, conf)
const rabbitmqResources = rabbitmq.setup(namespace, conf)

// Flatten all resources for dependsOn
const allInfraResources = [].concat(mysqlResources || [], redisResources || [], rabbitmqResources || [])

// Setup internal, write-back, admin-tool, log after mysql, redis, rabbitmq are ready
const internalResources = internal.setup(namespace, conf, allInfraResources)
const writeBackResources = wb.setup(namespace, conf, [].concat(redisResources || []))
const adminToolResources = adminTool.setup(namespace, conf, allInfraResources)
const logResources = log.setup(namespace, conf, allInfraResources)

// Setup login, gateway, game after internal is ready
const gatewayResources = gateway.setup(namespace, conf, internalResources || [])
const loginResources = login.setup(namespace, conf, internalResources || [])
const gameResources = game.setup(namespace, conf, internalResources || [])
// bot.setup(namespace, conf, [].concat(gatewayResources || [], loginResources || [], gameResources || []))