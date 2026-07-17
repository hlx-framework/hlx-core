import { execSync } from 'node:child_process'
import fs from 'node:fs'
import path from 'node:path'
import { BOOT_BUILD_DIR, BOOT_DIR, HAXE_EXE, LOADER_DIR, LOADER_BUILD_DIR } from './paths.mts'

// vswhere.exe has lived at this exact fixed path since VS 2017.
const VSWHERE = 'C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe'

function isCmdAvailable(cmd: string): boolean {
  try {
    execSync(`where ${cmd}`, { stdio: 'pipe' })
    return true
  } catch {
    return false
  }
}

function vswhereFind(pattern: string): string | null {
  if (!fs.existsSync(VSWHERE)) return null
  const out = execSync(
    `"${VSWHERE}" -latest -products * ` +
    '-requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 ' +
    `-find ${pattern}`,
    { encoding: 'utf8' },
  ).trim()
  return out ? out.split(/\r?\n/)[0] : null
}

// CMake's VS generator locates MSBuild itself; only cmake.exe needs to be reachable (falls back to the VS-bundled copy).
function resolveCmake(): string {
  if (isCmdAvailable('cmake.exe')) return 'cmake'
  console.log('cmake.exe not on PATH - locating the copy bundled with Visual Studio...')
  const bundled = vswhereFind('Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake.exe')
  if (!bundled) {
    throw new Error(
      'cmake.exe not found on PATH and no Visual Studio-bundled copy was located. Install ' +
      'CMake (or the "Desktop development with C++" workload, which bundles it), or add it to PATH.',
    )
  }
  return `"${bundled}"`
}

export function runBuild(): void {
  console.log('Building libhl64.dll (hlx-boot)...')
  const cmake = resolveCmake()
  execSync(`${cmake} -S . -B build`, { cwd: BOOT_DIR, stdio: 'inherit' })
  execSync(`${cmake} --build build --config Release`, { cwd: BOOT_DIR, stdio: 'inherit' })

  const dllPath = path.join(BOOT_BUILD_DIR, 'Release', 'libhl64.dll')
  if (!fs.existsSync(dllPath)) throw new Error(`Build did not produce ${dllPath}`)
  console.log('libhl64.dll -> built')

  console.log('\nBuilding hlx-loader.hl...')
  if (!fs.existsSync(HAXE_EXE)) {
    throw new Error(`Haxe compiler not found at ${HAXE_EXE} - update tools/lib/paths.mts if it moved`)
  }
  execSync(`"${HAXE_EXE}" compile.hxml`, { cwd: LOADER_DIR, stdio: 'inherit' })

  const loaderPath = path.join(LOADER_BUILD_DIR, 'hlx-loader.hl')
  if (!fs.existsSync(loaderPath)) throw new Error(`Build did not produce ${loaderPath}`)
  console.log('hlx-loader.hl -> built')
}
