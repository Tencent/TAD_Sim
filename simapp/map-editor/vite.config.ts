import { URL, fileURLToPath } from 'node:url'

import { defineConfig, loadEnv } from 'vite'
import vue from '@vitejs/plugin-vue'
import svgLoader from 'vite-svg-loader'

// https://vitejs.dev/config/
export default defineConfig(({ mode }) => {
  // 当前是否是云端版
  const isCloud = mode.includes('cloud')
  let baseURL = './'

  const server = {
    port: 8081,
    proxy: {},
    host: '127.0.0.1',
  }
  if (isCloud) {
    baseURL = '/map-editor/'
    let proxyTarget = 'https://autodrive-test.tadsim.com'
    server.port = 9090
    server.proxy = {
      '/api': {
        target: proxyTarget,
        changeOrigin: true,
        rewrite: (path: string) => path.replace(/^\/api/, ''),
      },
    }
  }

  return {
    plugins: [vue(), svgLoader()],
    resolve: {
      alias: {
        '@': fileURLToPath(new URL('./src', import.meta.url)),
        'vue-i18n': 'vue-i18n/dist/vue-i18n.cjs.js',
      },
    },
    base: baseURL,
    server,
    build: {
      outDir: 'build',
      rollupOptions: {
        input: {
          main: '/index.html',
          // 将 webWorker 线程单独打包成一个文件
          worker: 'src/workers/index.ts',
        },
        output: {
          entryFileNames: '[name].bundle.js',
        },
      },
    },
  }
})
