const fs = require('fs')
const path = require('path')

const NODE_PORT_MIN = 30000
const NODE_PORT_MAX = 32767

function check(conf) {
    const owners = new Map()
    const visit = (value, location) => {
        if (Array.isArray(value)) {
            value.forEach((item, index) => visit(item, `${location}/${index}`))
            return
        }

        if (value === null || typeof value !== 'object') {
            return
        }

        for (const [key, item] of Object.entries(value)) {
            const current = `${location}/${key}`
            const isNodePort = (key === 'node' || key === 'port') && Number.isInteger(item) && item >= NODE_PORT_MIN && item <= NODE_PORT_MAX
            if (isNodePort === false) {
                visit(item, current)
                continue
            }

            if (owners.has(item)) {
                throw new Error(`NodePort ${item} is used by both ${owners.get(item)} and ${current}`)
            }
            owners.set(item, current)
        }
    }

    visit(conf, '')
}

module.exports = { check }

if (require.main === module) {
    const conf = JSON.parse(fs.readFileSync(path.join(__dirname, 'develop.json'), 'utf-8'))
    check(conf)
    console.log('NodePorts are unique.')
}
