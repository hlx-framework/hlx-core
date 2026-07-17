import readline from 'node:readline'
import { readUserConfig, writeUserConfig } from './lib/paths.mts'

const DEFAULT_GAME_PATH = 'E:\\Games\\SteamLibrary\\steamapps\\common\\Farever'

function readStoredGamePath(): string | null {
  try {
    return readUserConfig().gamePath
  } catch {
    return null
  }
}

async function promptGamePath(): Promise<string> {
  const stored = readStoredGamePath()
  const defaultPath = stored ?? DEFAULT_GAME_PATH
  const rl = readline.createInterface({ input: process.stdin, output: process.stdout })
  return new Promise((resolve) => {
    const label = stored ? `Enter to keep: ${stored}` : `Enter for default: ${DEFAULT_GAME_PATH}`
    rl.question(`Game install path (${label}): `, (answer: string) => {
      rl.close()
      resolve(answer.trim() || defaultPath)
    })
  })
}

console.log('=== hlx-core Setup ===')
const gamePath = await promptGamePath()
writeUserConfig({ gamePath })
console.log(`Wrote tools/user-config.json (gamePath=${gamePath})`)
console.log('\nSetup complete. Next: pnpm run deploy')
