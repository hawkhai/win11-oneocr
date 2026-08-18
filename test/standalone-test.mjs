import assert from 'node:assert/strict'
import { apply } from '../src/index.js'

let definition
apply({ tools: { register(tool) { definition = tool } } })
assert.equal(definition.name, 'oneocr_recognize')
assert.deepEqual(definition.parameters.required, ['image_path'])
assert.equal(definition.parameters.properties.image_path.type, 'string')
assert.match(definition.description, /offline Windows 11 OneOCR/)
assert.throws(() => apply({ tools: { register() {} } }, { timeoutMs: 0 }), /positive integer/)
console.log('OneOCR DSH plugin contract: PASS')
