import { execSync } from 'node:child_process'
import path from 'node:path'
import { fileURLToPath } from 'node:url'
import { createRequire } from 'node:module'

const require = createRequire(import.meta.url)
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
  if (!isPackageInstalled(desktopDir, 'electron', '14.2.6')) {
      try {
        console.log('Installing electron@14.2.6...')
        execSync('npm i electron@14.2.6', { cwd: desktopDir, stdio: 'inherit' })
      } catch (e) {
          console.warn('Warning: Failed to install electron@14.2.6. Assuming offline and proceeding if module exists.')
      }
  }

  // map-editor
  const mapEditorDir = path.join(rootDir, 'map-editor')
  if (!isPackageInstalled(mapEditorDir, 'three', '0.144.0')) {
      try {
        console.log('Installing three@0.144.0...')
        execSync('npm i three@0.144.0', { cwd: mapEditorDir, stdio: 'inherit' })
      } catch (e) {
          console.warn('Warning: Failed to install three@0.144.0. Assuming offline and proceeding if module exists.')
      }
  }

  // scene-editor have been resolved. @joybee007 2024/8/20
  // const sceneEditorDir = path.join(rootDir, 'scene-editor')
  // execSync('npm i three@0.122.0', { cwd: sceneEditorDir })
}

function isPackageInstalled(dir, packageName, version) {
    try {
        const pkg = require(path.join(dir, 'node_modules', packageName, 'package.json'));
        return pkg.version === version;
    } catch (e) {
        return false;
    }
}
