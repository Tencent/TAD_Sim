import workerpool, { type WorkerPool } from 'workerpool'
import { globalConfig } from '@/utils/preset'

// eslint-disable-next-line import/no-mutable-exports
let pool: WorkerPool
const maxWorkers = 3

function patchWorker () {
  if (globalThis.Worker && !(globalThis.Worker as any).patched) {
    class PatchedWorker extends Worker {
      static patched = true
      constructor (scriptURL: string | URL, options?: WorkerOptions) {
        super(scriptURL, Object.assign({}, options, { type: 'module' }))
      }
    }
    globalThis.Worker = PatchedWorker
  }
}

if (!pool) {
  patchWorker()
  if (globalConfig.isDev) {
    // 开发环境
    pool = workerpool.pool(
      new URL(`../workers/index.ts?worker_file`, import.meta.url).href,
      {
        maxWorkers,
      },
    )
  } else {
    // // 构建环境
    pool = workerpool.pool('./worker.bundle.js', {
      maxWorkers,
    })
  }
}
pool.exec('init', [])

export default pool
