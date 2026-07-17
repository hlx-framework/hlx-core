import fs from 'node:fs'
import path from 'node:path'
import { runBuild } from './lib/build.mts'
import { BOOT_BUILD_DIR, LOADER_BUILD_DIR, gameDir } from './lib/paths.mts'

runBuild()

const dest = gameDir()
if (!fs.existsSync(dest)) {
  throw new Error(`Game folder not found: ${dest}\nCheck tools/user-config.json, or re-run: pnpm run setup`)
}

const dllSrc = path.join(BOOT_BUILD_DIR, 'Release', 'libhl64.dll')
const dllDst = path.join(dest, 'libhl64.dll')
fs.copyFileSync(dllSrc, dllDst)
console.log(`libhl64.dll -> ${dllDst}`)

const hlxDir = path.join(dest, 'hlx')
const loaderDir = path.join(hlxDir, 'loader')
const modsDir = path.join(hlxDir, 'mods')
const logsDir = path.join(hlxDir, 'logs')
fs.mkdirSync(loaderDir, { recursive: true })
fs.mkdirSync(modsDir, { recursive: true })
fs.mkdirSync(logsDir, { recursive: true })

const loaderSrc = path.join(LOADER_BUILD_DIR, 'hlx-loader.hl')
const loaderDst = path.join(loaderDir, 'hlx-loader.hl')
fs.copyFileSync(loaderSrc, loaderDst)
console.log(`hlx-loader.hl -> ${loaderDst}`)
console.log(`mods/ -> ${modsDir} (ready for mod subfolders, e.g. mods/<mod-name>/<mod-name>.hl)`)
console.log(`logs/ -> ${logsDir}`)

console.log(`\nDeployed to ${dest}`)
console.log('Launch the game - hlx-loader.hl loads at boot and scans hlx/mods/<name>/<name>.hl for plugins.')
console.log('Check hlx/logs/hlx.log for boot/hook/native diagnostics.')
console.log('Rollback: delete libhl64.dll and the hlx/ folder from that folder.')
