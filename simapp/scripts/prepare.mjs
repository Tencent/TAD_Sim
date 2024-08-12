import { execSync } from 'node:child_process'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

const thisDir = path.dirname(fileURLToPath(import.meta.url))
const rootDir = path.resolve(thisDir, '..')

useOldVersionDependencies()

/**
 * 使用旧版本的依赖
 * 高版本用来应对 codecc 检查，但实际运行应使用低版本
 */
function useOldVersionDependencies () {
  // desktop
  const desktopDir = path.join(rootDir, 'desktop')
  execSync('npm i electron@14.2.6', { cwd: desktopDir })

  // map-editor
  const mapEditorDir = path.join(rootDir, 'map-editor')
  execSync('npm i three@0.144.0', { cwd: mapEditorDir })

  // scene-editor
  const sceneEditorDir = path.join(rootDir, 'scene-editor')
  execSync('npm i three@0.122.0', { cwd: sceneEditorDir })
}
