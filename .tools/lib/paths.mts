import fs from 'node:fs'
import os from 'node:os'
import path from 'node:path'

export const REPO_ROOT = path.resolve(import.meta.dirname, '../..')
export const BOOT_DIR = path.join(REPO_ROOT, 'hlx-boot')
export const BOOT_BUILD_DIR = path.join(BOOT_DIR, 'build')
export const LOADER_DIR = path.join(REPO_ROOT, 'hlx-loader')
export const LOADER_BUILD_DIR = path.join(LOADER_DIR, 'build')

// Update this if the Haxe compiler moves.
export const HAXE_EXE = 'D:\\Projects\\haxe\\haxe\\haxe.exe'

const CONFIG_PATH = path.join(REPO_ROOT, '.tools', 'user-config.json')

interface UserConfig {
  gamePath: string
}

export function readUserConfig(): UserConfig {
  if (!fs.existsSync(CONFIG_PATH))
    throw new Error('.tools/user-config.json not found - run: pnpm run setup')
  return JSON.parse(fs.readFileSync(CONFIG_PATH, 'utf8'))
}

export function writeUserConfig(config: UserConfig): void {
  fs.writeFileSync(CONFIG_PATH, JSON.stringify(config, null, 2) + '\n', 'utf8')
}

// Only matters when run from WSL against a Windows-style path; the build step itself needs cmake.exe/haxe.exe on Windows.
export function toNativePath(windowsPath: string): string {
  if (os.platform() !== 'linux') return windowsPath
  return windowsPath
    .replace(/\\/g, '/')
    .replace(/^([A-Za-z]):/, (_, drive: string) => `/mnt/${drive.toLowerCase()}`)
}

export function gameDir(): string {
  return toNativePath(readUserConfig().gamePath)
}
